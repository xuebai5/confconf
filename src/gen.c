#include "gen.h"

#include "version.h"
#include "parse.h"
#include "opt.h"

#include "../reqs/uthash/include/uthash.h"

#include <time.h>
#include <assert.h>

#include "gen-consts.h"

static inline void sub_head(FILE *f, struct analyse_result_s *ar)
{
	fprintf(f, sheadp1);
	fprintf(f, sheadp2);

	if (ar->uses_type[PARSE_TYPE_BOOL])
		fprintf(f, sbool);
	if (ar->uses_type[PARSE_TYPE_STRING])
		fprintf(f, sstring);
	if (ar->uses_type[PARSE_TYPE_INT])
		fprintf(f, sint);
	if (ar->uses_type[PARSE_TYPE_INTL])
		fprintf(f, sintl);
	if (ar->uses_type[PARSE_TYPE_INTLL])
		fprintf(f, sintll);
	if (ar->uses_type[PARSE_TYPE_UINT])
		fprintf(f, suint);
	if (ar->uses_type[PARSE_TYPE_UINTL])
		fprintf(f, suintl);
	if (ar->uses_type[PARSE_TYPE_UINTLL])
		fprintf(f, suintll);
	if (ar->uses_type[PARSE_TYPE_FLOAT])
		fprintf(f, sfloat);
	if (ar->uses_type[PARSE_TYPE_DOUBLE])
		fprintf(f, sdouble);
	if (ar->uses_type[PARSE_TYPE_DOUBLEL])
		fprintf(f, sdoublel);
}

static inline void sub_deftypes_type(FILE *f, struct parse_result_s *pr,
	struct parse_deftype_s *dcur)
{
	unsigned i;

	/* enum for indicating which union elem is used */
	if (dcur->type == PARSE_DEFTYPE_UNION) {
		fprintf(f,
			"enum confconf_utype_%s_%s {\n",
			dcur->name,
			(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
		);

		for (i = 0; i < dcur->member_list_len; i++) {
			fprintf(f,
				"	CONFCONF_UTYPE_%s_%s_%s,\n",
				dcur->name,
				dcur->member_name_list[i],
				(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
			);
		}

		fprintf(f, "};\n\n");
	}

	fprintf(f,
		"%s confconf_type_%s_%s {\n",
		(dcur->type == PARSE_DEFTYPE_ENUM ? "enum" : "struct"),
		dcur->name,
		(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
	);

	if (dcur->type == PARSE_DEFTYPE_UNION) {
		fprintf(f,
			"	enum confconf_utype_%s_%s type;\n"
			"	union {\n",
			dcur->name,
			(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
		);
	}

	for (i = 0; i < dcur->member_list_len; i++) {
		if (dcur->type == PARSE_DEFTYPE_UNION) {
			fprintf(f, "	");
		}

		if (dcur->type == PARSE_DEFTYPE_ENUM) {
			fprintf(f,
				"	CONFCONF_TYPE_%s_%s_%s,\n",
				dcur->name,
				dcur->member_name_list[i],
				(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
			);
			continue;
		}

		switch (dcur->member_type_list[i]) {
		case PARSE_TYPE_BOOL:
			fprintf(f, "	bool ");
			break;
		case PARSE_TYPE_STRING:
			fprintf(f, "	char *");
			break;
		case PARSE_TYPE_ID:
			fprintf(f, "	char *");
			break;
		case PARSE_TYPE_INT:
			fprintf(f, "	int ");
			break;
		case PARSE_TYPE_INTL:
			fprintf(f, "	long int ");
			break;
		case PARSE_TYPE_INTLL:
			fprintf(f, "	long long int ");
			break;
		case PARSE_TYPE_UINT:
			fprintf(f, "	unsigned ");
			break;
		case PARSE_TYPE_UINTL:
			fprintf(f, "	long unsigned ");
			break;
		case PARSE_TYPE_UINTLL:
			fprintf(f, "	long long unsigned ");
			break;
		case PARSE_TYPE_FLOAT:
			fprintf(f, "	float ");
			break;
		case PARSE_TYPE_DOUBLE:
			fprintf(f, "	double ");
			break;
		case PARSE_TYPE_DOUBLEL:
			fprintf(f, "	long double ");
			break;
		default:
			assert(0);
		}

		fprintf(f, "%s;\n", dcur->member_name_list[i]);
	}
	if (dcur->type == PARSE_DEFTYPE_UNION) {
		fprintf(f, "	} val;\n");
	}

	fprintf(f, "};\n\n");
}

static inline void sub_deftypes_get_struct(FILE *f,
	struct parse_deftype_s *dcur)
{
	unsigned i;

	fprintf(f, "	int c;\n");

	/* add stmp for alloc-swap and set all string elements to NULL */
	if (dcur->contains_heap) {
		fprintf(f, "	char *stmp;\n");
		for (i = 0; i < dcur->member_list_len; i++) {
			if (dcur->member_type_list[i] == PARSE_TYPE_STRING) {
				fprintf(f,
					"\n"
					"	t->%s = NULL;\n",
					dcur->member_name_list[i]
				);
			}
		}
	}

	fprintf(f,
		"\n"
		"	c = (*(st->gcp))(st->fp);\n"
		"\n"
		"	if (c == EOF)\n"
		"		return CONFCONF_ERR_UNEXPECTED_EOF;\n"
		"\n"
		"	if (c != '{') {\n"
		"		(*(st->ugcp))(c, st->fp);\n"
		"		pos = confconf_priv_pos_get(st);\n"
		"		r = confconf_priv_get_tok(st);\n"
		"		if (r != CONFCONF_SUCCESS)\n"
		"			return r;\n"
		"		confconf_priv_pos_set(st, pos);\n"
		"		return CONFCONF_ERR_UNEXPECTED_TOKEN;\n"
		"	}\n"
		"\n"
		"	st->col++;\n"
		"	st->byte++;\n"
		"\n"
		"	confconf_priv_eat_space(st);\n"
	);

	for (i = 0; i < dcur->member_list_len; i++) {
		if (i != 0) {
			fprintf(f,
				"\n"
				"	confconf_priv_eat_space(st);\n"
				"\n"
				"	c = (*(st->gcp))(st->fp);\n"
				"	if (c == ',') {\n"
				"		st->col++;\n"
				"		st->byte++;\n"
				"		confconf_priv_eat_space(st);\n"
				"	} else {\n"
				"		(*(st->ugcp))(c, st->fp);\n"
				"	}\n"
			);
		}

		fprintf(f, "\n");

		if (dcur->member_type_list[i] != PARSE_TYPE_STRING) {
			fprintf(f,
				"	st->vp = &(t->%s);\n",
				dcur->member_name_list[i]
			);
		}

		switch (dcur->member_type_list[i]) {
		case PARSE_TYPE_BOOL:
			fprintf(f, "	r = confconf_priv_get_bool(st);\n");
			break;
		case PARSE_TYPE_STRING:
			fprintf(f, "	r = confconf_priv_get_str(st);\n");
			break;
		case PARSE_TYPE_ID:
			fprintf(f, "	r = confconf_priv_get_id(st);\n");
			break;
		case PARSE_TYPE_INT:
			fprintf(f, "	r = confconf_priv_get_int(st);\n");
			break;
		case PARSE_TYPE_INTL:
			fprintf(f, "	r = confconf_priv_get_intl(st);\n");
			break;
		case PARSE_TYPE_INTLL:
			fprintf(f, "	r = confconf_priv_get_intll(st);\n");
			break;
		case PARSE_TYPE_UINT:
			fprintf(f, "	r = confconf_priv_get_uint(st);\n");
			break;
		case PARSE_TYPE_UINTL:
			fprintf(f, "	r = confconf_priv_get_uintl(st);\n");
			break;
		case PARSE_TYPE_UINTLL:
			fprintf(f, "	r = confconf_priv_get_uintll(st);\n");
			break;
		case PARSE_TYPE_FLOAT:
			fprintf(f, "	r = confconf_priv_get_float(st);\n");
			break;
		case PARSE_TYPE_DOUBLE:
			fprintf(f, "	r = confconf_priv_get_double(st);\n");
			break;
		case PARSE_TYPE_DOUBLEL:
			fprintf(f, "	r = confconf_priv_get_doublel(st);\n");
			break;
		default:
			assert(0);
		}

		fprintf(f,
			"	if (r != CONFCONF_SUCCESS)\n"
			"		%s;\n",
			(dcur->contains_heap ? "goto err" : "return r")
		);

		if (dcur->member_type_list[i] == PARSE_TYPE_STRING) {
			fprintf(f,
				"\n"
				"	stmp = confconf_priv_dup_sbuf(st);\n"
				"	if (stmp == NULL)\n"
				"		goto err;\n"
				"	t->%s = stmp;\n",
				dcur->member_name_list[i]
			);
		}
	}

	fprintf(f,
		"\n"
		"	c = (*(st->gcp))(st->fp);\n"
		"\n"
		"	if (c == EOF)\n"
		"		return CONFCONF_ERR_UNEXPECTED_EOF;\n"
		"\n"
		"	if (c != '}') {\n"
		"		(*(st->ugcp))(c, st->fp);\n"
		"		pos = confconf_priv_pos_get(st);\n"
		"		r = confconf_priv_get_tok(st);\n"
		"		if (r != CONFCONF_SUCCESS)\n"
		"			%s;\n"
		"		confconf_priv_pos_set(st, pos);\n"
		"		return CONFCONF_ERR_UNEXPECTED_TOKEN;\n"
		"		%s;\n"
		"	}\n"
		"\n"
		"	return CONFCONF_SUCCESS;\n",
		(dcur->contains_heap ? "goto err" : "return r"),
		(dcur->contains_heap ? "goto err" : "return r")
	);

	if (dcur->contains_heap) {
		fprintf(f, "\nerr:\n");
		for (i = 0; i < dcur->member_list_len; i++) {
			if (dcur->member_type_list[i] == PARSE_TYPE_STRING
				|| dcur->member_type_list[i] == PARSE_TYPE_ID
			) {
				fprintf(f,
					"	if (t->%s != NULL)\n"
					"		free(t->%s);\n"
					"\n",
					dcur->member_name_list[i],
					dcur->member_name_list[i]
				);
			}
		}
		fprintf(f, "	return r;\n");
	}
}

/* emit a switch statement dfa to char-by-char recognise a string */
static void sub_print_switch(FILE *f, struct analyse_trie_s *cur,
	unsigned depth, const char *dest, const char *stmt)
{
	unsigned i;

	cur->idx = 0;
	do {
		/* skip leaves */
		if (cur->branch_count == 0) {
			for (i = 0; i < depth; i++)
				fprintf(f, "\t");
			fprintf(f, "if (*stmp == '\\0') {\n");
			for (i = 0; i < depth + 1; i++)
				fprintf(f, "\t");
			fprintf(f, "%s = %s;\n", dest, cur->sval);
			for (i = 0; i < depth + 1; i++)
				fprintf(f, "\t");
			fprintf(f, "%s;\n", stmt);
			for (i = 0; i < depth; i++)
				fprintf(f, "\t");
			fprintf(f, "}\n");
			for (i = 0; i < depth; i++)
				fprintf(f, "\t");
			fprintf(f, "break;\n");
			depth--;
		} else {
			if (cur->idx == 0) {
				for (i = 0; i < depth; i++)
					fprintf(f, "\t");
				fprintf(f, "switch ( *(stmp++) ) {\n");
			}

			if (cur->idx == cur->branch_count) {
				for (i = 0; i < depth; i++)
					fprintf(f, "\t");
				fprintf(f, "}\n");
				/* skip break for final '}' */
				if (cur->parent != NULL || cur->idx != cur->branch_count) {
					for (i = 0; i < depth; i++)
						fprintf(f, "\t");
					fprintf(f, "break;\n");
				}
				depth--;
			} else {
				/* match prefix terminals */
				if (cur->idx == 0 && cur->sval != NULL) {
					for (i = 0; i < depth; i++)
						fprintf(f, "\t");
					fprintf(f, "case '\\0':\n");
					for (i = 0; i < depth + 1; i++)
						fprintf(f, "\t");
					fprintf(f, "%s = %s;\n", dest, cur->sval);
					for (i = 0; i < depth + 1; i++)
						fprintf(f, "\t");
					fprintf(f, "%s;\n", stmt);
					for (i = 0; i < depth + 1; i++)
						fprintf(f, "\t");
					fprintf(f, "break;\n");
				}
				for (i = 0; i < depth; i++)
					fprintf(f, "\t");
				fprintf(f, "case '%c':\n", cur->branch_chars[cur->idx]);
				depth++;
			}
		}

		/* next node in the trie, iterating depth-first */
		if (cur->idx == cur->branch_count) {
			cur = cur->parent;
		} else {
			cur->idx++;
			cur = cur->branches[cur->idx - 1];
			cur->idx = 0;
		}
	} while (cur != NULL);

}

static inline void sub_deftypes_get_union(FILE *f, struct parse_result_s *pr,
	struct analyse_result_s *ar, struct parse_deftype_s *dcur, unsigned idx)
{
	struct analyse_trie_s *t = &(ar->deftype_mem_tries[idx]);
	unsigned i;

	fprintf(f,
		"	char *stmp;\n"
		"	unsigned mem;\n"
		"\n"
		"	confconf_priv_eat_space(st);\n"
		"\n"
		"	pos = confconf_priv_pos_get(st);\n"
		"	r = confconf_priv_get_tok(st);\n"
		"	if (r != CONFCONF_SUCCESS)\n"
		"		return r;\n"
		"	stmp = st->sbuf;\n"
		"\n"
	);

	sub_print_switch(f, t, 1, "mem", "goto match");

	fprintf(f,
		"\n"
		"	confconf_priv_pos_set(st, pos);\n"
		"	return CONFCONF_ERR_UNEXPECTED_TOKEN;\n"
		"\n"
		"match:\n"
		"	pos = confconf_priv_pos_get(st);\n"
		"\n"
		"	switch (mem) {\n"
	);

	for (i = 0; i < dcur->member_list_len; i++) {
		fprintf(f,
			"	case %u:\n"
			"		st->vp = &(t->val.%s);\n"
			"		r = confconf_priv_get_%s(st);\n"
			"		t->type = CONFCONF_UTYPE_%s_%s_%s;\n"
			"		break;\n",
			i,
			dcur->member_name_list[i],
			parse_typestrs[dcur->member_type_list[i]],
			dcur->name,
			dcur->member_name_list[i],
			(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
		);
	}

	fprintf(f,
		"	}\n"
		"\n"
		"	if (r != CONFCONF_SUCCESS)\n"
		"		return r;\n"
		"\n"
		"	return CONFCONF_SUCCESS;\n"
	);
}

static inline void sub_deftypes_get_enum(FILE *f, struct analyse_result_s *ar,
	struct parse_deftype_s *dcur, unsigned idx)
{
	struct analyse_trie_s *t = &(ar->deftype_mem_tries[idx]);

	fprintf(f,
		"	char *stmp;\n"
		"\n"
		"	confconf_priv_eat_space(st);\n"
		"\n"
		"	pos = confconf_priv_pos_get(st);\n"
		"	r = confconf_priv_get_tok(st);\n"
		"	if (r != CONFCONF_SUCCESS)\n"
		"		return r;\n"
		"	stmp = st->sbuf;\n"
		"\n"
	);

	sub_print_switch(f, t, 1, "*t", "return CONFCONF_SUCCESS");

	fprintf(f,
		"\n"
		"	confconf_priv_pos_set(st, pos);\n"
		"	return CONFCONF_ERR_UNEXPECTED_TOKEN;\n"
	);
}

static inline void sub_deftypes_get(FILE *f, struct parse_result_s *pr,
	struct analyse_result_s *ar, struct parse_deftype_s *dcur, unsigned idx)
{
	fprintf(f,
		"static enum confconf_result_type\n"
		"confconf_priv_get_%s_%s(struct confconf_priv_state *st)\n"
		"{\n"
		"	enum confconf_result_type r;\n"
		"	%s confconf_type_%s_%s *t = st->vp;\n"
		"	struct confconf_priv_pos pos;\n",
		dcur->name,
		(opt_suffix_str() ? opt_suffix_str() : pr->suffix),
		(dcur->type == PARSE_DEFTYPE_ENUM ? "enum" : "struct"),
		dcur->name,
		(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
	);

	switch (dcur->type) {
	case PARSE_DEFTYPE_STRUCT:
		sub_deftypes_get_struct(f, dcur);
		break;

	case PARSE_DEFTYPE_UNION:
		sub_deftypes_get_union(f, pr, ar, dcur, idx);
		break;

	case PARSE_DEFTYPE_ENUM:
		sub_deftypes_get_enum(f, ar, dcur, idx);
		break;

	default:
		assert(0);
	}

	fprintf(f, "}\n\n");
}

static inline void sub_deftypes(FILE *f, struct parse_result_s *pr,
	struct analyse_result_s *ar)
{
	struct parse_deftype_s *dcur, *dtmp;
	unsigned i = 0;

	HASH_ITER(hh, pr->deftypes, dcur, dtmp) {

		if (!dcur->is_used) {
			i++;
			continue;
		}

		/* print the type definition */
		sub_deftypes_type(f, pr, dcur);
		/* print it's get function */
		sub_deftypes_get(f, pr, ar, dcur, i);
		i++;
	}

}

static inline void sub_arraytype(FILE *f, struct parse_result_s *pr,
	struct analyse_result_s *ar)
{
	struct parse_deftype_s *dcur, *dtmp;

	fprintf(f,
		"struct confconf_array_%s {\n"
		"	size_t len;\n"
		"	union {\n"
		"%s%s%s%s%s%s%s%s%s%s%s",
		(opt_suffix_str() ? opt_suffix_str() : pr->suffix),
		(ar->uses_type[PARSE_TYPE_ARRAY_BOOL]
			? "		bool b;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_STRING]
			? "		char *s;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_INT]
			? "		int i;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_INTL]
			? "		long int il;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_INTLL]
			? "		long long int ill;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_UINT]
			? "		unsigned u;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_UINTL]
			? "		long unsigned ul;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_UINTLL]
			? "		long long unsigned ull;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_FLOAT]
			? "		float f;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_DOUBLE]
			? "		double d;\n" : ""),
		(ar->uses_type[PARSE_TYPE_ARRAY_DOUBLEL]
			? "		long double dl;\n" : "")
	);

	HASH_ITER(hh, pr->deftypes, dcur, dtmp) {
		if (dcur->is_used && dcur->is_in_array) {
			fprintf(f,
				"		%s confconf_type_%s_%s type_%s;\n",
				(dcur->type == PARSE_DEFTYPE_ENUM ? "enum" :
					(dcur->type == PARSE_DEFTYPE_UNION ? "union" : "struct")
				),
				dcur->name, pr->suffix, dcur->name
			);
		}
	}

	fprintf(f,
		"	} *arr;\n"
		"};\n\n"
	);
}

static inline void sub_hashtype(FILE *f, struct parse_result_s *pr,
	struct analyse_result_s *ar)
{
	struct parse_deftype_s *dcur, *dtmp;

	fprintf(f,
		"#include %s\n\n"
		"struct confconf_hash_%s {\n"
		"	char *key;\n"
		"	union {\n"
		"%s%s%s%s%s%s%s%s%s%s%s",
		(opt_header_str() ? opt_header_str() : pr->header),
		(opt_suffix_str() ? opt_suffix_str() : pr->suffix),
		(ar->uses_type[PARSE_TYPE_HASH_BOOL]
			? "		bool b;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_STRING]
			? "		char *s;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_INT]
			? "		int i;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_INTL]
			? "		long int il;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_INTLL]
			? "		long long int ill;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_UINT]
			? "		unsigned u;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_UINTL]
			? "		long unsigned ul;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_UINTLL]
			? "		long long unsigned ull;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_FLOAT]
			? "		float f;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_DOUBLE]
			? "		double d;\n" : ""),
		(ar->uses_type[PARSE_TYPE_HASH_DOUBLEL]
			? "		long double dl;\n" : "")
	);

	HASH_ITER(hh, pr->deftypes, dcur, dtmp) {
		if (dcur->is_used && dcur->is_in_hash) {
			fprintf(f,
				"		%s confconf_type_%s_%s type_%s;\n",
				(dcur->type == PARSE_DEFTYPE_ENUM ? "enum" :
					(dcur->type == PARSE_DEFTYPE_UNION ? "union" : "struct")
				),
				dcur->name,
				(opt_suffix_str() ? opt_suffix_str() : pr->suffix),
				dcur->name
			);
		}
	}

	fprintf(f,
		"	} val;\n"
		"	UT_hash_handle hh;\n"
		"};\n\n"
	);
}

static inline void sub_result_struct(FILE *f, struct parse_result_s *pr,
	struct analyse_result_s *ar)
{
	struct parse_var_s *vcur, *vtmp;
	struct parse_deftype_s *dtp;

	fprintf(f,
		"struct confconf_result_%s {\n"
		"	enum confconf_result_type result_type;\n"
		"	size_t end_col;\n"
		"	size_t end_line;\n"
		"	size_t end_byte;\n"
		"	char *end_tok;\n", 
		(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
	);

	HASH_ITER(hh, pr->vars, vcur, vtmp) {
		switch (vcur->type) {
		case PARSE_TYPE_BOOL:
			fprintf(f, "	bool ");
			break;
		case PARSE_TYPE_STRING:
			fprintf(f, "	char *");
			break;
		case PARSE_TYPE_ID:
			fprintf(f, "	char *");
			break;
		case PARSE_TYPE_INT:
			fprintf(f, "	int ");
			break;
		case PARSE_TYPE_INTL:
			fprintf(f, "	long int ");
			break;
		case PARSE_TYPE_INTLL:
			fprintf(f, "	long long int ");
			break;
		case PARSE_TYPE_UINT:
			fprintf(f, "	unsigned ");
			break;
		case PARSE_TYPE_UINTL:
			fprintf(f, "	long unsigned ");
			break;
		case PARSE_TYPE_UINTLL:
			fprintf(f, "	long long unsigned ");
			break;
		case PARSE_TYPE_FLOAT:
			fprintf(f, "	float ");
			break;
		case PARSE_TYPE_DOUBLE:
			fprintf(f, "	double ");
			break;
		case PARSE_TYPE_DOUBLEL:
			fprintf(f, "	long double ");
			break;

		case PARSE_TYPE_DEFTYPE:
			HASH_FIND_STR(pr->deftypes, vcur->deftype_name, dtp);
			assert(dtp != NULL);
			fprintf(f,
				"	%s confconf_type_%s_%s ",
				(dtp->type == PARSE_DEFTYPE_ENUM ? "enum" : "struct"),
				dtp->name,
				(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
			);
			break;

		default:
			if (vcur->type >= PARSE_TYPE_HASH_BOOL) {
				assert(vcur->type <= PARSE_TYPE_HASH_DEFTYPE);
				fprintf(f,
					"	struct confconf_hash_%s ",
					(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
				);
			} else if (vcur->type >= PARSE_TYPE_ARRAY_BOOL) {
				fprintf(f,
					"	struct confconf_array_%s ",
					(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
				);
			} else {
				assert(0);
			}
			break;
		}

		fprintf(f, "val_%s;\n", vcur->name);
	}

	fprintf(f, "};\n\n");
}

static inline void sub_body(FILE *f, struct parse_result_s *pr,
	struct analyse_result_s *ar)
{
	struct parse_var_s *vcur, *vtmp;

}

void gen(FILE *f, struct parse_result_s *pr, struct analyse_result_s *ar)
{
	time_t t;
	struct tm *ti;

	assert(pr != NULL);
	assert(ar != NULL);

	time(&t);
	ti = localtime(&t);

	fprintf(f, "/* generated by %s on %04d-%02d-%02d */\n\n",
		VERSION, ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday);

	/* conditionally include static subroutines from gen-consts.h */
	sub_head(f, ar);

	fprintf(f,
		"#ifndef CONFCONF_BODY_%s_H\n"
		"#define CONFCONF_BODY_%s_H\n\n",
		(opt_suffix_str() ? opt_suffix_str() : pr->suffix),
		(opt_suffix_str() ? opt_suffix_str() : pr->suffix)
	);

	/* define struct/enum/unions and their get functions */
	sub_deftypes(f, pr, ar);

	if (ar->uses_array)
		sub_arraytype(f, pr, ar);

	if (ar->uses_hash)
		sub_hashtype(f, pr, ar);

	sub_result_struct(f, pr, ar);

	sub_body(f, pr, ar);

	fprintf(f, "#endif\n");
}
