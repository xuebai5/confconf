#ifndef CONFCONF_TOK_H
#define CONFCONF_TOK_H

#include <stdio.h>

#define TOK_MAX_LEN 128

enum tok_type_e {
	TOK_LBRACE,
	TOK_RBRACE,
	TOK_EQUAL,
	TOK_COMMA,
	TOK_BANG,
	TOK_QMARK,
	TOK_ASTERISK,
	TOK_OP_STRUCT,
	TOK_OP_UNION,
	TOK_OP_HKEY_SIZE,
	TOK_OP_HKEY_NAME,
	TOK_OP_FUN_SUF,
	TOK_UINT,
	TOK_ID,
	TOK_UNKNWN,
	TOK_END,
};

struct tok_s {
	enum tok_type_e type;
	size_t line;
	size_t col;
	char *val;
};

void tok_reset(FILE *f);

struct tok_s tok_get(void);
void tok_unget(struct tok_s t);

#endif
