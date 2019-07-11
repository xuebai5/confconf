#ifndef CONFCONF_PARSE_H
#define CONFCONF_PARSE_H

#include "tok.h"

#include "../reqs/uthash/include/uthash.h"

#include <stdbool.h>

#define PARSE_DEFTYPE_MAX_LEN 128

/* very important these stay in order.
 * things like ">= PARSE_TYPE_ARRAY_BOOL" used */
enum parse_type_e {
	PARSE_TYPE_BOOL          = 0,
	PARSE_TYPE_STRING        = 1,
	PARSE_TYPE_ID            = 2,
	PARSE_TYPE_INT           = 3,
	PARSE_TYPE_INTL          = 4,
	PARSE_TYPE_INTLL         = 5,
	PARSE_TYPE_UINT          = 6,
	PARSE_TYPE_UINTL         = 7,
	PARSE_TYPE_UINTLL        = 8,
	PARSE_TYPE_FLOAT         = 9,
	PARSE_TYPE_DOUBLE        = 10,
	PARSE_TYPE_DOUBLEL       = 11,
	PARSE_TYPE_DEFTYPE       = 12,

	PARSE_TYPE_ARRAY_BOOL    = 13,
	PARSE_TYPE_ARRAY_STRING  = 14,
	PARSE_TYPE_ARRAY_ID      = 15,
	PARSE_TYPE_ARRAY_INT     = 16,
	PARSE_TYPE_ARRAY_INTL    = 17,
	PARSE_TYPE_ARRAY_INTLL   = 18,
	PARSE_TYPE_ARRAY_UINT    = 19,
	PARSE_TYPE_ARRAY_UINTL   = 20,
	PARSE_TYPE_ARRAY_UINTLL  = 21,
	PARSE_TYPE_ARRAY_FLOAT   = 22,
	PARSE_TYPE_ARRAY_DOUBLE  = 23,
	PARSE_TYPE_ARRAY_DOUBLEL = 24,
	PARSE_TYPE_ARRAY_DEFTYPE = 25,

	PARSE_TYPE_HASH_BOOL     = 26,
	PARSE_TYPE_HASH_STRING   = 27,
	PARSE_TYPE_HASH_ID       = 28,
	PARSE_TYPE_HASH_INT      = 29,
	PARSE_TYPE_HASH_INTL     = 30,
	PARSE_TYPE_HASH_INTLL    = 31,
	PARSE_TYPE_HASH_UINT     = 32,
	PARSE_TYPE_HASH_UINTL    = 33,
	PARSE_TYPE_HASH_UINTLL   = 34,
	PARSE_TYPE_HASH_FLOAT    = 35,
	PARSE_TYPE_HASH_DOUBLE   = 36,
	PARSE_TYPE_HASH_DOUBLEL  = 37,
	PARSE_TYPE_HASH_DEFTYPE  = 38,
};

enum parse_deftype_e {
	PARSE_DEFTYPE_STRUCT,
	PARSE_DEFTYPE_UNION,
	PARSE_DEFTYPE_ENUM,
};

struct parse_deftype_s {
	char name[TOK_MAX_LEN];
	enum parse_deftype_e type;
	size_t line;
	size_t col;
	bool is_used;
	bool is_in_array;
	bool is_in_hash;
	bool contains_heap;
	unsigned member_list_len;
	enum parse_type_e member_type_list[PARSE_DEFTYPE_MAX_LEN];
	char member_name_list[PARSE_DEFTYPE_MAX_LEN][TOK_MAX_LEN];
	UT_hash_handle hh;
};

struct parse_var_s {
	char name[TOK_MAX_LEN];
	size_t line;
	size_t col;
	bool is_required;
	enum parse_type_e type;
	char deftype_name[TOK_MAX_LEN];
	UT_hash_handle hh;
};

struct parse_result_s {
	bool suffix_seen;
	char suffix[TOK_MAX_LEN];
	bool header_seen;
	char header[TOK_MAX_LEN];
	struct parse_deftype_s *deftypes;
	struct parse_var_s *vars;
};

extern const char *parse_typestrs[];

int parse_typestr_to_type(const char *s);

struct parse_result_s* parse(FILE *f, const char *fname);

void parse_result_free(struct parse_result_s *pr);

#endif
