#include "parse.h"

#include "err.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

static const char *curfname;

#define ERR_AT(l, c, ...) \
	do { \
		fprintf(stderr, "\x1B[1m%s:%zu:%zu:\x1B[0m ", \
			curfname, (l), (c)); \
		ERR(__VA_ARGS__); \
	} while (0)

#define ERR_LONG(t) \
	do { \
		if ((t).type == TOK_LONG) { \
			ERR_AT((t).line, (t).col, "token too long"); \
		} \
	} while (0)

#define ERR_END(t, m) \
	do { \
		if ((t).type == TOK_END) { \
			ERR_AT((t).line, (t).col, "unexpected end of file (expected %s)", \
				(m) ); \
		} \
	} while (0)

#define WARN_AT(l, c, ...) \
	do { \
		fprintf(stderr, "\x1B[1m%s:%zu:%zu:\x1B[0m ", \
			curfname, (l), (c)); \
		WARN(__VA_ARGS__); \
	} while (0)

static struct parse_result_s prs;

const char *parse_typestrs[] = {
	[PARSE_TYPE_BOOL]    = "bool",
	[PARSE_TYPE_STRING]  = "string",
	[PARSE_TYPE_ID]      = "id",
	[PARSE_TYPE_INT]     = "int",
	[PARSE_TYPE_INTL]    = "intl",
	[PARSE_TYPE_INTLL]   = "intll",
	[PARSE_TYPE_UINT]    = "uint",
	[PARSE_TYPE_UINTL]   = "uintl",
	[PARSE_TYPE_UINTLL]  = "uintll",
	[PARSE_TYPE_FLOAT]   = "float",
	[PARSE_TYPE_DOUBLE]  = "double",
	[PARSE_TYPE_DOUBLEL] = "doublel"
};

int parse_typestr_to_type(const char *s)
{
	if (*s == 'b' 
		&& *(++s) == 'o' && *(++s) == 'o'
		&& *(++s) == 'l' && *(++s) == '\0'
	) {
		return PARSE_TYPE_BOOL;
	} else if (*s == 's'
		&& *(++s) == 't' && *(++s) == 'r'
		&& *(++s) == 'i' && *(++s) == 'n'
		&& *(++s) == 'g' && *(++s) == '\0'
	) {
		return PARSE_TYPE_STRING;
	} else if (*s == 'i') {
		s++;
		if (*s == 'd' && *(++s) == '\0') {
			return PARSE_TYPE_ID;
		} else if (*s == 'n' && *(++s) == 't') {
			s++;
			if (*s == '\0') {
				return PARSE_TYPE_INT;
			} else if (*s == 'l') {
				s++;
				if (*s == '\0') {
					return PARSE_TYPE_INTL;
				} else if (*s == 'l' && *(++s) == '\0') {
					return PARSE_TYPE_INTLL;
				} else {
					return -1;
				}
			}
		}
	} else if (*s == 'u'
		&& *(++s) == 'i' && *(++s) == 'n'
		&& *(++s) == 't'
	) {
		s++;
		if (*s == '\0') {
			return PARSE_TYPE_UINT;
		} else if (*s == 'l') {
			s++;
			if (*s == '\0') {
				return PARSE_TYPE_UINTL;
			} else if (*s == 'l' && *(++s) == '\0') {
				return PARSE_TYPE_UINTLL;
			} else {
				return -1;
			}
		}
	} else if (*s == 'f'
		&& *(++s) == 'l' && *(++s) == 'o'
		&& *(++s) == 'a' && *(++s) == 't'
		&& *(++s) == '\0'
	) {
		return PARSE_TYPE_FLOAT;
	} else if (*s == 'd'
		&& *(++s) == 'o' && *(++s) == 'u'
		&& *(++s) == 'b' && *(++s) == 'l'
		&& *(++s) == 'e'
	) {
		s++;
		if (*s == '\0') {
			return PARSE_TYPE_DOUBLE;
		} else if (*s == 'l' && *(++s) == '\0') {
			return PARSE_TYPE_DOUBLEL;
		} else {
			return -1;
		}
	}

	return -1;
}

static int sub_parse_type(void)
{
	struct tok_s t = tok_get();
	enum parse_type_e type = PARSE_TYPE_BOOL, tswp;
	size_t l, c;

	l = t.line;
	c = t.col;

	if (!strcmp(t.val, "array") ) {
		type = PARSE_TYPE_ARRAY_BOOL;
		t = tok_get();
	} else if (!strcmp(t.val, "hash") ) {
		type = PARSE_TYPE_HASH_BOOL;
		t = tok_get();
	}

	ERR_END(t, "type");
	ERR_LONG(t);

	if (t.type != TOK_ID) {
		if (type >= PARSE_TYPE_HASH_BOOL)
			ERR_AT(l, c, "invalid type `hash %s`", t.val);
		else if (type >= PARSE_TYPE_ARRAY_BOOL)
			ERR_AT(l, c, "invalid type `array %s`", t.val);
		else
			ERR_AT(l, c, "invalid type `%s`", t.val);
	}

	tswp = parse_typestr_to_type(t.val);

	if (tswp != -1)
		return type + tswp;

	tok_unget(t);

	return type + PARSE_TYPE_DEFTYPE;
}

static void sub_parse_deftype(size_t line, size_t col,
	enum parse_deftype_e dtype)
{
	struct tok_s t;
	enum parse_type_e type;
	struct parse_deftype_s *dtp;
	struct parse_deftype_s dt = {
		.line = line,
		.col = col,
		.is_used = false,
		.type = dtype,
		.is_in_array = false,
		.is_in_hash = false,
		.contains_heap = false,
		.member_list_len = 0,
	};
	unsigned i, j;

	/* name */

	t = tok_get();
	ERR_END(t,
		(dt.type == PARSE_DEFTYPE_UNION ? "union name" :
			(dt.type == PARSE_DEFTYPE_STRUCT ?
			 "struct name" : "enum name"
			)
		)
	);
	ERR_LONG(t);
	if (t.type != TOK_ID) {
		ERR_AT(t.line, t.col, "unexpected token `%s` (expected %s name)",
			t.val,
			(dt.type == PARSE_DEFTYPE_UNION ? "union"
				: (dt.type == PARSE_DEFTYPE_STRUCT ? "struct" : "enum")
			)
		);
	}

	if (parse_typestr_to_type(t.val) != -1) {
		ERR_AT(dt.line, dt.col,
			"defined type conflicts with builtin type `%s`", t.val);
	}

	HASH_FIND_STR(prs.deftypes, t.val, dtp);
	if (dtp != NULL) {
		ERR_AT(dt.line, dt.col,
			"type `%s` redefined (previous definition was at line %zu)",
			t.val, dtp->line);
	}

	strcpy(dt.name, t.val);

	/* { */

	t = tok_get();
	ERR_END(t, "`{`");
	if (t.type != TOK_LBRACE)
		ERR_AT(t.line, t.col, "unexpected token `%s` (expected `{`)", t.val);

	/* body */

	while (true) {
		if (dt.member_list_len == PARSE_DEFTYPE_MAX_LEN) {
			ERR_AT(dt.line, dt.col, "%s %s has too many %s",
				(dt.type == PARSE_DEFTYPE_ENUM ? "enum" :
					(dt.type == PARSE_DEFTYPE_UNION ? "union" : "struct")
				),
				dt.name,
				(dt.type == PARSE_DEFTYPE_ENUM ? "ids" : "members")
			);
		}

		t = tok_get();
		if (t.type == TOK_RBRACE) {
			if (dt.member_list_len < 2) {
				ERR_AT(dt.line, dt.col, "%s `%s` must specify at fewest two %s",
					(dt.type == PARSE_DEFTYPE_ENUM ? "enum" :
						(dt.type == PARSE_DEFTYPE_UNION ? "union" : "struct")
					),
					dt.name,
					(dt.type == PARSE_DEFTYPE_ENUM ? "ids" : "members")
				);
			}

			break;
		}

		if (dt.type != PARSE_DEFTYPE_ENUM) {
			tok_unget(t);

			ERR_END(t, 
				(dt.member_list_len < 2 ? "type" : "type or `}`")
			);

			type = sub_parse_type();

			if (type >= PARSE_TYPE_ARRAY_BOOL) {
				ERR_AT(t.line, t.col,
					"defined types may not contain arrays or hashes");
			}

			if (type == PARSE_TYPE_DEFTYPE) {
				t = tok_get();
				ERR_AT(t.line, t.col,
					"defined types may not contain other defined types");
			}

			if (type == PARSE_TYPE_STRING || type == PARSE_TYPE_ID)
				dt.contains_heap = true;

			t = tok_get();
		}

		ERR_END(t,
			(dt.type == PARSE_DEFTYPE_ENUM
				? (dt.member_list_len > 1 ? "id or `}`" : "id")
				: "member name"
			)
		);
		ERR_LONG(t);

		if (t.type != TOK_ID) {
			if (t.type == TOK_RBRACE || t.type == TOK_COMMA) {
				ERR_AT(t.line, t.col, "missing %s in %s `%s'",
					(dt.type == PARSE_DEFTYPE_ENUM ? "id" : "member name"),
					(dt.type == PARSE_DEFTYPE_ENUM ? "enum" :
						(dt.type == PARSE_DEFTYPE_UNION ? "union" : "struct")
					),
					dt.name
				);
			}

			ERR_AT(t.line, t.col, "bad %s `%s` in %s %s",
				(dt.type == PARSE_DEFTYPE_ENUM ? "id" : "member name"),
				t.val,
				(dt.type == PARSE_DEFTYPE_ENUM ? "enum" :
					(dt.type == PARSE_DEFTYPE_UNION ? "union" : "struct")
				),
				dt.name
			);
		}

		strcpy(dt.member_name_list[dt.member_list_len], t.val);
		dt.member_type_list[dt.member_list_len] = type;
		dt.member_list_len++;

		t = tok_get();
		if (t.type != TOK_COMMA)
			tok_unget(t);
	}

	for (i = 0; i < dt.member_list_len; i++) {
		for (j = i + 1; j < dt.member_list_len; j++) {
			if (!strcmp(dt.member_name_list[i], dt.member_name_list[j]) ) {
				ERR_AT(dt.line, dt.col,
					"%s `%s` contains multiple %s named `%s`",
					(dt.type == PARSE_DEFTYPE_ENUM ? "enum" :
						(dt.type == PARSE_DEFTYPE_UNION ? "union" : "struct")
					),
					dt.name,
					(dt.type == PARSE_DEFTYPE_ENUM ? "ids" : "members"),
					dt.member_name_list[i]
				);
			}

			if (dt.type == PARSE_DEFTYPE_UNION
				&& dt.member_type_list[i] == dt.member_type_list[j]
			) {
				ERR_AT(dt.line, dt.col,
					"union `%s` contains multiple members of type %s",
					dt.name,
					parse_typestrs[dt.member_type_list[i]]
				);
			}
		}
	}

	TRYALLOC(dtp, 1);
	memcpy(dtp, &dt, sizeof(*dtp));

	HASH_ADD_STR(prs.deftypes, name, dtp);
}

static bool sub_parse_op(void)
{
	struct tok_s t = tok_get();

	switch (t.type) {
	case TOK_OP_STRUCT:
		sub_parse_deftype(t.line, t.col, PARSE_DEFTYPE_STRUCT);
		return true;
	case TOK_OP_UNION:
		sub_parse_deftype(t.line, t.col, PARSE_DEFTYPE_UNION);
		return true;
	case TOK_OP_ENUM:
		sub_parse_deftype(t.line, t.col, PARSE_DEFTYPE_ENUM);
		return true;

	case TOK_OP_NAMING_SUFFIX:
		if (prs.suffix_seen) {
			WARN_AT(t.line, t.col,
				"naming suffix redefined (previous value was `%s`)",
				prs.suffix);
		}

		t = tok_get();
		ERR_END(t, "naming suffix");
		ERR_LONG(t);
		if (t.type != TOK_ID)
			ERR_AT(t.line, t.col, "invalid naming suffix `%s`", t.val);

		strcpy(prs.suffix, t.val);

		prs.suffix_seen = true;

		return true;

	case TOK_OP_UTHASH_HEADER:
		if (prs.header_seen) {
			WARN_AT(t.line, t.col,
				"uthash header redefined (previous value was %s)",
				prs.header);
		}

		t = tok_get();
		ERR_END(t, "uthash header");
		ERR_LONG(t);
		if (t.type != TOK_HEADER)
			ERR_AT(t.line, t.col, "invalid uthash header `%s`", t.val);

		strcpy(prs.header, t.val);

		prs.header_seen = true;

		return true;

	case TOK_LONG:
		ERR_LONG(t);

	default:
		tok_unget(t);
		return false;
	}
}

static bool sub_parse_rule(void)
{
	struct parse_var_s *vp;
	struct parse_var_s v;
	struct tok_s t = tok_get();

	if (t.type != TOK_BANG) {
		if (t.type != TOK_QMARK) {
			tok_unget(t);
			return false;
		}
		v.is_required = false;
	} else {
		v.is_required = true;
	}

	v.line = t.line;
	v.col = t.col;

	t = tok_get();
	ERR_END(t, "variable name");
	ERR_LONG(t);
	if (t.type != TOK_ID) {
		ERR_AT(t.line, t.col,
			"unexpected token `%s`, (expected variable name)", t.val);
	}

	HASH_FIND_STR(prs.vars, t.val, vp);
	if (vp != NULL) {
		ERR_AT(v.line, v.col,
			"`%s` redefined (previous definition was at line %zu)",
			t.val, vp->line);
	}

	strcpy(v.name, t.val);

	t = tok_get();
	ERR_END(t, "`=`");
	if (t.type != TOK_EQUAL)
		ERR_AT(t.line, t.col, "unexpected token `%s`, (expected `=`)", t.val);

	v.type = sub_parse_type();

	if (v.type == PARSE_TYPE_DEFTYPE
		|| v.type == PARSE_TYPE_ARRAY_DEFTYPE
		|| v.type == PARSE_TYPE_HASH_DEFTYPE
	) {
		t = tok_get();
		strcpy(v.deftype_name, t.val);
	}

	TRYALLOC(vp, 1);
	memcpy(vp, &v, sizeof(*vp));
	HASH_ADD_STR(prs.vars, name, vp);

	return true;
}

static int sub_sort_deftypes(struct parse_deftype_s *d1,
	struct parse_deftype_s *d2)
{
	return strcmp(d1->name, d2->name);
}

static int sub_sort_vars(struct parse_var_s *v1, struct parse_var_s *v2)
{
	return strcmp(v1->name, v2->name);
}

struct parse_result_s* parse(FILE *f, const char *fname)
{
	size_t i, j;
	struct tok_s t;

	struct parse_var_s *vcur, *vtmp;
	struct parse_deftype_s *dcur, *dtmp;

	struct parse_result_s *pr;

	assert(f != NULL);
	assert(fname != NULL);

	TRYALLOC(pr, 1);

	prs.suffix_seen = false;
	prs.header_seen = false;
	prs.deftypes = NULL;
	prs.vars = NULL;
	curfname = fname;

	tok_reset(f);

	while (sub_parse_op() || sub_parse_rule());

	t = tok_get();
	if (t.type != TOK_END) {
		if (t.type == TOK_UNKNWN)
			ERR_AT(t.line, t.col, "unrecognised token `%s`", t.val);
		else
			ERR_AT(t.line, t.col, "unexpected token `%s`", t.val);
	}

	/* err >0 rules */
	if (prs.vars == NULL) {
		fprintf(stderr, "\x1B[1m%s:\x1B[0m ", fname);
		ERR("config must specify at fewest one variable rule");
	}

	/* err undefined type */
	HASH_ITER(hh, prs.vars, vcur, vtmp) {
		switch (vcur->type) {
		case PARSE_TYPE_DEFTYPE:
		case PARSE_TYPE_ARRAY_DEFTYPE:
		case PARSE_TYPE_HASH_DEFTYPE:
			HASH_FIND_STR(prs.deftypes, vcur->deftype_name, dcur);
			if (dcur == NULL) {
				ERR_AT(vcur->line, vcur->col,
					"rule for variable `%s` references undefined type `%s`",
					vcur->name, vcur->deftype_name);
			}
			dcur->is_used = true;
			if (vcur->type == PARSE_TYPE_ARRAY_DEFTYPE)
				dcur->is_in_array = true;
			else if (vcur->type == PARSE_TYPE_HASH_DEFTYPE)
				dcur->is_in_hash = true;

		default:
			continue;
		}
	}

	/* warn type use */
	HASH_ITER(hh, prs.deftypes, dcur, dtmp) {
		if (!dcur->is_used) {
			WARN_AT(dcur->line, dcur->col,
				"type `%s` defined but not used",
				dcur->name);
		}
	}

	/* warn hash header */
	HASH_ITER(hh, prs.vars, vcur, vtmp) {
		if (vcur->type >= PARSE_TYPE_HASH_BOOL && !prs.header_seen) {
			fprintf(stderr, "\x1B[1m%s:\x1B[0m ", fname);
			WARN("no uthash location header specified. using `<uthash.h>`");
			strcpy(prs.header, "<uthash.h>");
			break;
		}
	}

	/* warn/err suffix */
	if (!prs.suffix_seen) {

		j = 0;

		for (i = strlen(fname); i > 0 && fname[i] != '/'
			&& fname[i] != '\\'; i--);

		j = i + (fname[i] == '/' || fname[i] == '\\');

		for (i = j; fname[i] != '\0' && fname[i] != '.'; i++) {
			if (!isalnum(fname[i]) && fname[i] != '_') {
				fprintf(stderr, "\x1B[1m%s:\x1B[0m ", fname);
				ERR("no function suffix specified, and could not generate one");
			}
			prs.suffix[i - j] = fname[i];
		}

		prs.suffix[i - j] = '\0';

		if (prs.suffix[0] == '\0') {
			fprintf(stderr, "\x1B[1m%s:\x1B[0m ", fname);
			ERR("no function suffix specified, and could not generate one");
		}

		fprintf(stderr, "\x1B[1m%s:\x1B[0m ", fname);
		WARN("no function suffix specified. using `%s`...", prs.suffix);
	}

	HASH_SORT(prs.deftypes, sub_sort_deftypes);
	HASH_SORT(prs.vars, sub_sort_vars);

	memcpy(pr, &prs, sizeof(struct parse_result_s));

	return pr;
}

void parse_result_free(struct parse_result_s *pr)
{
	struct parse_var_s *vcur, *vtmp;
	struct parse_deftype_s *dcur, *dtmp;

	assert(pr != NULL);

	if (pr->vars != NULL) {
		HASH_ITER(hh, pr->vars, vcur, vtmp) {
			HASH_DEL(pr->vars, vcur);
			free(vcur);
		}
	}

	if (pr->deftypes != NULL) {
		HASH_ITER(hh, pr->deftypes, dcur, dtmp) {
			HASH_DEL(pr->deftypes, dcur);
			free(dcur);
		}
	}

	free(pr);
}
