#include "err.h"
#include "opt.h"
#include "parse.h"
#include "analyse.h"

static void print_tree(struct analyse_tree_s *t)
{
	unsigned i;
	if (t->is_terminal)
		printf("!");

	if (t->branch_count > 1)
		printf("(");

	for (i = 0; i < t->branch_count; i++) {
		printf("%c", t->branch_chars[i]);
		print_tree(t->branches[i]);
		if (t->branch_count > 1 && i < t->branch_count - 1)
			printf("|");
	}

	if (t->branch_count > 1)
		printf(")");
}

int main(int argc, char **argv)
{
	FILE *fi = stdin;
	const char *finame = "stdin";
	struct parse_result_s pr;
	struct analyse_result_s ar;

	opt_parse(argc, argv);

	if (opt_infile_str() != NULL) {
		finame = opt_infile_str();
		fi = fopen(finame, "r");
		TRY(fi != NULL, "could not read file `%s`", finame);
	}

	pr = parse(fi, finame);

	if (fi != stdin)
		fclose(fi);

	ar = analyse(pr);

	print_tree(&ar.deftype_tree);
	puts("");
	print_tree(&ar.var_tree);
	puts("");

	parse_result_wipe(&pr);
	analyse_result_wipe(&ar);

	return 0;
}
