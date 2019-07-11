#include "err.h"
#include "opt.h"
#include "parse.h"
#include "analyse.h"
#include "gen.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	FILE *fo = stdout;
	FILE *fi = stdin;
	const char *finame = "stdin";
	struct parse_result_s *pr;
	struct analyse_result_s *ar;

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

	gen(fo, pr, ar);

	if (fo != stdout)
		fclose(fo);

	parse_result_free(pr);
	analyse_result_free(ar);

	return 0;
}
