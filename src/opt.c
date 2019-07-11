#include "version.h"
#include "opt.h"
#include "tok.h"

#include "../reqs/simple-opt/simple-opt.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

static struct simple_opt options[] = {
	{ SIMPLE_OPT_FLAG, 'h', "help", false,
		"print this help message and exit" },
	{ SIMPLE_OPT_FLAG, 'v', "version", false,
		"print the version of confconf in use and exit" },
	{ SIMPLE_OPT_STRING, 'i', "input", true,
		"specify file to read from (default is stdin)", "<file>" },
	{ SIMPLE_OPT_STRING, 'o', "output", true,
		"specify file to write to (default is stdout)", "<file>" },
	{ SIMPLE_OPT_STRING, 'u', "uthash-header", true,
		"specify location header for uthash in the output (default is <uthash.h>)", "<file>" },
	{ SIMPLE_OPT_STRING, 'n', "name-suffix", true,
		"specify unique suffix to append to generated functions and types", "<file>" },
	{ SIMPLE_OPT_END }
};

void opt_parse(int argc, char **argv)
{
	struct simple_opt_result result;
	size_t i;
	char c, cend;

	result = simple_opt_parse(argc, argv, options);

	/* parse err */
	if (result.result_type != SIMPLE_OPT_RESULT_SUCCESS) {
		simple_opt_print_error(stderr, 80, argv[0], result);
		exit(EXIT_FAILURE);
	}

	/* help */
	if (options[0].was_seen) {
		simple_opt_print_usage(stdout, 80, argv[0],
			"[-i input.confconf] [-o output.h]",
			"confconf is a config file parser generator for C",
			options);
		exit(EXIT_SUCCESS);
	}

	/* version */
	if (options[1].was_seen) {
		puts(VERSION);
		exit(EXIT_SUCCESS);
	}

	/* check header */
	if (options[4].was_seen) {
		cend = options[4].val.v_string[0];

		if (cend != '"' && cend != '<') {
			fprintf(stderr, "%s: err: badly-formatted uthash header `%s`\n",
				argv[0], options[4].val.v_string);
			exit(EXIT_FAILURE);
		}

		if (cend == '<')
			cend = '>';

		for (i = 1, c = options[4].val.v_string[1];
			c && c != cend && c != '\n';
			c = options[4].val.v_string[++i]
		);

		if (c != cend
			|| options[4].val.v_string[i+1] != '\0'
			|| strlen(options[4].val.v_string) > TOK_MAX_LEN
			|| options[4].val.v_string[2] == '\0'
		) {
			fprintf(stderr, "%s: err: badly-formatted uthash header `%s`\n",
				argv[0], options[4].val.v_string);
			exit(EXIT_FAILURE);
		}
	}

	/* check suffix */
	if (options[5].was_seen) {
		if (options[5].val.v_string[0] == '\0') {
			fprintf(stderr, "%s: err: badly-formatted name suffix ``\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}

		for (i = 0; options[5].val.v_string[i] != '\0'; i++) {
			if (!isalnum(options[5].val.v_string[i]) ) {
				fprintf(stderr, "%s: err: badly-formatted name suffix `%s`\n",
					argv[0], options[5].val.v_string);
				exit(EXIT_FAILURE);
			}
		}
	}

}

const char* opt_infile_str(void)
{
	return (options[2].was_seen
		? options[2].val.v_string
		: NULL);
}

const char* opt_outfile_str(void)
{
	return (options[3].was_seen
		? options[3].val.v_string
		: NULL);
}

const char* opt_header_str(void)
{
	return (options[4].was_seen
		? options[4].val.v_string
		: NULL);
}

const char* opt_suffix_str(void)
{
	return (options[5].was_seen
		? options[5].val.v_string
		: NULL);
}
