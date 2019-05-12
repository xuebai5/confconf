#include "err.h"
#include "opt.h"
#include "parse.h"

int main(int argc, char **argv)
{
	FILE *fi = stdin;
	const char *finame = "stdin";
	struct parse_result_s pr;

	opt_parse(argc, argv);

	if (opt_infile_str() != NULL) {
		finame = opt_infile_str();
		fi = fopen(finame, "r");
		TRY(fi != NULL, "could not read file `%s`", finame);
	}

	pr = parse(fi, finame);

	if (fi != stdin)
		fclose(fi);

	parse_result_wipe(&pr);

	return 0;
}
