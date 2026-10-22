// SPDX-License-Identifier: GPL-2.0-or-later
/* utility_logic_test.c -- direct tests against utility translation units
 * Copyright 2026 Red Hat Inc.
 * All Rights Reserved.
 *
 * Exercise shared output directly and link netcap's local parsing helpers
 * with main() compiled out. Tests must cover the paths the utilities use.
 */

#include "config.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proc-llist.h"
#include "proc-output.h"

int parse_u32_hex_or_dec(const char *s, unsigned int *out);

static void fail(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static void test_tree_output(void)
{
	const char *expected =
		"cap_chown, \ncap_setuid\n"
		"alpha beta \ngamma\n"
		"abcdefghij\nklmn\n"
		"\033[31malpha beta \033[0m\n\033[31mgamma\033[0m\n"
		"root\n├─ child\n│  └─ grandchild\n└─ last\n";
	char prefix[32], long_prefix[601], output[2048];
	FILE *file = tmpfile();
	int saved = dup(STDOUT_FILENO);
	size_t len;

	if (!file || saved < 0 || fflush(stdout) ||
	    dup2(fileno(file), STDOUT_FILENO) < 0)
		fail("Cannot capture tree output");
	proc_print_wrapped("", "", "cap_chown, cap_setuid", 12);
	proc_print_wrapped("", "", "alpha beta gamma", 12);
	proc_print_wrapped("", "", "abcdefghijklmn", 10);
	proc_print_wrapped("", "", "\033[31malpha beta gamma\033[0m", 12);
	proc_tree_print_node(NULL, 1, "root", 80);
	proc_tree_build_child_prefix(prefix, sizeof(prefix), NULL, 1);
	if (prefix[0])
		fail("Root should not add a branch");
	proc_tree_print_node(prefix, 0, "child", 80);
	proc_tree_build_child_prefix(prefix, sizeof(prefix), "", 0);
	proc_tree_print_node(prefix, 1, "grandchild", 80);
	proc_tree_print_node("", 1, "last", 80);
	proc_tree_build_child_prefix(prefix, sizeof(prefix), "", 1);
	if (strcmp(prefix, "   "))
		fail("Last sibling should not extend the vertical branch");
	/* Recursive pscap trees must not inherit netcap's old 512-byte cap. */
	memset(long_prefix, ' ', sizeof(long_prefix) - 1);
	long_prefix[sizeof(long_prefix) - 1] = '\0';
	proc_tree_print_node(long_prefix, 1, "deep", 80);
	if (fflush(stdout) || dup2(saved, STDOUT_FILENO) < 0)
		fail("Cannot restore stdout");
	close(saved);
	rewind(file);
	len = fread(output, 1, sizeof(output) - 1, file);
	output[len] = '\0';
	fclose(file);
	if (strncmp(output, expected, strlen(expected)) ||
	    strncmp(output + strlen(expected), long_prefix,
		    strlen(long_prefix)) ||
	    strcmp(output + strlen(expected) + strlen(long_prefix), "└─ deep\n"))
		fail("Unexpected wrapped tree output");
}

static void test_parse_u32_hex_or_dec(void)
{
	unsigned int out;

	/* netcap accepts decimal, 0x-prefixed hex, and some procfs hex forms. */
	if (parse_u32_hex_or_dec("123", &out) != 0 || out != 123)
		fail("decimal parse failed");
	if (parse_u32_hex_or_dec("0x10", &out) != 0 || out != 16)
		fail("hex parse with prefix failed");
	if (parse_u32_hex_or_dec("0010", &out) != 0 || out != 16)
		fail("hex parse with leading zero failed");
	if (parse_u32_hex_or_dec("G1", &out) == 0)
		fail("invalid parse should fail");
	if (parse_u32_hex_or_dec("4294967295", &out) != 0 || out != UINT_MAX)
		fail("maximum u32 parse failed");
	if (parse_u32_hex_or_dec("4294967296", &out) == 0)
		fail("overflow parse should fail");
	if (parse_u32_hex_or_dec("+1", &out) == 0)
		fail("positive sign parse should fail");
	if (parse_u32_hex_or_dec("-1", &out) == 0)
		fail("negative parse should fail");
}

static void test_list_inode_iteration(void)
{
	llist list;
	lnode first = { 0 };
	lnode second = { 0 };
	lnode third = { 0 };
	lnode *cur;

	list_create(&list);

	first.inode = 99;
	first.cmd = strdup("first");
	first.capabilities = strdup("cap_net_bind_service");
	first.bounds = strdup("");
	first.ambient = strdup("");
	second.inode = 99;
	second.cmd = strdup("second");
	second.capabilities = strdup("cap_net_admin");
	second.bounds = strdup("");
	second.ambient = strdup("");
	third.inode = 100;
	third.cmd = strdup("third");
	third.capabilities = strdup("cap_sys_admin");
	third.bounds = strdup("");
	third.ambient = strdup("");

	if (!first.cmd || !first.capabilities || !first.bounds ||
	    !first.ambient || !second.cmd || !second.capabilities ||
	    !second.bounds || !second.ambient || !third.cmd ||
	    !third.capabilities || !third.bounds || !third.ambient)
		fail("allocation failed in inode iteration test");

	list_append(&list, &first);
	list_append(&list, &second);
	list_append(&list, &third);

	cur = list_find_inode(&list, 99);
	if (!cur || strcmp(cur->cmd, "first") != 0)
		fail("list_find_inode should return first matching inode");
	cur = list_next_inode(&list, 99);
	if (!cur || strcmp(cur->cmd, "second") != 0)
		fail("list_next_inode should return later matching inode");
	if (list_next_inode(&list, 99) != NULL)
		fail("list_next_inode should stop at the last match");

	list_clear(&list);
}

int main(void)
{
	test_tree_output();
	test_parse_u32_hex_or_dec();
	test_list_inode_iteration();
	puts("Direct utility logic tests passed");
	return 0;
}
