#include "err.h"
#include "analyse.h"

#include <assert.h>
#include <string.h>

struct analyse_result_s analyse(struct parse_result_s pr)
{
	struct analyse_result_s ar = {
		.uses_array = false,
		.uses_hash = false,
		.uses_bool = false,
		.uses_string = false,
		.uses_id = false,
		.uses_int = false,
		.uses_intl = false,
		.uses_intll = false,
		.uses_uint = false,
		.uses_uintl = false,
		.uses_uintll = false,
		.uses_float = false,
		.uses_double = false,
		.uses_doublel = false,
	};
	struct parse_deftype_s *dcur, *dtmp;
	struct parse_var_s *vcur, *vtmp;
	struct analyse_tree_s *cur;
	unsigned i;

	ar.deftype_tree.branch_count = 0;
	ar.deftype_tree.is_terminal = false;

	if (pr.deftypes != NULL) {
		HASH_ITER(hh, pr.deftypes, dcur, dtmp) {
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

	for (i = 0; i < PARSE_TYPE_ARRAY_BOOL; i++)
		ar.hash_types[i] = false;

	HASH_ITER(hh, pr.vars, vcur, vtmp) {
		if (vcur->type >= PARSE_TYPE_HASH_BOOL) {
			ar.uses_hash = true;
			ar.uses_id = true;
			ar.hash_types[vcur->type - PARSE_TYPE_HASH_BOOL] = true;
		} else if (vcur->type >= PARSE_TYPE_ARRAY_BOOL) {
			ar.uses_array = true;
		}

		switch (vcur->type
			- (PARSE_TYPE_ARRAY_BOOL * (vcur->type >= PARSE_TYPE_ARRAY_BOOL))
			- (PARSE_TYPE_ARRAY_BOOL * (vcur->type >= PARSE_TYPE_HASH_BOOL))
		) {
		case PARSE_TYPE_BOOL:
			ar.uses_bool = true;
			break;
		case PARSE_TYPE_STRING:
			ar.uses_string = true;
			break;
		case PARSE_TYPE_ID:
			ar.uses_id = true;
			break;
		case PARSE_TYPE_INT:
			ar.uses_int = true;
			break;
		case PARSE_TYPE_INTL:
			ar.uses_intl = true;
			break;
		case PARSE_TYPE_INTLL:
			ar.uses_intll = true;
			break;
		case PARSE_TYPE_UINT:
			ar.uses_uint = true;
			break;
		case PARSE_TYPE_UINTL:
			ar.uses_uintl = true;
			break;
		case PARSE_TYPE_UINTLL:
			ar.uses_uintll = true;
			break;
		case PARSE_TYPE_FLOAT:
			ar.uses_float = true;
			break;
		case PARSE_TYPE_DOUBLE:
			ar.uses_double = true;
			break;
		case PARSE_TYPE_DOUBLEL:
			ar.uses_doublel = true;
			break;
		default:
			HASH_FIND_STR(pr.deftypes, vcur->deftype_name, dcur);
			assert(dcur != NULL);
			for (i = 0; i < dcur->member_list_len; i++) {
				switch (dcur->member_type_list[i]
					- (PARSE_TYPE_ARRAY_BOOL *
						(dcur->member_type_list[i] >= PARSE_TYPE_ARRAY_BOOL)
					)
					- (PARSE_TYPE_ARRAY_BOOL *
						(dcur->member_type_list[i] >= PARSE_TYPE_HASH_BOOL)
					)
				) {
				case PARSE_TYPE_BOOL:
					ar.uses_bool = true;
					break;
				case PARSE_TYPE_STRING:
					ar.uses_string = true;
					break;
				case PARSE_TYPE_ID:
					ar.uses_id = true;
					break;
				case PARSE_TYPE_INT:
					ar.uses_int = true;
					break;
				case PARSE_TYPE_INTL:
					ar.uses_intl = true;
					break;
				case PARSE_TYPE_INTLL:
					ar.uses_intll = true;
					break;
				case PARSE_TYPE_UINT:
					ar.uses_uint = true;
					break;
				case PARSE_TYPE_UINTL:
					ar.uses_uintl = true;
					break;
				case PARSE_TYPE_UINTLL:
					ar.uses_uintll = true;
					break;
				case PARSE_TYPE_FLOAT:
					ar.uses_float = true;
					break;
				case PARSE_TYPE_DOUBLE:
					ar.uses_double = true;
					break;
				case PARSE_TYPE_DOUBLEL:
					ar.uses_doublel = true;
					break;
				default:
					assert(false);
				}
			}
		}
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
