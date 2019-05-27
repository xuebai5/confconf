#include "err.h"
#include "opt.h"
#include "parse.h"
#include "analyse.h"
#include "gen.h"

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
	FILE *fo = stdout;
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

	if (opt_outfile_str() != NULL) {
		fo = fopen(opt_outfile_str(), "w");
		TRY(fo != NULL, "could not write to file `%s`", opt_outfile_str());
	}

	pr = parse(fi, finame);

	if (fi != stdin)
		fclose(fi);

	ar = analyse(pr);

	print_tree(&ar.deftype_tree);
	puts("");
	print_tree(&ar.var_tree);
	puts("");

	gen(fo, pr, ar);

	if (fo != stdout)
		fclose(fo);

	parse_result_wipe(&pr);
	analyse_result_wipe(&ar);

	return 0;
}
