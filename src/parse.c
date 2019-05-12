#include "parse.h"
#include "err.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char *curfname;

#define ERR_AT(l, c, ...) \
	do { \
		fprintf(stderr, "\e[1m%s:%zu:%zu:\e[0m ", \
				curfname, (l), (c)); \
		ERR(__VA_ARGS__); \
	} while (0)

#define ERR_END(t) \
	do { \
		if ((t).type == TOK_END) \
			ERR_AT((t).line, (t).col, "unexpected end of file"); \
	} while (0)

#define WARN_AT(l, c, ...) \
	do { \
		fprintf(stderr, "\e[1m%s:%zu:%zu:\e[0m ", \
				curfname, (l), (c)); \
		WARN(__VA_ARGS__); \
	} while (0)

static struct parse_result_s r;

static enum parse_type_e sub_parse_type(void)
{
	struct tok_s t = tok_get();
	enum parse_type_e type = PARSE_TYPE_BOOL;
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

	ERR_END(t);

	if (t.type != TOK_ID) {
		if (type >= PARSE_TYPE_HASH_BOOL)
			ERR_AT(l, c, "invalid type `hash %s`", t.val);
		else if (type >= PARSE_TYPE_ARRAY_BOOL)
			ERR_AT(l, c, "invalid type `array %s`", t.val);
		else
			ERR_AT(l, c, "invalid type `%s`", t.val);
	}

	if (!strcmp(t.val, "bool") || !strcmp(t.val, "boolean"))
		return type;

	if (!strcmp(t.val, "string"))
		return type + PARSE_TYPE_STRING;

	if (!strcmp(t.val, "char")) {
		t = tok_get();

		ERR_END(t);

		if (t.type != TOK_ASTERISK) {
			if (type >= PARSE_TYPE_HASH_BOOL)
				ERR_AT(l, c, "invalid type `hash char %s`", t.val);
			else if (type >= PARSE_TYPE_ARRAY_BOOL)
				ERR_AT(l, c, "invalid type `array char %s`", t.val);
			else
				ERR_AT(l, c, "invalid type `char %s`", t.val);
		}

		return type + PARSE_TYPE_STRING;
	}

	if (!strcmp(t.val, "int") || !strcmp(t.val, "integer"))
		return type + PARSE_TYPE_INT;

	if (!strcmp(t.val, "uint"))
		return type + PARSE_TYPE_UINT;

	if (!strcmp(t.val, "unsigned")) {
		l = t.line;
		c = t.col;
		t = tok_get();

		if (t.type != TOK_ID
				|| (strcmp(t.val, "int") && strcmp(t.val, "integer"))
		) {
			tok_unget(t);
		}

		return type + PARSE_TYPE_UINT;
	}

	tok_unget(t);

	return type + PARSE_TYPE_DEFTYPE;
}

static void sub_parse_deftype(size_t line, size_t col, bool is_union)
{
	struct tok_s t;
	enum parse_type_e type;
	struct parse_deftype_s *dtp;
	struct parse_deftype_s dt = {
		.line = line,
		.col = col,
		.is_union = is_union,
		.member_list_len = 0,
	};
	unsigned i, j;

	t = tok_get();
	ERR_END(t);
	if (t.type != TOK_ID) {
		ERR_AT(t.line, t.col, "unexpected token `%s` (expected %s name)",
				t.val, (dt.is_union ? "union" : "struct"));
	}

	HASH_FIND_STR(r.deftypes, t.val, dtp);
	if (dtp != NULL) {
		ERR_AT(dt.line, dt.col,
				"type `%s` redefined (previous definition was at line %zu)",
				t.val, dtp->line);
	}

	strcpy(dt.name, t.val);

	t = tok_get();
	ERR_END(t);
	if (t.type != TOK_LBRACE)
		ERR_AT(t.line, t.col, "unexpected token `%s` (expected `{`)", t.val);

	while (true) {
		if (dt.member_list_len == PARSE_DEFTYPE_MAX_LEN) {
			ERR_AT(dt.line, dt.col, "%s %s has too many members",
					(dt.is_union ? "union" : "struct"), dt.name);
		}

		t = tok_get();
		if (t.type == TOK_RBRACE) {
			if (dt.member_list_len < 2) {
				ERR_AT(dt.line, dt.col, "%s `%s` must specify at fewest two members",
						(dt.is_union ? "union" : "struct"), dt.name);
			}

			break;
		}

		tok_unget(t);

		type = sub_parse_type();

		if (type == PARSE_TYPE_DEFTYPE
				|| type == PARSE_TYPE_ARRAY_DEFTYPE
				|| type == PARSE_TYPE_HASH_DEFTYPE
		) {
			t = tok_get();
			ERR_AT(t.line, t.col, "defined types may not contain other defined types");
		}

		t = tok_get();
		ERR_END(t);
		if (t.type != TOK_ID) {
			if (t.type == TOK_RBRACE || t.type == TOK_COMMA) {
				ERR_AT(t.line, t.col, "missing member name in %s `%s'",
						(dt.is_union ? "union" : "struct"), dt.name);
			}

			ERR_AT(t.line, t.col, "bad %s member name `%s`",
					(dt.is_union ? "union" : "struct"), t.val);
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
				ERR_AT(dt.line, dt.col, "%s `%s` contains multiple members named %s",
						(dt.is_union ? "union" : "struct"), dt.name,
						dt.member_name_list[i]);
			}

		}
	}

	TRYALLOC(dtp, 1);
	memcpy(dtp, &dt, sizeof(*dtp));

	HASH_ADD_STR(r.deftypes, name, dtp);
}

static bool sub_parse_op(void)
{
	struct tok_s t = tok_get();

	switch (t.type) {
	case TOK_OP_STRUCT:
		sub_parse_deftype(t.line, t.col, false);
		return true;

	case TOK_OP_UNION:
		sub_parse_deftype(t.line, t.col, true);
		return true;

	case TOK_OP_FUN_SUF:
		if (r.fun_suf_seen) {
			WARN_AT(t.line, t.col,
					"function-suffix redefined (previous value was `%s`)",
					r.fun_suf);
		}

		t = tok_get();
		ERR_END(t);
		if (t.type != TOK_ID)
			ERR_AT(t.line, t.col, "invalid function-suffix `%s`", t.val);

		strcpy(r.fun_suf, t.val);

		r.fun_suf_seen = true;

		return true;

	case TOK_OP_HKEY_SIZE:
		if (r.hkey_size_seen) {
			WARN_AT(t.line, t.col,
					"hkey_size redefined (previous value was %lu)",
					r.hkey_size);
		}

		t = tok_get();
		ERR_END(t);
		if (t.type != TOK_UINT)
			ERR_AT(t.line, t.col, "invalid hkey_size `%s`", t.val);

		r.hkey_size = strtoul(t.val, NULL, 10);

		r.hkey_size_seen = true;

		return true;

	case TOK_OP_HKEY_NAME:
		if (r.hkey_name_seen) {
			ERR_AT(t.line, t.col,
					"hkey_name redefined (previous value was `%s`)",
					r.hkey_name);
		}

		t = tok_get();
		ERR_END(t);
		if (t.type != TOK_ID)
			ERR_AT(t.line, t.col, "invalid hkey_name `%s`", t.val);

		strcpy(r.hkey_name, t.val);

		r.hkey_name_seen = true;

		return true;

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
	ERR_END(t);
	if (t.type != TOK_ID) {
		ERR_AT(t.line, t.col,
				"unexpected token `%s`, (expected variable name)", t.val);
	}

	HASH_FIND_STR(r.vars, t.val, vp);
	if (vp != NULL) {
		ERR_AT(v.line, v.col,
				"`%s` redefined (previous definition was at line %zu)",
				t.val, vp->line);
	}

	strcpy(v.name, t.val);

	t = tok_get();
	ERR_END(t);
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
	HASH_ADD_STR(r.vars, name, vp);

	return true;
}

struct parse_result_s parse(FILE *f, const char *fname)
{
	size_t i, j;
	struct tok_s t;

	struct parse_var_s *vcur, *vtmp;
	struct parse_deftype_s *d;

	r.hkey_size = 16;
	strcpy(r.hkey_name, "id");
	r.hkey_name_seen = false;
	r.hkey_size_seen = false;
	r.fun_suf_seen = false;
	r.deftypes = NULL;
	r.vars = NULL;
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

	HASH_ITER(hh, r.vars, vcur, vtmp) {
		switch (vcur->type) {
		case PARSE_TYPE_DEFTYPE:
		case PARSE_TYPE_ARRAY_DEFTYPE:
		case PARSE_TYPE_HASH_DEFTYPE:
			HASH_FIND_STR(r.deftypes, vcur->deftype_name, d);
			if (d == NULL) {
				ERR_AT(vcur->line, vcur->col,
						"rule for variable `%s` references undefined type `%s`",
						vcur->name, vcur->deftype_name);
			}
		default:
			continue;
		}
	}

	if (!r.fun_suf_seen) {

		j = 0;

		for (i = strlen(fname); i > 0 && fname[i] != '/'
				&& fname[i] != '\\'; i--);

		j = i + (fname[i] == '/' || fname[i] == '\\');

		for (i = j; fname[i] != '\0' && fname[i] != '.'
				&& i - j < r.hkey_size; i++) {
			if (!isalnum(fname[i])) {
				fprintf(stderr, "\e[1m%s:\e[0m ", fname);
				ERR("no function suffix specified, and could not generate one");
			}
			r.fun_suf[i - j] = fname[i];
		}

		r.fun_suf[i - j] = '\0';

		if (r.fun_suf[0] == '\0') {
			fprintf(stderr, "\e[1m%s:\e[0m ", fname);
			ERR("no function suffix specified, and could not generate one");
		}

		fprintf(stderr, "\e[1m%s:\e[0m ", fname);
		WARN("no function suffix specified. using `%s`...", r.fun_suf);
	}

	return r;
}

void parse_result_wipe(struct parse_result_s *r)
{
	struct parse_var_s *vcur, *vtmp;
	struct parse_deftype_s *dcur, *dtmp;

	assert(r != NULL);

	if (r->vars != NULL) {
		HASH_ITER(hh, r->vars, vcur, vtmp) {
			HASH_DEL(r->vars, vcur);
			free(vcur);
		}
	}

	if (r->deftypes != NULL) {
		HASH_ITER(hh, r->deftypes, dcur, dtmp) {
			HASH_DEL(r->deftypes, dcur);
			free(dcur);
		}
	}

}
