#include "opt.h"
#include "err.h"
#include "tok.h"

#include <stdio.h>
#include <errno.h>




int main(int argc, char **argv)
{
	FILE *fi = stdin;
	struct tok_s t;

	opt_parse(argc, argv);

	if (opt_infile_str() != NULL) {
		fi = fopen(opt_infile_str(), "r");
		TRY(fi != NULL, "could not read file `%s`", opt_infile_str());
	}

	while (1) {
		t = tok_get(fi);

		if (t.type == TOK_UNKNWN || t.type == TOK_END)
			break;

		printf("%s:%zu:%zu: ", (fi == stdin ? "stdin" : opt_infile_str()),
				t.line, t.col);

		if (t.type > TOK_QMARK) {
			printf("%u, `%s`\n", t.type, t.val);
		} else {
			printf("%u\n", t.type);
		}
	};

	if (t.type == TOK_UNKNWN) {
		printf("%s:%zu:%zu: error: unrecognised token `%s`\n",
				(fi == stdin ? "stdin" : opt_infile_str()),
				t.line, t.col, t.val);
	}

	fclose(fi);

	return 0;
}
