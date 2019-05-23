#ifndef CONFCONF_ANALYSE_H
#define CONFCONF_ANALYSE_H

#include "tok.h"
#include "parse.h"

#include <stdbool.h>

/* 26 letters * 2 for case + 1 for '_' */
#define ANALYSE_MAX_BRANCH (26 * 2 + 1)

struct analyse_tree_s {
	bool is_terminal;
	char name[TOK_MAX_LEN];
	unsigned branch_count;
	char branch_chars[ANALYSE_MAX_BRANCH];
	struct analyse_tree_s *branches[ANALYSE_MAX_BRANCH];
};

struct analyse_result_s {
	bool uses_bool;
	bool uses_string;
	bool uses_id;
	bool uses_int;
	bool uses_intl;
	bool uses_intll;
	bool uses_uint;
	bool uses_uintl;
	bool uses_uintll;
	bool uses_float;
	bool uses_double;
	bool uses_doublel;
	struct analyse_tree_s deftype_tree;
	struct analyse_tree_s var_tree;
};

struct analyse_result_s analyse(struct parse_result_s pr);

void analyse_result_wipe(struct analyse_result_s *r);

#endif
