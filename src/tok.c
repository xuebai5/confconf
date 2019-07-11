#include "tok.h"

#include <assert.h>
#include <stdbool.h>
#include <ctype.h>

static char val[TOK_MAX_LEN];
static size_t vlen;
static struct tok_s curtok = { .val = val };
static FILE *curf;
static bool unget;

static bool sub_eat_spaces(void)
{
	int c;
	bool seen = false;

	while (true) {
		c = getc(curf);

		if (c == '\n') {
			curtok.col = 1;
			curtok.line++;
			continue;
		}

		if (!isspace(c)) {
			ungetc(c, curf);
			break;
		}

		curtok.col++;

		seen = true;
	}

	return seen;
}

static bool sub_eat_comment(void)
{
	int c;

	c = getc(curf);

	if (c != '#') {
		ungetc(c, curf);
		return false;
	}

	while (true) {
		c = getc(curf);

		if (c == '\n') {
			curtok.col = 1;
			curtok.line++;
			return true;
		}

		if (c == EOF) {
			ungetc(c, curf);
			return true;
		}
	}
}

static void sub_match_op(void)
{
	struct {
		bool possible;
		enum tok_type_e type;
		char name[32];
	} ops[] = {
		{ true, TOK_OP_STRUCT, ".struct" },
		{ true, TOK_OP_UNION,  ".union"  },
		{ true, TOK_OP_ENUM,   ".enum"   },
		{ true, TOK_OP_NAMING_SUFFIX,   ".name-suffix"   },
		{ true, TOK_OP_UTHASH_HEADER, ".uthash-header" },
	};
	unsigned i, j;
	bool again;
	int c;

	val[0] = '.';
	vlen = 1;

	for (i = 1;; i++) {
		again = false;
		c = getc(curf);

		if (c == EOF || isspace(c)) {
			ungetc(c, curf);
			curtok.type = TOK_UNKNWN;
			val[vlen] = '\0';
			return;
		}

		val[vlen] = c;
		vlen++;

		for (j = 0; j < 5; j++) {
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
				c = getc(curf);
			} while (c != EOF && !isspace(c) && vlen < TOK_MAX_LEN - 1);

			if (vlen + 2 == TOK_MAX_LEN) {
				vlen = 0;
				val[0] = '\0';
				curtok.type = TOK_LONG;
				return;
			}

			ungetc(c, curf);
			val[vlen] = '\0';
			curtok.type = TOK_UNKNWN;
			return;
		}
	}
}

static void sub_match_uint(void)
{
	int c;

	curtok.type = TOK_UINT;

	while (true) {

		c = getc(curf);

		if (!isdigit(c)) {
			ungetc(c, curf);
			val[vlen] = '\0';
			return;
		}

		if (vlen + 2 == TOK_MAX_LEN) {
			vlen = 0;
			val[0] = '\0';
			curtok.type = TOK_LONG;
			return;
		}

		val[vlen] = c;
		vlen++;
	}
}

static void sub_match_id(void)
{
	int c;

	curtok.type = TOK_ID;

	while (true) {
		c = getc(curf);

		if (!isalnum(c) && c != '_') {
			ungetc(c, curf);
			val[vlen] = '\0';
			return;
		}

		if (vlen + 2 == TOK_MAX_LEN) {
			vlen = 0;
			val[0] = '\0';
			curtok.type = TOK_LONG;
			return;
		}

		val[vlen] = c;
		vlen++;
	}
}

static void sub_match_header(void)
{
	int c, cend;

	curtok.type = TOK_END;
	
	cend = (val[0] == '"' ? '"' : '>');

	while (true) {
		c = getc(curf);

		if (c == EOF)
			return;

		if (c == '\n') {
			val[vlen] = '\0';
			curtok.type = TOK_UNKNWN;
			return;
		}

		if (vlen + 2 == TOK_MAX_LEN) {
			vlen = 0;
			val[0] = '\0';
			curtok.type = TOK_LONG;
			return;
		}

		val[vlen] = c;
		vlen++;

		if (c == cend)
			break;
	}

	curtok.type = TOK_HEADER;

	val[vlen] = '\0';
	vlen++;
}

void tok_reset(FILE *f)
{
	assert(f != NULL);

	curf = f;
	curtok.line = 1;
	curtok.col = 1;
	vlen = 0;
	unget = false;
}

struct tok_s tok_get(void)
{
	int c;

	assert(TOK_MAX_LEN >= 32);

	if (unget) {
		unget = false;
		return curtok;
	}
	curtok.col += vlen;
	vlen = 0;

	while (sub_eat_spaces() || sub_eat_comment());

	c = getc(curf);

	switch (c) {
	case '{':
		curtok.type = TOK_LBRACE;
		vlen = 1;
		val[0] = '{';
		val[1] = '\0';
		return curtok;

	case '}':
		curtok.type = TOK_RBRACE;
		vlen = 1;
		val[0] = '}';
		val[1] = '\0';
		return curtok;

	case '=':
		curtok.type = TOK_EQUAL;
		vlen = 1;
		val[0] = '=';
		val[1] = '\0';
		return curtok;

	case ',':
		curtok.type = TOK_COMMA;
		vlen = 1;
		val[0] = ',';
		val[1] = '\0';
		return curtok;

	case '!':
		curtok.type = TOK_BANG;
		vlen = 1;
		val[0] = '!';
		val[1] = '\0';
		return curtok;

	case '?':
		curtok.type = TOK_QMARK;
		vlen = 1;
		val[0] = '?';
		val[1] = '\0';
		return curtok;

	case '*':
		curtok.type = TOK_ASTERISK;
		vlen = 1;
		val[0] = '*';
		val[1] = '\0';
		return curtok;

	case EOF:
		ungetc(c, curf);
		curtok.type = TOK_END;
		vlen = 0;
		val[0] = '\0';
		return curtok;

	case '.':
		sub_match_op();
		return curtok;
		
	default:
		if (c == '"' || c == '<') {
			val[0] = c;
			vlen = 1;
			sub_match_header();
			return curtok;
		}

		if (isdigit(c)) {
			val[0] = c;
			vlen = 1;
			sub_match_uint();
			return curtok;
		}

		if (isalpha(c) || c == '_') {
			val[0] = c;
			vlen = 1;
			sub_match_id();
			return curtok;
		}

		curtok.type = TOK_UNKNWN;

		do {
			val[vlen] = c;
			vlen++;
			c = getc(curf);
		} while (c != EOF && !isspace(c) && vlen < TOK_MAX_LEN - 1);

		ungetc(c, curf);
		val[vlen] = '\0';

		return curtok;
	}

	return curtok;
}

void tok_unget(struct tok_s t)
{
	unget = true;
	curtok = t;
}
