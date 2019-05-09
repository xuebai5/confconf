#include "opt.h"

#include "../reqs/simple-opt/simple-opt.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static struct simple_opt options[] = {
	{ SIMPLE_OPT_FLAG, 'h', "help", false,
		"print this help message and exit" },
	{ SIMPLE_OPT_FLAG, 'v', "version", false,
		"print the version of confconf in use and exit" },
	{ SIMPLE_OPT_STRING, 'i', "input", true,
		"specify file to read from (default is stdin)", "<file>" },
	{ SIMPLE_OPT_STRING, 'o', "output", true,
		"specify file to write to (default is stdout)", "<file>" },
	{ SIMPLE_OPT_END }
};

void opt_parse(int argc, char **argv)
{
	struct simple_opt_result result;
	const char version[] = "confconf develop";

	result = simple_opt_parse(argc, argv, options);

	/* parse err */
	if (result.result_type != SIMPLE_OPT_RESULT_SUCCESS) {
		simple_opt_print_error(stderr, 80, argv[0], result);
		exit(EXIT_FAILURE);
	}

	/* help */
	if (options[0].was_seen) {
		simple_opt_print_usage(stdout, 70, argv[0],
				"[-i input.confconf] [-o output.h]",
				"confconf is a config file parser generator for C",
				options);
		exit(EXIT_SUCCESS);
	}

	/* version */
	if (options[1].was_seen) {
		puts(version);
		exit(EXIT_SUCCESS);
	}
}

const char* opt_infile_str(void)
{
	return (options[2].was_seen
			? options[2].val_string
			: NULL);
}

const char* opt_outfile_str(void)
{
	return (options[3].was_seen
			? options[3].val_string
			: NULL);
}
