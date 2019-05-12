#ifndef CONFCONF_PARSE_H
#define CONFCONF_PARSE_H

#include "tok.h"

#include <uthash.h>

#include <stdbool.h>

#define PARSE_DEFTYPE_MAX_LEN 32

enum parse_type_e {
	PARSE_TYPE_BOOL          = 0,
	PARSE_TYPE_STRING        = 1,
	PARSE_TYPE_INT           = 2,
	PARSE_TYPE_UINT          = 3,
	PARSE_TYPE_DEFTYPE       = 4,

	PARSE_TYPE_ARRAY_BOOL    = 5,
	PARSE_TYPE_ARRAY_STRING  = 6,
	PARSE_TYPE_ARRAY_INT     = 7,
	PARSE_TYPE_ARRAY_UINT    = 8,
	PARSE_TYPE_ARRAY_DEFTYPE = 9,

	PARSE_TYPE_HASH_BOOL     = 10,
	PARSE_TYPE_HASH_STRING   = 11,
	PARSE_TYPE_HASH_INT      = 12,
	PARSE_TYPE_HASH_UINT     = 13,
	PARSE_TYPE_HASH_DEFTYPE  = 14,
};

struct parse_deftype_s {
	char name[TOK_MAX_LEN];
	size_t line;
	size_t col;
	bool is_union;
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
	unsigned long hkey_size;
	bool hkey_size_seen;
	char hkey_name[TOK_MAX_LEN];
	bool hkey_name_seen;
	char fun_suf[TOK_MAX_LEN];
	bool fun_suf_seen;
	struct parse_deftype_s *deftypes;
	struct parse_var_s *vars;
};

struct parse_result_s parse(FILE *f, const char *fname);

void parse_result_wipe(struct parse_result_s *r);

#endif
