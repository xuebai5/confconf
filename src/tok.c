#include "tok.h"

#include <stdbool.h>
#include <ctype.h>

#define TOK_MAX_LEN 128

static char val[TOK_MAX_LEN];
static size_t vlen = 0;
static struct tok_s curtok = { .line = 1, .col = 1, .val = val };

static bool sub_eat_spaces(FILE *f)
{
	int c;
	bool seen = false;

	while (true) {
		c = getc(f);

		if (c == '\n') {
			curtok.col = 1;
			curtok.line++;
			continue;
		}

		if (!isspace(c)) {
			ungetc(c, f);
			break;
		}

		curtok.col++;

		seen = true;
	}

	return seen;
}

static bool sub_eat_comment(FILE *f)
{
	int c;

	c = getc(f);

	if (c != '#') {
		ungetc(c, f);
		return false;
	}

	while (true) {
		c = getc(f);

		if (c == '\n') {
			curtok.col = 1;
			curtok.line++;
			return true;
		}

		if (c == EOF) {
			ungetc(c, f);
			return true;
		}
	}
}

static void sub_match_op(FILE *f)
{
	struct {
		bool possible;
		enum tok_type_e type;
		char name[(32 < TOK_MAX_LEN ? 32 : TOK_MAX_LEN)];
	} ops[] = {
		{ true, TOK_OP_STRUCT,    ".struct"          },
		{ true, TOK_OP_HKEY_SIZE, ".hash-key-size"   },
		{ true, TOK_OP_HKEY_NAME, ".hash-key-name"   },
		{ true, TOK_OP_FUN_SUF,   ".function-suffix" },
	};
	unsigned i, j;
	bool again;
	int c;

	val[0] = '.';
	vlen = 1;

	for (i = 1;; i++) {
		again = false;
		c = getc(f);

		if (c == EOF || isspace(c)) {
			ungetc(c, f);
			curtok.type = TOK_UNKNWN;
			val[vlen] = '\0';
			return;
		}

		val[vlen] = c;
		vlen++;

		for (j = 0; j < 4; j++) {
			if (!ops[j].possible)
				continue;

			if (c != ops[j].name[i]) {
				ops[j].possible = false;
				continue;
			}

			if (ops[j].name[i+1] == '\0') {
				curtok.type = ops[j].type;
				val[vlen] = '\0';
				return;
			}

			again = true;
		}

		if (!again) {
			vlen--;
			do {
				val[vlen] = c;
				vlen++;
				c = getc(f);
			} while (c != EOF && !isspace(c) && vlen < TOK_MAX_LEN - 1);
			ungetc(c, f);
			val[vlen] = '\0';
			curtok.type = TOK_UNKNWN;
			return;
		}
	}
}

static void sub_match_uint(FILE *f)
{
	int c;

	curtok.type = TOK_UINT;

	while (true) {
		c = getc(f);

		if (!isdigit(c)) {
			ungetc(c, f);
			val[vlen] = '\0';
			return;
		}

		val[vlen] = c;
		vlen++;
	}
}

static void sub_match_id(FILE *f)
{
	int c;

	curtok.type = TOK_ID;

	while (true) {
		c = getc(f);

		if (!isalnum(c) && c != '_') {
			ungetc(c, f);
			val[vlen] = '\0';
			return;
		}

		val[vlen] = c;
		vlen++;
	}
}

struct tok_s tok_get(FILE *f)
{
	int c;

	curtok.col += vlen;
	vlen = 0;

eat:
	if (sub_eat_spaces(f))
		goto eat;
	if (sub_eat_comment(f))
		goto eat;

	c = getc(f);

	switch (c) {
	case '{':
		curtok.type = TOK_LBRACE;
		vlen = 1;
		return curtok;

	case '}':
		curtok.type = TOK_RBRACE;
		vlen = 1;
		return curtok;

	case '=':
		curtok.type = TOK_EQUAL;
		vlen = 1;
		return curtok;

	case ',':
		curtok.type = TOK_COMMA;
		vlen = 1;
		return curtok;

	case '!':
		curtok.type = TOK_BANG;
		vlen = 1;
		return curtok;

	case '?':
		curtok.type = TOK_QMARK;
		vlen = 1;
		return curtok;

	case EOF:
		curtok.type = TOK_END;
		return curtok;

	case '.':
		sub_match_op(f);
		return curtok;
		
	default:
		if (isdigit(c)) {
			val[0] = c;
			vlen = 1;
			sub_match_uint(f);
			return curtok;
		}

		if (isalpha(c) || c == '_') {
			val[0] = c;
			vlen = 1;
			sub_match_id(f);
			return curtok;
		}

		curtok.type = TOK_UNKNWN;

		do {
			val[vlen] = c;
			vlen++;
			c = getc(f);
		} while (c != EOF && !isspace(c) && vlen < TOK_MAX_LEN - 1);

		ungetc(c, f);
		val[vlen] = '\0';

		return curtok;
	}

	return curtok;
}
