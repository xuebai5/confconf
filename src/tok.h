#ifndef CONFCONF_TOK_H
#define CONFCONF_TOK_H

#include <stdio.h>

enum tok_type_e {
	      TOK_LBRACE = 0,
	      TOK_RBRACE = 1,
	       TOK_EQUAL = 2,
	       TOK_COMMA = 3,
	        TOK_BANG = 4,
	       TOK_QMARK = 5,
	   TOK_OP_STRUCT = 6,
	TOK_OP_HKEY_SIZE = 7,
	TOK_OP_HKEY_NAME = 8,
	  TOK_OP_FUN_SUF = 9,
	        TOK_UINT = 10,
	          TOK_ID = 11,
	      TOK_UNKNWN = 12,
	         TOK_END = 13,
};

struct tok_s {
	enum tok_type_e type;
	size_t line;
	size_t col;
	char *val;
};

struct tok_s tok_get(FILE *f);

#endif
