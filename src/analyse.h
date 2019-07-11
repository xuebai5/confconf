#ifndef CONFCONF_ANALYSE_H
#define CONFCONF_ANALYSE_H

#include "tok.h"
#include "parse.h"

#include "../reqs/uthash/include/uthash.h"

#include <stdbool.h>

/* 26 letters * 2 for case + 1 for '_' */
#define ANALYSE_MAX_BRANCH (26 * 2 + 1)

struct analyse_trie_s {
	/* NULL for non-terminals */
	char *sval;
	unsigned branch_count;
	char branch_chars[ANALYSE_MAX_BRANCH];
	struct analyse_trie_s *branches[ANALYSE_MAX_BRANCH];
	/* these two used for traversal */
	struct analyse_trie_s *parent;
	unsigned idx;
};

struct analyse_result_s {
	bool uses_type[PARSE_TYPE_HASH_DEFTYPE];
	bool uses_array;
	bool uses_hash;
	struct analyse_trie_s var_trie;
	unsigned deftype_mem_trie_count;
	struct analyse_trie_s *deftype_mem_tries;
};

struct analyse_result_s* analyse(struct parse_result_s *pr);

void analyse_result_free(struct analyse_result_s *ar);

#endif
