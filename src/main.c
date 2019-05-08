#include "opt.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	opt_parse(argc, argv);

	if (opt_infile_str())
		puts(opt_infile_str());

	if (opt_outfile_str())
		puts(opt_outfile_str());
	
	return 0;
}
