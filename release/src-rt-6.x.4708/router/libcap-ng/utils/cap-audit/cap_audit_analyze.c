// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * cap-audit - Trace a target process to discover required capabilities.
 * Copyright (c) 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * This software may be freely redistributed and/or modified under the
 * terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Authors:
 *   Steve Grubb <sgrubb@redhat.com>
 *   Portions of this code were made with codex 5.2
 */

/* Human-readable analysis output: capability summaries, deployment
 * recommendations, and wrapped terminal formatting helpers.
 */

#include "cap_audit.h"

#include <ctype.h>
#include <grp.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void cap_name_upper_buf(int cap, char *buf, size_t buf_len)
{
	const char *name = cap_name_safe(cap);
	size_t i;

	if (buf_len == 0)
		return;

	for (i = 0; name[i] && i + 1 < buf_len; i++)
		buf[i] = toupper((unsigned char)name[i]);
	buf[i] = '\0';
}

static int get_output_width(void)
{
	struct winsize ws;
	const char *columns;
	long env_width;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
		return ws.ws_col;

	columns = getenv("COLUMNS");
	if (columns) {
		env_width = strtol(columns, NULL, 10);
		if (env_width >= 40 && env_width <= 400)
			return (int)env_width;
	}

	return 80;
}

static void print_rule(char ch)
{
	int i;
	int width = get_output_width();

	if (width < 40)
		width = 40;

	for (i = 0; i < width; i++)
		putchar(ch);
	putchar('\n');
}

static void print_wrapped_text(const char *indent, const char *text)
{
	size_t indent_len;
	char *cont_indent;
	int width;
	int content_width;
	const char *p;
	int line_len = 0;

	if (!text) {
		printf("%s\n", indent);
		return;
	}

	indent_len = strlen(indent);
	cont_indent = malloc(indent_len + 1);
	if (!cont_indent) {
		printf("%s%s\n", indent, text);
		return;
	}
	memset(cont_indent, ' ', indent_len);
	cont_indent[indent_len] = '\0';

	width = get_output_width();
	if (width < 40)
		width = 40;
	content_width = width - (int)indent_len;
	if (content_width < 16)
		content_width = 16;

	printf("%s", indent);
	p = text;
	while (*p) {
		size_t word_len;
		int need_space = line_len > 0;

		while (*p == ' ')
			p++;

		if (*p == '\n') {
			putchar('\n');
			printf("%s", cont_indent);
			line_len = 0;
			p++;
			continue;
		}
		if (*p == '\0')
			break;

		word_len = strcspn(p, " \n");
		if (need_space &&
		    line_len + 1 + (int)word_len > content_width) {
			putchar('\n');
			printf("%s", cont_indent);
			line_len = 0;
			need_space = 0;
		}
		if (need_space) {
			putchar(' ');
			line_len++;
		}
		fwrite(p, 1, word_len, stdout);
		line_len += word_len;
		p += word_len;
	}
	putchar('\n');
	free(cont_indent);
}

static void print_wrappedf(const char *indent, const char *fmt, ...)
{
	va_list ap;
	char *buf;

	va_start(ap, fmt);
	if (vasprintf(&buf, fmt, ap) < 0)
		buf = NULL;
	va_end(ap);

	if (buf) {
		print_wrapped_text(indent, buf);
		free(buf);
	} else {
		print_wrapped_text(indent, "(formatting error)");
	}
}

static int cap_in_programmatic_set(int cap)
{
	if (state.capset_observed)
		return state.app.checks[cap].op_granted > 0;

	return state.app.checks[cap].granted > 0;
}

static int capset_only_count(void)
{
	int cap;
	int count = 0;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (cap_is_capset_only(cap))
			count++;
	}
	return count;
}

static void print_capset_requested_sets(int cap, const char *prefix)
{
	__u64 mask = 1ULL << cap;
	int first = 1;

	printf("%s", prefix);
	if (state.app.capset.effective & mask) {
		printf("effective");
		first = 0;
	}
	if (state.app.capset.permitted & mask) {
		printf("%spermitted", first ? "" : ", ");
		first = 0;
	}
	if (state.app.capset.inheritable & mask)
		printf("%sinheritable", first ? "" : ", ");
	printf("\n");
}

static void print_capset_only_section(void)
{
	int cap;
	int found = 0;

	printf("CAPSET-ONLY CAPABILITIES:\n");
	print_rule('-');
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (!cap_is_capset_only(cap))
			continue;

		printf("  %s (#%d)\n", cap_name_safe(cap), cap);
		print_capset_requested_sets(cap,
			"    Requested by successful capset in: ");
		printf("    Granted use observed: no\n");
		print_wrapped_text("    Assessment: ",
			"The current binary requested this capability in a "
			"capset that completed successfully, but no granted "
			"capability check was observed. This is a compatibility "
			"constraint, not confirmed functional use.");
		print_wrapped_text("    Recommendation: ",
			"Retain it in deployment capability sets for the current "
			"binary. Manually review the application, exercise relevant "
			"code paths, and remove it from the application's capset "
			"setup before removing it from the deployment boundary.");
		printf("\n");
		found = 1;
	}
	if (!found)
		printf("  None\n\n");
}

static bool svc_is_root_service(const service_config_t *cfg)
{
	return !cfg->user_is_set || cfg->user_uid == 0;
}

static gid_t svc_effective_gid(const service_config_t *cfg)
{
	if (cfg->group_is_set)
		return cfg->group_gid;
	if (cfg->user_is_set)
		return cfg->user_primary_gid;
	return 0;
}

static const char *svc_group_name(gid_t gid, char *buf, size_t buf_len)
{
	struct group *grp;

	grp = getgrgid(gid);
	if (grp)
		return grp->gr_name;

	snprintf(buf, buf_len, "%u", (unsigned int)gid);
	return buf;
}

static const char *svc_user_name(const service_config_t *cfg)
{
	if (cfg->user_is_set && cfg->user_raw)
		return cfg->user_raw;

	return "root";
}

static int svc_print_cap_list(const bool caps[CAP_LAST_CAP + 1])
{
	int cap;
	int first = 1;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (!caps[cap])
			continue;
		if (!first)
			printf(" ");
		printf("%s", cap_name_safe(cap));
		first = 0;
	}

	return !first;
}

static void svc_print_wrapped_cap_list(
	const char *label, const bool caps[CAP_LAST_CAP + 1])
{
	const int continuation = 8;
	int width = get_output_width();
	int current = strlen(label);
	int cap;
	int line_has_cap = 0;

	if (width < 40)
		width = 40;
	printf("%s", label);
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		const char *name;
		int name_len;

		if (!caps[cap])
			continue;
		name = cap_name_safe(cap);
		name_len = strlen(name);
		if (current + (line_has_cap ? 1 : 0) + name_len > width &&
		    current > continuation) {
			printf("\n%*s", continuation, "");
			current = continuation;
			line_has_cap = 0;
		}
		if (line_has_cap) {
			printf(" ");
			current++;
		}
		printf("%s", name);
		current += name_len;
		line_has_cap = 1;
	}
	printf("\n");
}

static void svc_print_cap_set(const cap_set_t *set)
{
	if (!set->seen) {
		printf("not configured (all capabilities inherited)");
		return;
	}

	if (!svc_print_cap_list(set->caps))
		printf("(none)");
}

static void svc_print_ambient_set(const cap_set_t *set)
{
	if (!set->seen) {
		printf("not configured (none)");
		return;
	}

	if (!svc_print_cap_list(set->caps))
		printf("(none)");
}

struct svc_deployment_caps {
	bool ambient[CAP_LAST_CAP + 1];
	bool bounding[CAP_LAST_CAP + 1];
};

struct svc_recommendations {
	struct svc_deployment_caps compatible;
	int configured_unobserved;
};

static int svc_cap_configured_unobserved(const service_config_t *cfg,
					 int cap)
{
	struct cap_check *check = &state.app.checks[cap];
	int in_bounding;
	int in_ambient;

	if (cap_requested_by_capset(cap) || cap_total_checks(check) > 0)
		return 0;

	in_bounding = cfg->bounding.seen && cfg->bounding.caps[cap];
	in_ambient = cfg->ambient.seen && cfg->ambient.caps[cap] &&
		     (!cfg->bounding.seen || in_bounding);

	return in_bounding || in_ambient;
}

static void svc_build_recommendations(const service_config_t *cfg,
				      struct svc_recommendations *caps)
{
	int cap;

	memset(caps, 0, sizeof(*caps));
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		/*
		 * A denied check may be advisory or precede a fallback, so it
		 * does not prove that the capability is required.
		 */
		if (cap_is_compat_requirement(cap)) {
			caps->compatible.ambient[cap] = true;
			caps->compatible.bounding[cap] = true;
		}

		if (!svc_cap_configured_unobserved(cfg, cap))
			continue;

		/*
		 * No observation can mean the run missed a feature-specific path.
		 * Preserve explicit unit intent in the generated configuration;
		 * absence of evidence is not enough to recommend removal.
		 */
		if (cfg->bounding.seen && cfg->bounding.caps[cap])
			caps->compatible.bounding[cap] = true;
		if (cfg->ambient.seen && cfg->ambient.caps[cap]) {
			caps->compatible.ambient[cap] = true;
			caps->compatible.bounding[cap] = true;
		}
		caps->configured_unobserved++;
	}
}

static void svc_print_reason(const struct cap_check *check)
{
	const char *reason = cap_union_reason(check);

	if (reason)
		printf(" (%s)", reason);
}

static void svc_print_needed_caps(void)
{
	int cap;
	int found = 0;

	printf("    Needed capabilities:");
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		struct cap_check *check = &state.app.checks[cap];

		if (!cap_required_union(check))
			continue;
		if (!found)
			printf("\n");
		printf("      %s", cap_name_safe(cap));
		svc_print_reason(check);
		printf("\n");
		found = 1;
	}
	if (!found)
		printf(" none\n");
}

static void svc_print_phase_caps(const char *label, int operational)
{
	int cap;
	int first = 1;

	printf("    %s", label);
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		struct cap_check *check = &state.app.checks[cap];
		unsigned long granted;

		granted = operational ? check->op_granted : check->granted;
		if (granted == 0)
			continue;
		if (!first)
			printf(" ");
		printf("%s", cap_name_safe(cap));
		first = 0;
	}
	if (first)
		printf("none");
	printf("\n");
}

static void print_outcome_error(const struct syscall_outcome *outcome,
				const char *syscall_name)
{
	enum syscall_outcome_class class;
	const char *result_name;
	int error;

	class = classify_syscall_outcome(outcome->result);
	if (class == SYSCALL_OUTCOME_SUCCESS) {
		printf("%s: %lu succeeded\n", syscall_name, outcome->count);
		return;
	}

	result_name = syscall_result_name(outcome->result);
	error = syscall_result_errno(outcome->result);
	printf("%s: %lu %s with ", syscall_name, outcome->count,
	       class == SYSCALL_OUTCOME_INTERRUPTED ? "interrupted" :
	       "failed");
	if (result_name)
		printf("-%s", result_name);
	else
		printf("%lld", (long long)outcome->result);
	if (error > 0 && error < 256)
		printf(" (%s)", strerror(error));
	printf("\n");
}

static void print_denied_outcomes(const struct cap_check *check,
				  const char *indent)
{
	size_t i;
	int found = 0;

	printf("%sOutcomes of syscall invocations containing such a check:\n",
	       indent);
	for (i = 0; i < check->outcome_count; i++) {
		const struct syscall_outcome *outcome = &check->outcomes[i];
		const char *syscall_name;
		char unknown[32];

		syscall_name = syscall_name_from_nr(outcome->syscall_nr);
		if (!syscall_name) {
			snprintf(unknown, sizeof(unknown), "unknown(#%d)",
				 outcome->syscall_nr);
			syscall_name = unknown;
		}
		printf("%s  ", indent);
		print_outcome_error(outcome, syscall_name);
		found = 1;
	}

	for (i = 0; i < check->denied_syscall_count; i++) {
		const char *syscall_name;
		char unknown[32];
		size_t j;

		for (j = 0; j < check->outcome_count; j++) {
			if (check->denied_syscalls[i] ==
			    check->outcomes[j].syscall_nr)
				break;
		}
		if (j < check->outcome_count)
			continue;

		syscall_name = syscall_name_from_nr(check->denied_syscalls[i]);
		if (!syscall_name) {
			snprintf(unknown, sizeof(unknown), "unknown(#%d)",
				 check->denied_syscalls[i]);
			syscall_name = unknown;
		}
		printf("%s  %s: outcome unavailable\n", indent, syscall_name);
		found = 1;
	}
	if (!found)
		printf("%s  unavailable\n", indent);
}

static const char *failure_word(unsigned long count)
{
	return count == 1 ? "failure" : "failures";
}

static const char *interruption_word(unsigned long count)
{
	return count == 1 ? "interruption" : "interruptions";
}

static void print_denial_assessment(const struct cap_check *check,
				    const char *indent)
{
	struct cap_outcome_summary summary;
	char evidence[320];
	unsigned long failed;

	summarize_cap_outcomes(check, &summary);
	switch (assess_cap_denials(check)) {
	case DENIAL_ASSESSMENT_PERMISSION:
		failed = summary.permission_failed + summary.other_failed;
		if (summary.other_failed > 0 && summary.interrupted > 0)
			snprintf(evidence, sizeof(evidence),
				 "Associated syscall invocations: %lu succeeded, "
				 "%lu failed, and %lu interrupted. Failure "
				 "categories: %lu permission-related; %lu other.",
				 summary.succeeded,
				 failed, summary.interrupted,
				 summary.permission_failed,
				 summary.other_failed);
		else if (summary.other_failed > 0)
			snprintf(evidence, sizeof(evidence),
				 "Associated syscall invocations: %lu succeeded and "
				 "%lu failed. Failure categories: %lu "
				 "permission-related; %lu other.",
				 summary.succeeded, failed,
				 summary.permission_failed,
				 summary.other_failed);
		else if (summary.interrupted > 0)
			snprintf(evidence, sizeof(evidence),
				 "Associated syscall invocations: %lu succeeded, "
				 "%lu failed, and %lu interrupted. All failures "
				 "were permission-related.", summary.succeeded,
				 summary.permission_failed,
				 summary.interrupted);
		else
			snprintf(evidence, sizeof(evidence),
				 "Associated syscall invocations: %lu succeeded and "
				 "%lu failed. All failures were permission-related.",
				 summary.succeeded,
				 summary.permission_failed);
		print_wrappedf(indent,
			"%s A permission failure does not prove that the "
			"capability check caused it. Manually investigate the "
			"permission-related failures and add the capability only "
			"if required functionality reproducibly fails because it "
			"is absent.", evidence);
		break;
	case DENIAL_ASSESSMENT_MIXED_INTERRUPTED:
		if (summary.other_failed > 0)
			print_wrappedf(indent,
				"Associated syscall outcomes: %lu succeeded, %lu "
				"non-permission %s, and %lu %s. Cap-audit cannot "
				"determine whether they came from the same call site "
				"or used the same arguments. Manual investigation is "
				"required to confirm that required functionality "
				"completed or was safely retried.", summary.succeeded,
				summary.other_failed,
				failure_word(summary.other_failed),
				summary.interrupted,
				interruption_word(summary.interrupted));
		else
			print_wrappedf(indent,
				"Associated syscall outcomes: %lu succeeded and %lu "
				"%s. Cap-audit cannot determine whether they came "
				"from the same call site or used the same arguments. "
				"Manual investigation is required to confirm that "
				"required functionality completed or was safely "
				"retried.", summary.succeeded, summary.interrupted,
				interruption_word(summary.interrupted));
		break;
	case DENIAL_ASSESSMENT_MIXED_OTHER:
		print_wrappedf(indent,
			"Associated syscall invocations: %lu succeeded and %lu "
			"failed for non-permission reasons. The not-granted "
			"capability checks are not established as the cause of "
			"those failures. The capability is not automatically "
			"recommended.", summary.succeeded, summary.other_failed);
		break;
	case DENIAL_ASSESSMENT_INCONCLUSIVE:
		if (check->outcome_count == 0)
			print_wrapped_text(indent,
				"Associated syscall outcomes are unavailable. Exercise "
				"the affected functionality again before making a "
				"capability decision.");
		else
			print_wrappedf(indent,
				"Available associated outcomes: %lu succeeded, %lu "
				"other %s, and %lu %s. The evidence remains "
				"inconclusive; exercise the affected functionality "
				"again before making a capability decision.",
				summary.succeeded, summary.other_failed,
				failure_word(summary.other_failed),
				summary.interrupted,
				interruption_word(summary.interrupted));
		break;
	case DENIAL_ASSESSMENT_SUCCEEDED:
		print_wrappedf(indent,
			"Associated syscall outcomes: %lu succeeded. The "
			"capability was not granted by those checks, but the "
			"associated syscall invocations completed. The capability "
			"is not automatically recommended.", summary.succeeded);
		break;
	case DENIAL_ASSESSMENT_OTHER:
		print_wrappedf(indent,
			"Associated syscall outcomes: %lu non-permission %s. The "
			"not-granted capability checks are not established as "
			"their cause.",
			summary.other_failed,
			failure_word(summary.other_failed));
		break;
	case DENIAL_ASSESSMENT_NONE:
		break;
	}
}

static const char *denial_review_label(enum denial_assessment assessment)
{
	switch (assessment) {
	case DENIAL_ASSESSMENT_PERMISSION:
	case DENIAL_ASSESSMENT_MIXED_INTERRUPTED:
		return "Manual investigation required";
	case DENIAL_ASSESSMENT_MIXED_OTHER:
		return "Omitted; successes and non-permission failures observed";
	case DENIAL_ASSESSMENT_INCONCLUSIVE:
		return "Additional evidence required";
	case DENIAL_ASSESSMENT_SUCCEEDED:
		return "Omitted; associated syscalls succeeded";
	case DENIAL_ASSESSMENT_OTHER:
		return "Omitted; capability check not established as cause";
	case DENIAL_ASSESSMENT_NONE:
		return NULL;
	}
	return NULL;
}

static void svc_print_denied_review(const service_config_t *cfg)
{
	int cap;
	int found = 0;
	int capset_overlap = 0;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		struct cap_check *check = &state.app.checks[cap];
		const char *label;

		if (cap_total_denied(check) == 0 ||
		    cap_total_granted(check) > 0)
			continue;

		if (!found)
			printf("    Capabilities with only not-granted checks:\n");
		label = denial_review_label(assess_cap_denials(check));
		printf("      %s: %s\n", cap_name_safe(cap),
		       label ? label : "no assessment");
		if (cfg->bounding.seen && !cfg->bounding.caps[cap])
			print_wrapped_text("        Note: ",
				"Capability is absent from the configured "
				"CapabilityBoundingSet.");
		if (cap_is_capset_only(cap))
			capset_overlap = 1;
		found = 1;
	}

	if (found) {
		print_wrapped_text("      ",
			"Detailed evidence appears in CAPABILITIES WITH ONLY "
			"NOT-GRANTED CHECKS above. A not-granted check does not "
			"add a capability automatically.");
		if (capset_overlap)
			print_wrapped_text("      ",
				"A listed capability is retained separately because a "
				"successful capset requested it.");
	} else {
		printf("    Capabilities with only not-granted checks: none\n");
	}
}

static void svc_print_recommended_denials(void)
{
	bool caps[CAP_LAST_CAP + 1] = { false };
	int cap;
	int found = 0;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		struct cap_check *check = &state.app.checks[cap];

		if (cap_total_denied(check) == 0 ||
		    !cap_required_union(check))
			continue;
		caps[cap] = true;
		found = 1;
	}

	if (!found)
		return;

	printf("    Mixed capability-check results: ");
	svc_print_cap_list(caps);
	printf("\n");
	print_wrapped_text("      ",
		"These capabilities had both granted and not-granted checks. "
		"They are included because granted use was observed; the "
		"not-granted checks do not change the generated configuration.");
}

static void svc_print_capset_only(const service_config_t *cfg)
{
	bool caps[CAP_LAST_CAP + 1] = { false };
	int cap;
	int found = 0;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (!cap_is_capset_only(cap))
			continue;
		caps[cap] = true;
		found = 1;
	}
	if (!found)
		return;

	printf("    Capset-only compatibility capabilities: ");
	svc_print_cap_list(caps);
	printf("\n");
	print_wrapped_text("      ",
		"The current-binary-compatible configuration retains these "
		"capabilities. Detailed application-cleanup guidance appears "
		"in CAPSET-ONLY CAPABILITIES above.");
	if (!cfg->bounding.seen)
		return;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		if (!caps[cap] || cfg->bounding.caps[cap])
			continue;
		print_wrappedf("      Warning: ",
			"%s is absent from the configured CapabilityBoundingSet. "
			"The observed capset may fail until the application stops "
			"requesting it or the capability is restored.",
			cap_name_safe(cap));
	}
}

static void svc_print_configured_unobserved(const service_config_t *cfg)
{
	bool ambient_and_bounding[CAP_LAST_CAP + 1] = { false };
	bool ambient_only[CAP_LAST_CAP + 1] = { false };
	bool bounding_only[CAP_LAST_CAP + 1] = { false };
	int cap;
	int found = 0;
	int found_ambient_and_bounding = 0;
	int found_ambient_only = 0;
	int found_bounding_only = 0;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		int in_ambient;
		int in_bounding;

		if (!svc_cap_configured_unobserved(cfg, cap))
			continue;

		in_ambient = cfg->ambient.seen && cfg->ambient.caps[cap];
		in_bounding = cfg->bounding.seen && cfg->bounding.caps[cap];
		if (in_ambient && in_bounding) {
			ambient_and_bounding[cap] = true;
			found_ambient_and_bounding = 1;
		} else if (in_ambient) {
			ambient_only[cap] = true;
			found_ambient_only = 1;
		} else {
			bounding_only[cap] = true;
			found_bounding_only = 1;
		}
		found = 1;
	}

	if (!found) {
		printf("    Configured but not observed in this run: none\n");
		return;
	}

	printf("    Configured but not observed in this run:\n");
	if (found_ambient_and_bounding)
		svc_print_wrapped_cap_list(
			"      AmbientCapabilities and CapabilityBoundingSet: ",
			ambient_and_bounding);
	if (found_ambient_only)
		svc_print_wrapped_cap_list(
			"      AmbientCapabilities only: ", ambient_only);
	if (found_bounding_only)
		svc_print_wrapped_cap_list(
			"      CapabilityBoundingSet only: ", bounding_only);
	print_wrapped_text("      Assessment: ",
		"No confirmed capability check or successful capset request was "
		"observed. Relevant functionality may not have been exercised.");
	print_wrapped_text("      Recommendation: ",
		"Retained in the current-service-compatible configuration. "
		"Treat these as removal candidates only after manual review and "
		"targeted testing.");
}

static void svc_print_generated_config(const service_config_t *cfg,
				       const char *group, int is_root,
				       const struct svc_deployment_caps *caps)
{
	printf("    [Service]\n");
	printf("    User=%s\n", svc_user_name(cfg));
	printf("    Group=%s\n", group);
	if (is_root)
		printf("    # AmbientCapabilities omitted for root service\n");
	else {
		printf("    AmbientCapabilities=");
		svc_print_cap_list(caps->ambient);
		printf("\n");
	}
	printf("    CapabilityBoundingSet=");
	svc_print_cap_list(caps->bounding);
	printf("\n");
	/*
	 * Enabling NoNewPrivileges was not tested when the service was traced
	 * without it, so preserve the setting that governed the audit.
	 */
	printf("    NoNewPrivileges=%s\n",
	       cfg->no_new_privs ? "yes" : "no");
}

/*
 * A configured/requested identity capability is not necessarily a capability
 * to retain after initialization. The unchanged daemon still needs it in its
 * deployment boundary to perform the transition; never suggest removing it
 * there merely because a matched keepcaps pair explains its observed use.
 */
static void svc_print_transition_caps(const service_config_t *cfg)
{
	int cap;

	if (!state.capset_observed || state.keepcaps_incomplete)
		return;
	for (cap = CAP_SETGID; cap <= CAP_SETUID; cap++) {
		if (!(state.keepcaps_init_caps & (1ULL << cap)) ||
		    state.app.checks[cap].op_count ||
		    !((cfg->bounding.seen && cfg->bounding.caps[cap]) ||
		      (cfg->ambient.seen && cfg->ambient.caps[cap]) ||
		      cap_requested_by_capset(cap)))
			continue;
		print_wrappedf("    Credential transition: ",
			"%s is requested by the unit or application, but is not "
			"needed for observed operation. Its KEEPCAPS-bracketed "
			"credential changes are initialization work; no "
			"operational checks were observed. Keep it available "
			"for startup in the deployment configuration, including "
			"CapabilityBoundingSet. Review and test unexercised paths "
			"before removing it from the application's retained set.",
			cap_name_safe(cap));
	}
}

static void print_service_recommendations(void)
{
	const service_config_t *cfg = state.service_cfg;
	struct svc_recommendations recommended;
	char group_buf[32];
	gid_t gid = svc_effective_gid(cfg);
	const char *group = svc_group_name(gid, group_buf, sizeof(group_buf));
	int is_root = svc_is_root_service(cfg);

	svc_build_recommendations(cfg, &recommended);

	printf("SYSTEMD SERVICE RECOMMENDATIONS:\n");
	print_rule('-');
	printf("  Current service context:\n");
	printf("    Unit file: %s\n", state.service_file);
	printf("    ExecStart: %s\n", cfg->exec_start ?
	       cfg->exec_start : "(not configured)");
	printf("    User: %s (uid=%u)\n", svc_user_name(cfg),
	       cfg->user_is_set ? (unsigned int)cfg->user_uid : 0);
	printf("    Group: %s (gid=%u)\n", group, (unsigned int)gid);
	printf("    NoNewPrivileges: %s\n",
	       cfg->no_new_privs ? "yes" : "no");
	printf("    AmbientCapabilities: ");
	svc_print_ambient_set(&cfg->ambient);
	printf("\n");
	printf("    CapabilityBoundingSet: ");
	svc_print_cap_set(&cfg->bounding);
	printf("\n\n");

	printf("  Analysis:\n");
	svc_print_needed_caps();
	if (state.capset_observed) {
		print_wrapped_text("    ",
				   "capset was observed. CapabilityBoundingSet "
				   "should include the union of initialization "
				   "and operational capabilities.");
		svc_print_phase_caps("Initialization capabilities: ", 0);
		svc_print_phase_caps("Operational capabilities: ", 1);
	}
	svc_print_transition_caps(cfg);
	if (is_root)
		print_wrapped_text("    ",
				   "AmbientCapabilities omitted because ambient "
				   "capabilities are not meaningful for root "
				   "services.");
	if (!cfg->no_new_privs)
		print_wrapped_text("    NoNewPrivileges: ",
			"The service was audited with this setting disabled. "
			"Enabling it was not validated, so the generated "
			"configuration retains NoNewPrivileges=no. Test the "
			"change separately, including functionality that starts "
			"child processes or helpers, before enabling it.");
	svc_print_denied_review(cfg);
	svc_print_recommended_denials();
	svc_print_capset_only(cfg);
	svc_print_configured_unobserved(cfg);
	printf("\n");

	if (recommended.configured_unobserved > 0)
		printf("  Recommended current-service-compatible [Service] "
		       "configuration:\n");
	else if (capset_only_count() > 0)
		printf("  Recommended current-binary-compatible [Service] "
		       "configuration:\n");
	else
		printf("  Recommended [Service] configuration:\n");
	svc_print_generated_config(cfg, group, is_root,
				   &recommended.compatible);
	printf("\n");
}

static void print_updatev_wrapped(const char *prefix, const char *cap_prefix,
				  const char *suffix)
{
	int width = get_output_width();
	size_t prefix_len = strlen(prefix);
	size_t suffix_len = strlen(suffix);
	int cur_len = prefix_len;
	int cont_indent = 8;
	int i;

	if (width < 40)
		width = 40;

	for (i = 0; prefix[i] == ' '; i++)
		;
	if (i > 0)
		cont_indent = i + 4;

	printf("%s", prefix);
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		char cap_name[64];
		char item[96];
		size_t item_len;

		if (!cap_in_programmatic_set(i))
			continue;

		cap_name_upper_buf(i, cap_name, sizeof(cap_name));
		snprintf(item, sizeof(item), "%s%s", cap_prefix, cap_name);
		item_len = strlen(item);

		if (cur_len > (int)prefix_len &&
		    cur_len + 2 + (int)item_len + (int)suffix_len > width) {
			printf(",\n%*s%s", cont_indent, "", item);
			cur_len = cont_indent + item_len;
		} else {
			printf(", %s", item);
			cur_len += 2 + item_len;
		}
	}

	if (cur_len + 2 + (int)suffix_len > width && cur_len > cont_indent) {
		printf(",\n%*s%s", cont_indent, "", suffix);
	} else {
		printf(", %s", suffix);
	}
}

static void print_denied_recommendation_review(void)
{
	int cap;
	int found = 0;
	int capset_overlap = 0;

	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		struct cap_check *check = &state.app.checks[cap];
		enum denial_assessment assessment;
		const char *label = NULL;

		if (cap_total_denied(check) == 0 ||
		    cap_total_granted(check) > 0)
			continue;
		if (!found)
			printf("  Capabilities with only not-granted checks:\n");

		assessment = assess_cap_denials(check);
		label = denial_review_label(assessment);
		if (!label)
			continue;
		printf("    %s: %s\n", cap_name_safe(cap), label);
		if (cap_is_capset_only(cap))
			capset_overlap = 1;
		found = 1;
	}

	if (found) {
		print_wrapped_text("    ",
			"A not-granted check does not add a capability "
			"automatically.");
		if (capset_overlap)
			print_wrapped_text("    ",
				"A listed capability is retained separately because a "
				"successful capset requested it.");
		printf("\n");
	}
}

static int cap_has_related_system_context(int cap)
{
	/*
	 * Global settings cannot be correlated with an individual check, so
	 * they remain diagnostic context and never drive recommendations.
	 * protected_symlinks is intentionally absent because that policy
	 * explicitly ignores CAP_DAC_OVERRIDE.
	 */
	switch (cap) {
	case CAP_SYS_PTRACE:
		return state.app.yama_ptrace_scope > 0 ||
		       state.app.suid_dumpable == 2;
	case CAP_PERFMON:
		return state.app.perf_event_paranoid >= 2;
	case CAP_BPF:
		return state.app.unprivileged_bpf_disabled == 1;
	case CAP_SYSLOG:
		return state.app.kptr_restrict >= 1 ||
		       state.app.dmesg_restrict >= 1;
	case CAP_SYS_MODULE:
		return state.app.modules_disabled == 1;
	case CAP_SYS_RAWIO:
		return state.app.mmap_min_addr > 0;
	case CAP_FOWNER:
		return state.app.protected_hardlinks == 1;
	default:
		return 0;
	}
}

static void print_system_context_value(const char *name, int value,
				       const char *effect)
{
	print_wrappedf("    ", "%s = %d: %s", name, value, effect);
}

static int print_capability_system_context(void)
{
	int cap;
	int count = 0;

	printf("CAPABILITY-RELATED SYSTEM CONTEXT:\n");
	print_rule('-');
	for (cap = 0; cap <= CAP_LAST_CAP; cap++) {
		char cap_name[64];

		if (cap_total_checks(&state.app.checks[cap]) == 0 ||
		    !cap_has_related_system_context(cap))
			continue;
		if (count == 0)
			print_wrapped_text("  ",
				"These settings govern operations that can use the "
				"listed capabilities. They cannot be correlated with a "
				"particular observed check and do not prove that a "
				"capability is required.");

		cap_name_upper_buf(cap, cap_name, sizeof(cap_name));
		printf("  CAP_%s\n", cap_name);
		switch (cap) {
		case CAP_SYS_PTRACE:
			if (state.app.yama_ptrace_scope > 0)
				print_system_context_value(
					"kernel.yama.ptrace_scope",
					state.app.yama_ptrace_scope,
					"may restrict ptrace access.");
			if (state.app.suid_dumpable == 2)
				print_system_context_value(
					"fs.suid_dumpable", state.app.suid_dumpable,
					"affects core dumps and ptrace of privileged "
					"programs.");
			break;
		case CAP_PERFMON:
			print_system_context_value(
				"kernel.perf_event_paranoid",
				state.app.perf_event_paranoid,
				"may restrict performance monitoring.");
			break;
		case CAP_BPF:
			print_system_context_value(
				"kernel.unprivileged_bpf_disabled",
				state.app.unprivileged_bpf_disabled,
				"restricts unprivileged BPF use.");
			break;
		case CAP_SYSLOG:
			if (state.app.kptr_restrict >= 1)
				print_system_context_value(
					"kernel.kptr_restrict",
					state.app.kptr_restrict,
					"restricts exposure of kernel pointers.");
			if (state.app.dmesg_restrict >= 1)
				print_system_context_value(
					"kernel.dmesg_restrict",
					state.app.dmesg_restrict,
					"restricts access to the kernel log.");
			break;
		case CAP_SYS_MODULE:
			print_system_context_value(
				"kernel.modules_disabled", state.app.modules_disabled,
				"permanently disables module loading; CAP_SYS_MODULE "
				"cannot override it.");
			break;
		case CAP_SYS_RAWIO:
			print_system_context_value(
				"vm.mmap_min_addr", state.app.mmap_min_addr,
				"restricts mappings below this address.");
			break;
		case CAP_FOWNER:
			print_system_context_value(
				"fs.protected_hardlinks",
				state.app.protected_hardlinks,
				"restricts creation of hard links to files not owned "
				"by the caller.");
			break;
		default:
			break;
		}
		printf("\n");
		count++;
	}
	if (count == 0)
		printf("  None\n\n");

	return count;
}

void analyze_capabilities(void)
{
	int has_required = 0;
	int has_denied = 0;
	int required_count = 0;
	int context_count;
	int denied_count = 0;
	int denied_manual = 0;
	int denied_mixed_other = 0;
	int denied_inconclusive = 0;
	int denied_succeeded = 0;
	int denied_other = 0;
	int capset_only = capset_only_count();
	unsigned long total_checks = 0;
	int i;
	int first;

	printf("\n");
	print_rule('=');
	print_wrappedf("", "CAPABILITY ANALYSIS FOR: %s (PID %d)",
		       state.app.exe, state.app.pid);
	print_rule('=');
	printf("\n");

	printf("SYSTEM CONTEXT:\n");
	print_rule('-');
	printf("  Kernel version: %s\n", state.app.kernel_version);
	printf("  kernel.yama.ptrace_scope: %d\n", state.app.yama_ptrace_scope);
	printf("  kernel.kptr_restrict: %d\n", state.app.kptr_restrict);
	printf("  kernel.dmesg_restrict: %d\n", state.app.dmesg_restrict);
	printf("  kernel.modules_disabled: %d\n", state.app.modules_disabled);
	printf("  kernel.perf_event_paranoid: %d\n",
	       state.app.perf_event_paranoid);
	printf("  kernel.unprivileged_bpf_disabled: %d\n",
	       state.app.unprivileged_bpf_disabled);
	printf("  net.core.bpf_jit_enable: %d\n", state.app.bpf_jit_enable);
	printf("  net.core.bpf_jit_harden: %d\n", state.app.bpf_jit_harden);
	printf("  net.core.bpf_jit_kallsyms: %d\n",
	       state.app.bpf_jit_kallsyms);
	printf("  vm.mmap_min_addr: %d\n", state.app.mmap_min_addr);
	printf("  fs.protected_hardlinks: %d\n", state.app.protected_hardlinks);
	printf("  fs.protected_symlinks: %d\n", state.app.protected_symlinks);
	printf("  fs.suid_dumpable: %d\n", state.app.suid_dumpable);
	printf("\n");

	printf("REQUIRED CAPABILITIES:\n");
	print_rule('-');
	if (!state.capset_observed) {
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check;

			check = &state.app.checks[i];
			if (check->granted > 0) {
				has_required = 1;
				printf("  %s (#%d)\n", cap_name_safe(i), i);
				printf("    Checks: %lu granted, %lu denied\n",
				       check->granted, check->denied);
				if (check->reason)
					print_wrappedf("    Reason: ",
						       "%s", check->reason);
				printf("\n");
			}
		}
		if (!has_required)
			print_wrapped_text("  ",
				"None - No confirmed granted capability use was "
				"observed.\n");
	} else {
		print_wrapped_text("",
				   "INITIALIZATION CAPABILITIES (before capability drop):");
		if (state.keepcaps_init_caps)
			print_wrapped_text("  ",
				"Includes UID/GID changes within completed KEEPCAPS "
				"transitions, even after an intermediate capset.");
		print_rule('-');
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check = &state.app.checks[i];

			if (check->granted > 0) {
				has_required = 1;
				printf("  %s (#%d)\n", cap_name_safe(i), i);
				printf("    Checks: %lu granted, %lu denied\n",
				       check->granted, check->denied);
				if (check->reason)
					print_wrappedf("    Reason: ",
						       "%s", check->reason);
				printf("\n");
			}
		}
		if (!has_required)
			printf("  None\n\n");

		print_wrapped_text("",
				   "OPERATIONAL CAPABILITIES (after capability drop):");
		print_rule('-');
		has_required = 0;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			struct cap_check *check = &state.app.checks[i];

			if (check->op_granted > 0) {
				has_required = 1;
				printf("  %s (#%d)\n", cap_name_safe(i), i);
				printf("    Checks: %lu granted, %lu denied\n",
				       check->op_granted, check->op_denied);
				if (check->op_reason)
					print_wrappedf("    Reason: ",
						       "%s", check->op_reason);
				printf("\n");
			}
		}
		if (!has_required)
			printf("  None\n\n");
	}

	print_capset_only_section();

	context_count = print_capability_system_context();

	printf("CAPABILITIES WITH ONLY NOT-GRANTED CHECKS:\n");
	print_rule('-');
	for (i = 0; i <= CAP_LAST_CAP; i++) {
		struct cap_check *check;

		check = &state.app.checks[i];
		if (cap_total_denied(check) > 0 &&
		    cap_total_granted(check) == 0) {
			has_denied = 1;
			printf("  %s (#%d)\n", cap_name_safe(i), i);
			printf("    Capability checks returning not granted: %lu\n",
			       cap_total_denied(check));
			print_denied_outcomes(check, "    ");
			print_denial_assessment(check, "    Assessment: ");
			if (cap_is_capset_only(i))
				print_wrapped_text("    Recommendation: ",
					"Not added based on the not-granted checks. "
					"Retained separately in compatible deployment "
					"configurations because a successful capset "
					"requested it.");
			else
				print_wrapped_text("    Recommendation: ",
						   "Not added automatically.");
			printf("\n");
		}
	}
	if (!has_denied)
		printf("  None\n\n");

	for (i = 0; i <= CAP_LAST_CAP; i++) {
		total_checks += cap_total_checks(&state.app.checks[i]);
		if (cap_required_union(&state.app.checks[i]))
			required_count++;
		if (cap_total_denied(&state.app.checks[i]) > 0 &&
		    cap_total_granted(&state.app.checks[i]) == 0) {
			enum denial_assessment assessment;

			denied_count++;
			assessment = assess_cap_denials(&state.app.checks[i]);
			switch (assessment) {
			case DENIAL_ASSESSMENT_PERMISSION:
			case DENIAL_ASSESSMENT_MIXED_INTERRUPTED:
				denied_manual++;
				break;
			case DENIAL_ASSESSMENT_INCONCLUSIVE:
				denied_inconclusive++;
				break;
			case DENIAL_ASSESSMENT_MIXED_OTHER:
				denied_mixed_other++;
				break;
			case DENIAL_ASSESSMENT_SUCCEEDED:
				denied_succeeded++;
				break;
			case DENIAL_ASSESSMENT_OTHER:
				denied_other++;
				break;
			case DENIAL_ASSESSMENT_NONE:
				break;
			}
		}
	}

	printf("SUMMARY:\n");
	print_rule('-');
	printf("  Total capability checks: %lu\n", total_checks);
	printf("  Required capabilities: %d\n", required_count);
	printf("  Capabilities with related system context: %d\n",
	       context_count);
	printf("  Successful capset calls: %lu\n",
	       state.app.capset.successful_calls);
	printf("  Capset-only capabilities: %d\n", capset_only);
	printf("  Capabilities with only not-granted checks: %d\n",
	       denied_count);
	if (denied_count > 0) {
		printf("    Assessment counts:\n");
		if (denied_manual > 0)
			printf("      Manual investigation required: %d\n",
			       denied_manual);
		if (denied_inconclusive > 0)
			printf("      Additional evidence required: %d\n",
			       denied_inconclusive);
		if (denied_mixed_other > 0)
			printf("      Omitted after mixed success/non-permission "
			       "failure: %d\n", denied_mixed_other);
		if (denied_succeeded > 0)
			printf("      Omitted after associated syscall success: %d\n",
			       denied_succeeded);
		if (denied_other > 0)
			printf("      Capability check not established as cause: %d\n",
			       denied_other);
	}
	printf("\n");

	if (state.foreign_target_ns_observed) {
		printf("RECOMMENDATIONS:\n");
		print_rule('-');
		print_wrapped_text("  Warning: ",
				   "Capability checks targeted a different user "
				   "namespace. Automatic capability recommendations "
				   "are suppressed because their safe scope cannot "
				   "be determined.");
		printf("\n");
	} else if (state.service_cfg) {
		print_service_recommendations();
	} else if (required_count > 0 || capset_only > 0) {
		printf("RECOMMENDATIONS:\n");
		print_rule('-');
		print_denied_recommendation_review();
		if (state.app.prog_type != UNSUPPORTED && required_count > 0) {
			printf("  Programmatic solution (%s):\n",
			       state.app.prog_type == ELF ?
			       "C with libcap-ng" :
			       "Python with python3-libcap-ng");
			if (state.capset_observed)
				print_wrapped_text("    Note: ",
						   "The application drops capabilities after initialization. The programmatic snippet reflects the operational set only.");

			if (state.app.prog_type == ELF) {
				printf("    #include <cap-ng.h>\n");
				printf("    #include <stdio.h>\n");
				printf("    #include <stdlib.h>\n");
				printf("    ...\n");
				printf("    capng_clear(CAPNG_SELECT_BOTH);\n");
				print_updatev_wrapped("    capng_updatev(CAPNG_ADD, "
						      "CAPNG_EFFECTIVE|CAPNG_PERMITTED",
						      "CAP_", "-1);\n");
				printf("    if (capng_change_id(uid, gid, "
				       "CAPNG_DROP_SUPP_GRP | "
				       "CAPNG_CLEAR_BOUNDING)) {\n");
				printf("\tperror(\"capng_change_id\");\n");
				/* Failure can leave partial privileges. */
				printf("\texit(EXIT_FAILURE);\n");
				printf("    }\n\n");
			} else if (state.app.prog_type == PYTHON) {
				printf("    import sys\n");
				printf("    import _capng as capng\n");
				printf("    ...\n");
				printf("    capng.capng_clear(capng.CAPNG_SELECT_BOTH)\n");
				print_updatev_wrapped("    capng.capng_updatev("
						      "capng.CAPNG_ADD, "
						      "capng.CAPNG_EFFECTIVE|"
						      "capng.CAPNG_PERMITTED",
						      "capng.", "-1)\n");
				printf("    e = capng.capng_change_id(uid, gid, "
				       "capng.CAPNG_DROP_SUPP_GRP | "
				       "capng.CAPNG_CLEAR_BOUNDING)\n");
				printf("    if e < 0:\n");
				printf("\tprint(f\"Error: {e}\")\n");
				printf("\tsys.exit(1)\n\n");
			}
		}

		if (capset_only > 0)
			print_wrapped_text("  Compatibility note: ",
				"Deployment snippets retain capset-only capabilities "
				"so the observed capset calls can still succeed. Remove "
				"them from application capability setup before narrowing "
				"these deployment sets.");
		printf("  For systemd service%s:\n",
		       capset_only > 0 ? " (current-binary-compatible)" : "");
		if (state.capset_observed)
			print_wrapped_text("    Note: ",
					   "Ambient capabilities must include initialization requirements. The application drops to the operational set internally via capset.");
		printf("    [Service]\n");
		printf("    User=<non-root-user>\n");
		printf("    Group=<non-root-group>\n");
		printf("    AmbientCapabilities=");
		first = 1;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_is_compat_requirement(i)) {
				if (!first)
					printf(" ");
				printf("%s", cap_name_safe(i));
				first = 0;
			}
		}
		printf("\n");
		printf("    CapabilityBoundingSet=");
		first = 1;
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_is_compat_requirement(i)) {
				if (!first)
					printf(" ");
				printf("%s", cap_name_safe(i));
				first = 0;
			}
		}
		printf("\n\n");

		printf("  For file capabilities (via filecap):\n");
		if (state.capset_observed)
			print_wrapped_text("    Note: ",
					   "File capabilities must include initialization requirements. The application drops to the operational set internally via capset.");
		printf("    filecap /path/to/binary");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_is_compat_requirement(i))
				printf(" %s", cap_name_safe(i));
		}
		printf("\n\n");

		printf("  For Docker/Podman:\n");
		if (state.capset_observed)
			print_wrapped_text("    Note: ",
					   "Container capabilities must include initialization requirements. The application drops to the operational set internally via capset.");
		printf("    docker run --user $(id -u):$(id -g) \\\n");
		printf("      --cap-drop=ALL \\\n");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_is_compat_requirement(i))
				printf("      --cap-add=%s \\\n",
				       cap_name_safe(i));
		}
		printf("      your-image:tag\n\n");

		printf("  For Kubernetes:\n");
		if (state.capset_observed)
			print_wrapped_text("    Note: ",
					   "Container capabilities must include initialization requirements. The application drops to the operational set internally via capset.");
		printf("    securityContext:\n");
		printf("      runAsUser: 1000\n");
		printf("      runAsGroup: 1000\n");
		printf("      capabilities:\n");
		printf("        drop:\n");
		printf("          - ALL\n");
		printf("        add:\n");
		for (i = 0; i <= CAP_LAST_CAP; i++) {
			if (cap_is_compat_requirement(i))
				printf("          - %s\n", cap_name_safe(i));
		}
		printf("\n");
	} else {
		printf("RECOMMENDATIONS:\n");
		print_rule('-');
		if (denied_count > 0) {
			print_wrapped_text("  ",
				"No capabilities have confirmed granted use, so none "
				"are recommended automatically.");
			print_denied_recommendation_review();
		} else {
			print_wrapped_text("  ",
				"This application does not require any elevated "
				"capabilities!");
			print_wrapped_text("  ",
				"Run as an unprivileged user with no special "
				"capabilities.");
		}
		printf("\n");
	}

	print_wrapped_text("EXPERIMENTAL NOTICE: ",
			   "cap-audit output is experimental, but very close.");
}
