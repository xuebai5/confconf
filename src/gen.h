#ifndef CONFCONF_GEN_H
#define CONFCONF_GEN_H

#include "parse.h"
#include "analyse.h"

void gen(FILE *f, struct parse_result_s *pr, struct analyse_result_s *ar);

#endif
