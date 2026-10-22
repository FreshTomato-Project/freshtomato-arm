// SPDX-License-Identifier: GPL-2.0-or-later
/* Exercise pscap's own node formatting without depending on host processes. */
#define main pscap_main
#include "../pscap.c"
#undef main

static void fail(const char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}

int main(void)
{
	const char *expected =
		"worker(12345:codex) [none]\n"
		"\\x1B\\x1B\\x1B\\x1B\\x1B"
		"\\x1B\\x1B\\x1B\\x1B\\x1B"
		"\\x1B\\x1B\\x1B\\x1B\\x1B"
		"(12345:account-name-at-the-buffer-edge) "
		"[cap_chown, cap_net_raw]\n";
	struct proc_info proc = {
		.pid = 12345, .cmd = "worker", .account = "codex",
		.caps_text = "none"
	};
	struct proc_tree tree = { &proc, 1, 240 };
	char comm[16], output[512];
	char *escaped;
	FILE *file = tmpfile();
	int saved = dup(STDOUT_FILENO);
	size_t len;

	if (!file || saved < 0 || fflush(stdout) ||
	    dup2(fileno(file), STDOUT_FILENO) < 0)
		fail("Cannot capture pscap output");
	print_tree_node(&tree, &proc, NULL, true);

	/* A legal 15-byte comm can expand to 60 bytes after escaping. */
	memset(comm, '\033', sizeof(comm) - 1);
	comm[sizeof(comm) - 1] = '\0';
	escaped = sanitize_untrusted_field(comm);
	if (!escaped)
		fail("Cannot escape process name");
	proc.cmd = escaped;
	strcpy(proc.account, "account-name-at-the-buffer-edge");
	proc.caps_text = "cap_chown, cap_net_raw";
	print_tree_node(&tree, &proc, NULL, true);
	free(escaped);

	if (fflush(stdout) || dup2(saved, STDOUT_FILENO) < 0)
		fail("Cannot restore stdout");
	close(saved);
	rewind(file);
	len = fread(output, 1, sizeof(output) - 1, file);
	output[len] = '\0';
	fclose(file);
	if (strcmp(output, expected)) {
		fprintf(stderr, "Expected:\n%sActual:\n%s", expected, output);
		fail("Process name, PID, account, or capabilities were truncated");
	}
	return 0;
}
