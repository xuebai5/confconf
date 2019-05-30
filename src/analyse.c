#include "err.h"
#include "analyse.h"

#include <assert.h>
#include <string.h>

struct analyse_result_s analyse(struct parse_result_s pr)
{
	struct analyse_result_s ar = {
		.uses_hash = false,
		.uses_array = false,
	};
	struct parse_deftype_s *dcur, *dtmp;
	struct parse_var_s *vcur, *vtmp;
	struct analyse_tree_s *cur;
	unsigned i;


	/***************
	 * BUILD TREES *
	 ***************/

	ar.deftype_tree.branch_count = 0;
	ar.deftype_tree.is_terminal = false;

	if (pr.deftypes != NULL) {
		HASH_ITER(hh, pr.deftypes, dcur, dtmp) {
			if (!dcur->is_used)
				continue;

			cur = &(ar.deftype_tree);

			/* walk down the tree, creating nodes when necessary */
			for (i = 0; dcur->name[i] != '\0'; i++) {
				if (cur->branch_count == 0
						|| cur->branch_chars[cur->branch_count - 1]
						!= dcur->name[i]
				) {
					cur->branch_count++;
					cur->branch_chars[cur->branch_count - 1] = dcur->name[i];
					TRYALLOC(cur->branches[cur->branch_count - 1], 1);
					cur = cur->branches[cur->branch_count - 1];
					cur->branch_count = 0;
					cur->is_terminal = false;
				} else {
					cur = cur->branches[cur->branch_count - 1];
				}
			}

			cur->is_terminal = true;
			strcpy(cur->name, dcur->name);
		}
	}

	ar.var_tree.branch_count = 0;
	ar.var_tree.is_terminal = false;

	if (pr.vars != NULL) {
		HASH_ITER(hh, pr.vars, vcur, vtmp) {
			cur = &(ar.var_tree);

			/* walk down the tree, creating nodes when necessary */
			for (i = 0; vcur->name[i] != '\0'; i++) {
				if (cur->branch_count == 0
						|| cur->branch_chars[cur->branch_count - 1]
						!= vcur->name[i]
				) {
					cur->branch_count++;
					cur->branch_chars[cur->branch_count - 1] = vcur->name[i];
					TRYALLOC(cur->branches[cur->branch_count - 1], 1);
					cur = cur->branches[cur->branch_count - 1];
					cur->branch_count = 0;
					cur->is_terminal = false;
				} else {
					cur = cur->branches[cur->branch_count - 1];
				}
			}

			cur->is_terminal = true;
			strcpy(cur->name, vcur->name);
		}
	}


	/************************
	 * CALCULATE USED TYPES *
	 ************************/

	for (i = PARSE_TYPE_BOOL; i < PARSE_TYPE_HASH_DEFTYPE; i++)
		ar.uses_type[i] = false;

	HASH_ITER(hh, pr.deftypes, dcur, dtmp) {
		if (!dcur->is_used)
			continue;

		for (i = 0; i < dcur->member_list_len; i++) {
			if (dcur->member_type_list[i] >= PARSE_TYPE_HASH_BOOL) {
				ar.uses_hash = true;
				ar.uses_type[dcur->member_type_list[i]
					- PARSE_TYPE_HASH_BOOL] = true;
			} else if (dcur->member_type_list[i] >= PARSE_TYPE_ARRAY_BOOL) {
				ar.uses_array = true;
				ar.uses_type[dcur->member_type_list[i]
					- PARSE_TYPE_ARRAY_BOOL] = true;
			}

			ar.uses_type[dcur->member_type_list[i]] = true;
		}
	}

	HASH_ITER(hh, pr.vars, vcur, vtmp) {
		if (vcur->type >= PARSE_TYPE_HASH_BOOL) {
			ar.uses_hash = true;
			ar.uses_type[vcur->type - PARSE_TYPE_HASH_BOOL] = true;
		} else if (vcur->type >= PARSE_TYPE_ARRAY_BOOL) {
			ar.uses_array = true;
			ar.uses_type[vcur->type - PARSE_TYPE_ARRAY_BOOL] = true;
		}

		if (vcur->type == PARSE_TYPE_DEFTYPE
				|| vcur->type == PARSE_TYPE_ARRAY_DEFTYPE 
				|| vcur->type == PARSE_TYPE_HASH_DEFTYPE
		) {
			continue;
		}

		ar.uses_type[vcur->type] = true;
	}

	return ar;
}

static void sub_recurse_free(struct analyse_tree_s *t)
{
	unsigned i;

	for (i = 0; i < t->branch_count; i++) {
		sub_recurse_free(t->branches[i]);
		free(t->branches[i]);
	}
}

void analyse_result_wipe(struct analyse_result_s *r)
{
	assert(r != NULL);

	sub_recurse_free(&(r->deftype_tree));
	sub_recurse_free(&(r->var_tree));
}
