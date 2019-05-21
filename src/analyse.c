#include "err.h"
#include "analyse.h"

#include <assert.h>
#include <string.h>

struct analyse_result_s analyse(struct parse_result_s pr)
{
	struct analyse_result_s ar;
	struct parse_deftype_s *dtp, *tmp_dtp;
	struct parse_var_s *vtp, *tmp_vtp;
	struct analyse_tree_s *cur;
	unsigned i;

	ar.deftype_tree.branch_count = 0;
	ar.deftype_tree.is_terminal = false;

	if (pr.deftypes != NULL) {
		HASH_ITER(hh, pr.deftypes, dtp, tmp_dtp) {
			cur = &(ar.deftype_tree);

			/* walk down the tree, creating nodes when necessary */
			for (i = 0; dtp->name[i] != '\0'; i++) {
				if (cur->branch_count == 0
						|| cur->branch_chars[cur->branch_count - 1]
						!= dtp->name[i]
				) {
					cur->branch_count++;
					cur->branch_chars[cur->branch_count - 1] = dtp->name[i];
					TRYALLOC(cur->branches[cur->branch_count - 1], 1);
					cur = cur->branches[cur->branch_count - 1];
					cur->branch_count = 0;
					cur->is_terminal = false;
				} else {
					cur = cur->branches[cur->branch_count - 1];
				}
			}

			cur->is_terminal = true;
			strcpy(cur->name, dtp->name);
		}
	}

	ar.var_tree.branch_count = 0;
	ar.var_tree.is_terminal = false;

	if (pr.vars != NULL) {
		HASH_ITER(hh, pr.vars, vtp, tmp_vtp) {
			cur = &(ar.var_tree);

			/* walk down the tree, creating nodes when necessary */
			for (i = 0; vtp->name[i] != '\0'; i++) {
				if (cur->branch_count == 0
						|| cur->branch_chars[cur->branch_count - 1]
						!= vtp->name[i]
				) {
					cur->branch_count++;
					cur->branch_chars[cur->branch_count - 1] = vtp->name[i];
					TRYALLOC(cur->branches[cur->branch_count - 1], 1);
					cur = cur->branches[cur->branch_count - 1];
					cur->branch_count = 0;
					cur->is_terminal = false;
				} else {
					cur = cur->branches[cur->branch_count - 1];
				}
			}

			cur->is_terminal = true;
			strcpy(cur->name, vtp->name);
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
