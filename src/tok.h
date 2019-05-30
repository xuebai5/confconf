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
	TOK_OP_ENUM,
	TOK_OP_NAMING_SUFFIX,
	TOK_OP_UTHASH_LOCATION,
	TOK_UINT,
	TOK_ID,
	TOK_HEADER,
	TOK_UNKNWN,
	TOK_END,
	TOK_LONG,
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
