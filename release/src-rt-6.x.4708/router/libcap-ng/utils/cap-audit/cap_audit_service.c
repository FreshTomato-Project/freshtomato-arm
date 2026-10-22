// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * cap-audit - systemd service simulation support.
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 */

#include "cap_audit.h"

#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/prctl.h>
#include <unistd.h>

struct argv_builder {
	char **argv;
	size_t argc;
	size_t capacity;
};

struct word_buf {
	char *data;
	size_t len;
	size_t capacity;
	int started;
};

enum numeric_id_result {
	NUMERIC_ID_INVALID = -1,
	NUMERIC_ID_NAME,
	NUMERIC_ID_VALID,
};

#define LEGACY_INVALID_ID 65535UL
/* capng_change_id takes int IDs and reserves -1 to mean "no change". */
#define MAX_CHANGE_ID ((unsigned long)INT_MAX)
/* Supplementary groups use gid_t directly, whose all-ones value is invalid. */
#define MAX_SUPPLEMENTARY_GID ((unsigned long)((gid_t)-2))

static char *trim(char *str)
{
	char *end;

	while (isspace((unsigned char)*str))
		str++;
	if (*str == '\0')
		return str;

	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end))
		*end-- = '\0';

	return str;
}

static void clear_cap_set(cap_set_t *set)
{
	int cap;

	set->seen = true;
	for (cap = 0; cap <= CAP_LAST_CAP; cap++)
		set->caps[cap] = false;
}

static void fill_cap_set(cap_set_t *set)
{
	int cap;

	set->seen = true;
	for (cap = 0; cap <= CAP_LAST_CAP; cap++)
		set->caps[cap] = true;
}

static int cap_set_any(const cap_set_t *set)
{
	int cap;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (set->caps[cap])
			return 1;
	}

	return 0;
}

static void free_exec_argv(service_config_t *cfg)
{
	size_t i;

	if (!cfg->exec_argv)
		return;

	for (i = 0; i < cfg->exec_argc; i++)
		free(cfg->exec_argv[i]);
	free(cfg->exec_argv);
	cfg->exec_argv = NULL;
	cfg->exec_argc = 0;
}

void free_config(service_config_t *cfg)
{
	if (!cfg)
		return;

	free(cfg->user_raw);
	cfg->user_raw = NULL;
	free(cfg->sup_groups.gids);
	cfg->sup_groups.gids = NULL;
	cfg->sup_groups.count = 0;
	cfg->sup_groups.is_set = false;
	free(cfg->service_type);
	cfg->service_type = NULL;
	free(cfg->exec_start);
	cfg->exec_start = NULL;
	free_exec_argv(cfg);
}

static int replace_string(char **dst, const char *src)
{
	char *copy = NULL;

	if (src) {
		copy = strdup(src);
		if (!copy)
			return -1;
	}

	free(*dst);
	*dst = copy;
	return 0;
}

static int id_in_range(unsigned long id, unsigned long max)
{
	return id <= max && id != LEGACY_INVALID_ID;
}

static enum numeric_id_result parse_numeric_id(const char *value,
						unsigned long max,
						unsigned long *id)
{
	char *end = NULL;
	unsigned long parsed;

	if (!value || value[0] == '\0' || value[0] == '+' ||
	    value[0] == '-')
		return NUMERIC_ID_INVALID;
	if (!isdigit((unsigned char)value[0]))
		return NUMERIC_ID_NAME;

	errno = 0;
	parsed = strtoul(value, &end, 10);
	if (!end || end == value || *end != '\0')
		return NUMERIC_ID_NAME;
	if (errno || !id_in_range(parsed, max))
		return NUMERIC_ID_INVALID;

	*id = parsed;
	return NUMERIC_ID_VALID;
}

static int resolve_group_token(const char *value, gid_t *gid,
			       unsigned long max)
{
	struct group *grp;
	unsigned long parsed;
	enum numeric_id_result result;

	result = parse_numeric_id(value, max, &parsed);
	if (result == NUMERIC_ID_VALID) {
		*gid = (gid_t)parsed;
		return 0;
	}
	if (result == NUMERIC_ID_INVALID)
		return -1;

	grp = getgrnam(value);
	if (!grp || !id_in_range((unsigned long)grp->gr_gid, max))
		return -1;

	*gid = grp->gr_gid;
	return 0;
}

static int resolve_user_raw(service_config_t *cfg)
{
	struct passwd *pw;
	unsigned long parsed;
	enum numeric_id_result result;

	if (!cfg->user_is_set)
		return 0;

	result = parse_numeric_id(cfg->user_raw, MAX_CHANGE_ID, &parsed);
	if (result == NUMERIC_ID_INVALID)
		return -EINVAL;
	if (result == NUMERIC_ID_VALID) {
		cfg->user_uid = (uid_t)parsed;
		pw = getpwuid(cfg->user_uid);
		if (pw) {
			if (!id_in_range((unsigned long)pw->pw_gid,
					 MAX_CHANGE_ID))
				return -ERANGE;
			cfg->user_primary_gid = pw->pw_gid;
		} else {
			cfg->user_primary_gid = (gid_t)parsed;
		}
		cfg->user_resolved = true;
		return 0;
	}

	pw = getpwnam(cfg->user_raw);
	if (!pw)
		return -ENOENT;
	if (!id_in_range((unsigned long)pw->pw_uid, MAX_CHANGE_ID) ||
	    !id_in_range((unsigned long)pw->pw_gid, MAX_CHANGE_ID))
		return -ERANGE;

	cfg->user_uid = pw->pw_uid;
	cfg->user_primary_gid = pw->pw_gid;
	cfg->user_resolved = true;
	return 0;
}

static int fallback_to_nobody(service_config_t *cfg)
{
	struct passwd *pw;

	pw = getpwnam("nobody");
	if (pw && pw->pw_uid != 0 && pw->pw_gid != 0 &&
	    id_in_range((unsigned long)pw->pw_uid, MAX_CHANGE_ID) &&
	    id_in_range((unsigned long)pw->pw_gid, MAX_CHANGE_ID)) {
		cfg->user_uid = pw->pw_uid;
		cfg->user_primary_gid = pw->pw_gid;
	} else {
		cfg->user_uid = 65534;
		cfg->user_primary_gid = 65534;
	}

	if (replace_string(&cfg->user_raw, "nobody") != 0)
		return -1;

	/* Downstream code treats this as the effective simulated User=. */
	cfg->user_is_set = true;
	cfg->user_resolved = true;
	return 0;
}

static int finalize_user(service_config_t *cfg)
{
	int rc;

	if (!cfg->user_is_set) {
		if (cfg->dynamic_user)
			return fallback_to_nobody(cfg);
		return 0;
	}

	rc = resolve_user_raw(cfg);
	if (rc == 0)
		return 0;

	/* Only a missing dynamic name is eligible for the nobody fallback. */
	if (rc == -ENOENT && cfg->dynamic_user)
		return fallback_to_nobody(cfg);

	fprintf(stderr, "Error: invalid or unresolved User=%s\n",
		cfg->user_raw);
	return -1;
}

static int parse_bool_value(const char *value, bool *result)
{
	if (!strcasecmp(value, "yes") || !strcasecmp(value, "true") ||
	    !strcasecmp(value, "on") || !strcmp(value, "1")) {
		*result = true;
		return 0;
	}

	if (!strcasecmp(value, "no") || !strcasecmp(value, "false") ||
	    !strcasecmp(value, "off") || !strcmp(value, "0")) {
		*result = false;
		return 0;
	}

	return -1;
}

static int word_append(struct word_buf *word, char ch)
{
	char *tmp;
	size_t new_capacity;

	if (word->len + 2 <= word->capacity) {
		word->data[word->len++] = ch;
		word->data[word->len] = '\0';
		word->started = 1;
		return 0;
	}

	new_capacity = word->capacity ? word->capacity * 2 : 32;
	while (word->len + 2 > new_capacity)
		new_capacity *= 2;

	tmp = realloc(word->data, new_capacity);
	if (!tmp)
		return -1;

	word->data = tmp;
	word->capacity = new_capacity;
	word->data[word->len++] = ch;
	word->data[word->len] = '\0';
	word->started = 1;
	return 0;
}

static int argv_append(struct argv_builder *builder, char *word)
{
	char **tmp;
	size_t new_capacity;

	if (builder->argc + 2 > builder->capacity) {
		new_capacity = builder->capacity ? builder->capacity * 2 : 8;
		while (builder->argc + 2 > new_capacity)
			new_capacity *= 2;
		tmp = realloc(builder->argv, new_capacity * sizeof(char *));
		if (!tmp)
			return -1;
		builder->argv = tmp;
		builder->capacity = new_capacity;
	}

	builder->argv[builder->argc++] = word;
	builder->argv[builder->argc] = NULL;
	return 0;
}

static int flush_word(struct argv_builder *builder, struct word_buf *word)
{
	char *copy;

	if (!word->started)
		return 0;

	copy = strdup(word->data ? word->data : "");
	if (!copy)
		return -1;
	if (argv_append(builder, copy) != 0) {
		free(copy);
		return -1;
	}

	word->len = 0;
	word->started = 0;
	if (word->data)
		word->data[0] = '\0';
	return 0;
}

static void free_builder(struct argv_builder *builder)
{
	size_t i;

	for (i = 0; i < builder->argc; i++)
		free(builder->argv[i]);
	free(builder->argv);
	builder->argv = NULL;
	builder->argc = 0;
	builder->capacity = 0;
}

static int reject_command_substitution(const char *pos)
{
	return pos[0] == '`' || (pos[0] == '$' && pos[1] == '(');
}

static int tokenize_exec_start(const char *value, char ***argv, size_t *argc)
{
	struct argv_builder builder = { 0 };
	struct word_buf word = { 0 };
	int quote = 0;
	const char *pos;
	int rc = -1;

	for (pos = value; *pos; pos++) {
		if (quote == 0 && isspace((unsigned char)*pos)) {
			if (flush_word(&builder, &word) != 0)
				goto out;
			continue;
		}

		if (quote != '\'' && reject_command_substitution(pos)) {
			fprintf(stderr,
				"Error: ExecStart command substitution is unsupported\n");
			goto out;
		}

		/*
		 * systemd uses C escapes, including inside single quotes.
		 * Support literal backslashes/quotes only; discarding the slash
		 * from other escapes would silently change the command.
		 */
		if (*pos == '\\') {
			if (pos[1] != '\\' && pos[1] != '\'' && pos[1] != '"') {
				fprintf(stderr,
					"Error: unsupported ExecStart escape\n");
				goto out;
			}
			if (word_append(&word, *++pos) != 0)
				goto out;
			continue;
		}

		if (quote == 0) {
			if (*pos == '\'' || *pos == '"') {
				quote = *pos;
				word.started = 1;
			} else if (word_append(&word, *pos) != 0) {
				goto out;
			}
		} else if (quote == *pos) {
			quote = 0;
		} else if (word_append(&word, *pos) != 0) {
			goto out;
		}
	}

	if (quote != 0) {
		fprintf(stderr, "Error: unterminated quote in ExecStart\n");
		goto out;
	}
	if (flush_word(&builder, &word) != 0)
		goto out;
	if (builder.argc == 0) {
		fprintf(stderr, "Error: ExecStart has no executable\n");
		goto out;
	}

	*argv = builder.argv;
	*argc = builder.argc;
	builder.argv = NULL;
	builder.argc = 0;
	rc = 0;

out:
	free(word.data);
	free_builder(&builder);
	return rc;
}

static int strip_exec_prefixes(service_config_t *cfg, int *no_env_expand)
{
	const char *arg0;
	char *copy;
	size_t offset = 0;
	int no_setuid = 0;

	if (!cfg->exec_argv || !cfg->exec_argv[0])
		return 0;

	arg0 = cfg->exec_argv[0];
	while (arg0[offset] == '-' || arg0[offset] == '@' ||
	       arg0[offset] == ':' || arg0[offset] == '+' ||
	       arg0[offset] == '!') {
		/*
		 * @ separates the executable path from argv[0], which our
		 * command model cannot represent. Stripping it changes arguments.
		 */
		if (arg0[offset] == '@') {
			fprintf(stderr,
				"Error: ExecStart '@' prefix is unsupported\n");
			return -1;
		}
		if (arg0[offset] == ':')
			*no_env_expand = 1;
		if (arg0[offset] == '+') {
			fprintf(stderr,
				"Error: ExecStart '+' prefix is unsupported\n");
			return -1;
		}
		if (arg0[offset] == '!') {
			if (no_setuid) {
				fprintf(stderr,
					"Error: ExecStart '!!' prefix is unsupported\n");
				return -1;
			}
			no_setuid = 1;
		}
		offset++;
	}
	if (offset == 0)
		return 0;
	if (arg0[offset] == '\0') {
		fprintf(stderr, "Error: ExecStart has no executable\n");
		return -1;
	}

	copy = strdup(arg0 + offset);
	if (!copy)
		return -1;

	free(cfg->exec_argv[0]);
	cfg->exec_argv[0] = copy;
	cfg->exec_start_no_setuid = no_setuid;
	return 0;
}

static int is_exec_environment_word(const char *arg)
{
	const unsigned char *pos = (const unsigned char *)arg;

	if (*pos++ != '$' || (!isalpha(*pos) && *pos != '_'))
		return 0;
	while (isalnum(*pos) || *pos == '_')
		pos++;

	return *pos == '\0';
}

static void omit_exec_environment_words(service_config_t *cfg)
{
	size_t src;
	size_t dst = 1;

	/*
	 * The service environment is unavailable here. An unset standalone
	 * $NAME expands to zero arguments under systemd, so do not pass the
	 * unresolved placeholder literally to the audited application.
	 */
	for (src = 1; src < cfg->exec_argc; src++) {
		if (is_exec_environment_word(cfg->exec_argv[src])) {
			free(cfg->exec_argv[src]);
			continue;
		}
		cfg->exec_argv[dst++] = cfg->exec_argv[src];
	}
	cfg->exec_argv[dst] = NULL;
	cfg->exec_argc = dst;
}

static int set_exec_start(service_config_t *cfg, const char *value)
{
	char **argv = NULL;
	size_t argc = 0;
	int no_env_expand = 0;

	/*
	 * Multiple oneshot commands are a sequence, not replacements. We can
	 * trace only one command; an empty assignment may explicitly reset it.
	 */
	if (value[0] && cfg->exec_argc) {
		fprintf(stderr, "Error: multiple ExecStart commands are unsupported\n");
		return -1;
	}
	free_exec_argv(cfg);
	cfg->exec_start_no_setuid = false;
	if (replace_string(&cfg->exec_start, value) != 0)
		return -1;
	if (value[0] == '\0')
		return 0;

	if (tokenize_exec_start(value, &argv, &argc) != 0)
		return -1;

	cfg->exec_argv = argv;
	cfg->exec_argc = argc;
	if (strip_exec_prefixes(cfg, &no_env_expand) != 0)
		return -1;
	if (!no_env_expand)
		omit_exec_environment_words(cfg);
	return 0;
}

static int cap_from_name(const char *name)
{
	if (!strncasecmp(name, "CAP_", 4))
		name += 4;

	return capng_name_to_capability(name);
}

static int parse_cap_tokens(cap_set_t *set, char *value, int invert)
{
	char *saveptr = NULL;
	char *token;

	for (token = strtok_r(value, " \t", &saveptr);
	     token;
	     token = strtok_r(NULL, " \t", &saveptr)) {
		int cap = cap_from_name(token);

		if (cap < 0 || cap > CAP_LAST_CAP) {
			fprintf(stderr,
				"Error: unsupported capability name: %s\n",
				token);
			return -1;
		}
		set->caps[cap] = invert ? false : true;
	}

	return 0;
}

static int parse_cap_list(cap_set_t *set, const char *value)
{
	char *copy;
	char *tokens;
	int invert = 0;
	int rc;

	copy = strdup(value);
	if (!copy)
		return -1;
	tokens = trim(copy);

	if (tokens[0] == '\0') {
		clear_cap_set(set);
		free(copy);
		return 0;
	}

	/* Both directives use union, inverted subtraction, and full resets. */
	if (tokens[0] == '~') {
		invert = 1;
		tokens = trim(tokens + 1);
		/* A bare ~ resets even an explicitly empty or restricted set. */
		if (!set->seen || tokens[0] == '\0')
			fill_cap_set(set);
	}
	set->seen = true;

	rc = parse_cap_tokens(set, tokens, invert);
	free(copy);
	return rc;
}

static int append_gid(gid_list_t *list, gid_t gid)
{
	gid_t *tmp;

	tmp = realloc(list->gids, (list->count + 1) * sizeof(gid_t));
	if (!tmp)
		return -1;

	list->gids = tmp;
	list->gids[list->count++] = gid;
	list->is_set = true;
	return 0;
}

static int parse_supplementary_groups(gid_list_t *list, const char *value)
{
	char *copy;
	char *tokens;
	char *saveptr = NULL;
	char *token;

	copy = strdup(value);
	if (!copy)
		return -1;
	tokens = trim(copy);

	if (tokens[0] == '\0') {
		free(list->gids);
		list->gids = NULL;
		list->count = 0;
		list->is_set = true;
		free(copy);
		return 0;
	}

	for (token = strtok_r(tokens, " \t", &saveptr);
	     token;
	     token = strtok_r(NULL, " \t", &saveptr)) {
		gid_t gid;

		if (resolve_group_token(token, &gid,
					MAX_SUPPLEMENTARY_GID) != 0) {
			fprintf(stderr,
				"Error: unable to resolve SupplementaryGroups=%s\n",
				token);
			free(copy);
			return -1;
		}
		if (append_gid(list, gid) != 0) {
			free(copy);
			return -1;
		}
	}

	free(copy);
	return 0;
}

static int set_user(service_config_t *cfg, const char *value)
{
	cfg->user_is_set = value[0] != '\0';
	cfg->user_resolved = false;
	cfg->user_uid = 0;
	cfg->user_primary_gid = 0;

	if (!cfg->user_is_set) {
		free(cfg->user_raw);
		cfg->user_raw = NULL;
		return 0;
	}

	return replace_string(&cfg->user_raw, value);
}

static int set_group(service_config_t *cfg, const char *value)
{
	if (value[0] == '\0') {
		cfg->group_is_set = false;
		cfg->group_gid = 0;
		return 0;
	}

	if (resolve_group_token(value, &cfg->group_gid, MAX_CHANGE_ID) != 0) {
		fprintf(stderr, "Error: unable to resolve Group=%s\n", value);
		return -1;
	}

	cfg->group_is_set = true;
	return 0;
}

static int handle_service_directive(service_config_t *cfg,
				    const char *key, const char *value)
{
	if (!strcmp(key, "User"))
		return set_user(cfg, value);
	if (!strcmp(key, "Group"))
		return set_group(cfg, value);
	if (!strcmp(key, "SupplementaryGroups"))
		return parse_supplementary_groups(&cfg->sup_groups, value);
	if (!strcmp(key, "AmbientCapabilities"))
		return parse_cap_list(&cfg->ambient, value);
	if (!strcmp(key, "CapabilityBoundingSet") ||
	    !strcmp(key, "BoundingSet"))
		return parse_cap_list(&cfg->bounding, value);
	if (!strcmp(key, "NoNewPrivileges")) {
		if (parse_bool_value(value, &cfg->no_new_privs) != 0) {
			fprintf(stderr,
				"Error: invalid NoNewPrivileges=%s\n",
				value);
			return -1;
		}
		return 0;
	}
	if (!strcmp(key, "DynamicUser")) {
		if (parse_bool_value(value, &cfg->dynamic_user) != 0) {
			fprintf(stderr, "Error: invalid DynamicUser=%s\n",
				value);
			return -1;
		}
		return 0;
	}
	if (!strcmp(key, "ExecStart"))
		return set_exec_start(cfg, value);
	if (!strcmp(key, "Type"))
		return replace_string(&cfg->service_type, value);

	return 0;
}

int parse_service_file(const char *path, service_config_t *cfg)
{
	FILE *file;
	char *line = NULL;
	size_t line_len = 0;
	ssize_t read_len;
	int in_service = 0;
	int rc = -1;

	if (!path || !cfg)
		return -1;

	memset(cfg, 0, sizeof(*cfg));
	file = fopen(path, "r");
	if (!file) {
		fprintf(stderr, "Error: failed to open service file %s: %s\n",
			path, strerror(errno));
		return -1;
	}

	while ((read_len = getline(&line, &line_len, file)) != -1) {
		char *cur;
		char *eq;

		if (read_len > 0 && line[read_len - 1] == '\n')
			line[read_len - 1] = '\0';
		cur = trim(line);
		if (cur[0] == '\0' || cur[0] == '#' || cur[0] == ';')
			continue;

		/*
		 * systemd joins continued physical lines before parsing sections.
		 * Reject them even in ignored sections: otherwise a continuation
		 * could make us interpret text as a new directive or section.
		 */
		if (cur[strlen(cur) - 1] == '\\') {
			fprintf(stderr,
				"Error: unit line continuations are unsupported\n");
			goto out;
		}

		if (cur[0] == '[') {
			char *end = strchr(cur, ']');

			if (!end) {
				in_service = 0;
				continue;
			}
			*end = '\0';
			in_service = strcmp(cur + 1, "Service") == 0;
			continue;
		}

		if (!in_service)
			continue;

		eq = strchr(cur, '=');
		if (!eq)
			continue;
		*eq = '\0';
		if (handle_service_directive(cfg, trim(cur),
					     trim(eq + 1)) != 0)
			goto out;
	}

	if (ferror(file)) {
		fprintf(stderr, "Error: failed reading service file %s\n",
			path);
		goto out;
	}
	if (finalize_user(cfg) != 0)
		goto out;

	rc = 0;

out:
	free(line);
	fclose(file);
	if (rc != 0)
		free_config(cfg);
	return rc;
}

static void print_cap_set_line(const char *label, const cap_set_t *set)
{
	int cap;
	int first = 1;

	printf("    %s", label);
	if (!set->seen) {
		printf("not configured\n");
		return;
	}

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (!set->caps[cap])
			continue;
		if (!first)
			printf(" ");
		printf("%s", cap_name_safe(cap));
		first = 0;
	}
	if (first)
		printf("(empty)");
	printf("\n");
}

static const char *gid_name(gid_t gid)
{
	struct group *grp = getgrgid(gid);

	if (grp)
		return grp->gr_name;

	return NULL;
}

void print_service_config(const service_config_t *cfg)
{
	gid_t gid = 0;
	const char *group_name;
	char group_buf[32];

	if (!cfg)
		return;

	if (cfg->group_is_set)
		gid = cfg->group_gid;
	else if (cfg->user_is_set)
		gid = cfg->user_primary_gid;
	group_name = gid_name(gid);
	if (!group_name) {
		snprintf(group_buf, sizeof(group_buf), "%u",
			 (unsigned int)gid);
		group_name = group_buf;
	}

	printf("[*] Parsed systemd service configuration\n");
	if (state.service_file)
		printf("    Unit file: %s\n", state.service_file);
	printf("    Type: %s\n", cfg->service_type ?
	       cfg->service_type : "simple");
	if (cfg->service_type && !strcmp(cfg->service_type, "forking"))
		printf("[*] Warning: Type=forking detected. The traced "
		       "process may daemonize; capability events after fork "
		       "may be missed. Consider using a foreground flag if "
		       "available.\n");
	printf("    ExecStart: %s\n", cfg->exec_start ?
	       cfg->exec_start : "(not configured)");
	printf("    User: %s (uid=%u)\n", cfg->user_is_set ?
	       cfg->user_raw : "root", cfg->user_is_set ?
	       (unsigned int)cfg->user_uid : 0);
	printf("    Group: %s (gid=%u)\n", group_name, (unsigned int)gid);
	printf("    NoNewPrivileges: %s\n",
	       cfg->no_new_privs ? "yes" : "no");
	print_cap_set_line("AmbientCapabilities: ", &cfg->ambient);
	print_cap_set_line("CapabilityBoundingSet: ", &cfg->bounding);
}

static int stage_service_caps(const service_config_t *cfg, int non_root)
{
	capng_type_t type = CAPNG_INHERITABLE | CAPNG_AMBIENT;
	int cap;

	/*
	 * AmbientCapabilities applies to root services too. Replace inherited
	 * ambient state, but retain effective/permitted caps for root and !;
	 * only a non-root credential transition needs the reduced working set.
	 */
	capng_clear(CAPNG_SELECT_AMBIENT);
	if (non_root) {
		capng_clear(CAPNG_SELECT_CAPS);
		type |= CAPNG_EFFECTIVE | CAPNG_PERMITTED;
	}
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (!cfg->ambient.caps[cap])
			continue;
		if (capng_update(CAPNG_ADD, type, cap) != 0)
			return -1;
	}

	return 0;
}

static int stage_bounding_set(const service_config_t *cfg)
{
	int cap;

	if (!cfg->bounding.seen)
		return 0;

	capng_clear(CAPNG_SELECT_BOUNDS);
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (!cfg->bounding.caps[cap])
			continue;
		if (capng_update(CAPNG_ADD, CAPNG_BOUNDING_SET, cap) != 0)
			return -1;
	}

	return 0;
}

int apply_service_config(const service_config_t *cfg)
{
	capng_flags_t flags = CAPNG_NO_FLAG;
	int target_uid = -1;
	int target_gid = -1;
	int non_root = 0;
	int rc;

	if (!cfg)
		return 0;

	if ((cfg->user_is_set && !cfg->user_resolved) ||
	    (cfg->dynamic_user && !cfg->user_resolved)) {
		fprintf(stderr, "Error: service user is not resolved\n");
		return -1;
	}

	if (!cfg->exec_start_no_setuid && cfg->user_is_set) {
		gid_t gid = cfg->group_is_set ? cfg->group_gid :
			    cfg->user_primary_gid;

		if (!id_in_range((unsigned long)cfg->user_uid,
				 MAX_CHANGE_ID) ||
		    !id_in_range((unsigned long)gid, MAX_CHANGE_ID)) {
			fprintf(stderr,
				"Error: service credentials are out of range\n");
			return -1;
		}
		target_uid = (int)cfg->user_uid;
		target_gid = (int)gid;
		non_root = cfg->user_uid != 0;
		flags = (capng_flags_t)(flags | CAPNG_INIT_SUPP_GRP);
	} else if (!cfg->exec_start_no_setuid && cfg->group_is_set) {
		if (!id_in_range((unsigned long)cfg->group_gid,
				 MAX_CHANGE_ID)) {
			fprintf(stderr,
				"Error: service group is out of range\n");
			return -1;
		}
		target_gid = (int)cfg->group_gid;
	}

	if (capng_get_caps_process() != 0) {
		fprintf(stderr,
			"Error: failed to read child process capabilities\n");
		return -1;
	}

	if (stage_bounding_set(cfg) != 0) {
		fprintf(stderr, "Error: failed to stage bounding set\n");
		return -1;
	}
	if (cfg->bounding.seen)
		flags = (capng_flags_t)(flags | CAPNG_APPLY_BOUNDING);

	if (stage_service_caps(cfg, non_root) != 0) {
		fprintf(stderr, "Error: failed to stage ambient capabilities\n");
		return -1;
	}
	if (!cfg->ambient.seen || !cap_set_any(&cfg->ambient))
		flags = (capng_flags_t)(flags | CAPNG_CLEAR_AMBIENT);

	if (!cfg->exec_start_no_setuid && cfg->sup_groups.is_set &&
	    cfg->sup_groups.count > 0) {
		if (capng_stage_additional_groups(cfg->sup_groups.gids,
						  cfg->sup_groups.count) != 0) {
			fprintf(stderr,
				"Error: failed to stage supplementary groups\n");
			return -1;
		}
		flags = (capng_flags_t)(flags | CAPNG_APPLY_STAGED_GROUPS);
	}

	rc = capng_change_id(target_uid, target_gid, flags);
	if (rc != 0) {
		fprintf(stderr,
			"Error: failed to apply service credentials: %d\n",
			rc);
		return -1;
	}

	if (cfg->no_new_privs && prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
		fprintf(stderr, "Error: failed to set NoNewPrivileges: %s\n",
			strerror(errno));
		return -1;
	}

	return 0;
}
