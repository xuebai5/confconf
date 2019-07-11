#include "analyse.h"

#include "err.h"

#include <assert.h>
#include <string.h>

static void sub_trie_insert(struct analyse_trie_s *t,
	const char *s, char *sv)
{
	const char *si = s;
	struct analyse_trie_s *parent = NULL;
	unsigned i;

	/* walk down the trie, creating nodes when necessary */
	for (; *si != '\0'; si++) {
		for (i = 0; i < t->branch_count && *si != t->branch_chars[i]; i++);
		if (t->branch_count == 0
			|| i == t->branch_count
		) {
			t->branch_count++;
			t->branch_chars[t->branch_count - 1] = *si;
			TRYALLOC(t->branches[t->branch_count - 1], 1);
			parent = t;
			t = t->branches[t->branch_count - 1];
			t->parent = parent;
			t->branch_count = 0;
			t->sval = NULL;
		} else {
			t = t->branches[i];
		}
	}

	TRYALLOC(t->sval, strlen(sv) + 1);
	strcpy(t->sval, sv);
}

/* tries used to generate static jump-based token matching */
static inline void sub_build_tries(struct parse_result_s *pr,
	struct analyse_result_s *ar)
{
	struct parse_deftype_s *dcur, *dtmp;
	struct parse_var_s *vcur, *vtmp;
	unsigned i, j;
	/* dtype sval == TOK_MAX_LEN
	 * dtype member == TOK_MAX_LEN
	 * suffix == TOK_MAX_LEN
	 * 32 == padding for "CONFCONF_TYPE_" + "_" + "_" + "\0" */
	char sbuf[TOK_MAX_LEN * 3 + 32];

	/* variable trie */
	ar->var_trie.branch_count = 0;
	ar->var_trie.sval = NULL;
	ar->var_trie.parent = NULL;

	if (pr->vars != NULL) {
		HASH_ITER(hh, pr->vars, vcur, vtmp) {
			sprintf(sbuf, "%s", vcur->name);
			sub_trie_insert(&(ar->var_trie), vcur->name, sbuf);
		}
	}

	ar->deftype_mem_tries = NULL;

	if (pr->deftypes != NULL) {
		TRYALLOC(
			ar->deftype_mem_tries,
			HASH_COUNT(pr->deftypes)
		);

		ar->deftype_mem_trie_count = HASH_COUNT(pr->deftypes);

		i = 0;
		HASH_ITER(hh, pr->deftypes, dcur, dtmp) {
			ar->deftype_mem_tries[i].branch_count = 0;
			ar->deftype_mem_tries[i].sval = NULL;
			ar->deftype_mem_tries[i].parent = NULL;

			if (!dcur->is_used) {
				i++;
				continue;
			}

			/* union types */
			if (dcur->type == PARSE_DEFTYPE_UNION) {
				for (j = 0; j < dcur->member_list_len; j++) {
					sprintf(sbuf, "%u", j);

					sub_trie_insert(
						&(ar->deftype_mem_tries[i]),
						dcur->member_name_list[j],
						sbuf
					);
				}
			}

			/* enum mems */
			if (dcur->type == PARSE_DEFTYPE_ENUM) {
				for (j = 0; j < dcur->member_list_len; j++) {
					sprintf(sbuf,
						"CONFCONF_TYPE_%s_%s_%s",
						dcur->name,
						dcur->member_name_list[j],
						pr->suffix
					);
					sub_trie_insert(
						&(ar->deftype_mem_tries[i]),
						dcur->member_name_list[j],
						sbuf
					);
				}
			}

			i++;
		}
	}
}

static inline void sub_calculate_used_types(struct parse_result_s *pr,
	struct analyse_result_s *ar)
{
	struct parse_deftype_s *dcur, *dtmp;
	struct parse_var_s *vcur, *vtmp;
	unsigned i;

	for (i = PARSE_TYPE_BOOL; i < PARSE_TYPE_HASH_DEFTYPE; i++)
		ar->uses_type[i] = false;

	HASH_ITER(hh, pr->deftypes, dcur, dtmp) {
		if (!dcur->is_used)
			continue;

		if (dcur->type == PARSE_DEFTYPE_ENUM)
			continue;

		for (i = 0; i < dcur->member_list_len; i++) {
			if (dcur->member_type_list[i] >= PARSE_TYPE_HASH_BOOL) {
				ar->uses_hash = true;
				ar->uses_type[dcur->member_type_list[i]
					- PARSE_TYPE_HASH_BOOL] = true;
			} else if (dcur->member_type_list[i] >= PARSE_TYPE_ARRAY_BOOL) {
				ar->uses_array = true;
				ar->uses_type[dcur->member_type_list[i]
					- PARSE_TYPE_ARRAY_BOOL] = true;
			}

			ar->uses_type[dcur->member_type_list[i]] = true;
		}
	}

	HASH_ITER(hh, pr->vars, vcur, vtmp) {
		if (vcur->type >= PARSE_TYPE_HASH_BOOL) {
			ar->uses_hash = true;
			ar->uses_type[vcur->type - PARSE_TYPE_HASH_BOOL] = true;
		} else if (vcur->type >= PARSE_TYPE_ARRAY_BOOL) {
			ar->uses_array = true;
			ar->uses_type[vcur->type - PARSE_TYPE_ARRAY_BOOL] = true;
		}

		if (vcur->type == PARSE_TYPE_DEFTYPE
			|| vcur->type == PARSE_TYPE_ARRAY_DEFTYPE
			|| vcur->type == PARSE_TYPE_HASH_DEFTYPE
		) {
			continue;
		}

		ar->uses_type[vcur->type] = true;
	}
}

struct analyse_result_s* analyse(struct parse_result_s *pr)
{
	struct analyse_result_s *ar;

	assert(pr != NULL);

	TRYALLOC(ar, 1);
	ar->uses_hash = false;
	ar->uses_array = false;

	sub_build_tries(pr, ar);

	sub_calculate_used_types(pr, ar);

	return ar;
}

static void sub_wipe_trie(struct analyse_trie_s *t)
{
	struct analyse_trie_s *cur = t, *prev = NULL;

	assert(t != NULL);

	t->parent = NULL;
	t->idx = 0;

	/* do nothing for an empty trie */
	if (t->branch_count == 0)
		return;

	do {
		/* drop into a branch to its tail */
		if (cur->idx != cur->branch_count) {
			prev = cur;
			cur = cur->branches[cur->idx];
			cur->idx = 0;
			prev->idx++;
			continue;
		}

		/* break at the head node */
		if (cur->parent == NULL)
			break;

		/* at a tail. jump up to its parent */
		prev = cur;
		cur = cur->parent;

		/* prev guaranteed to have no more children here */
		if (prev->sval != NULL)
			free(prev->sval);

		free(prev);
	} while (true);
}

void analyse_result_free(struct analyse_result_s *ar)
{
	unsigned i;

	assert(ar != NULL);

	sub_wipe_trie(&(ar->var_trie));

	if (ar->deftype_mem_tries != NULL) {
		for (i = 0; i < ar->deftype_mem_trie_count; i++) {
			sub_wipe_trie(&(ar->deftype_mem_tries[i]));
		}

		free(ar->deftype_mem_tries);
	}

	free(ar);
}
