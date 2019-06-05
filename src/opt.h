#ifndef CONFCONF_OPT_H
#define CONFCONF_OPT_H

void opt_parse(int argc, char **argv);

const char* opt_infile_str(void);
const char* opt_outfile_str(void);
const char* opt_header_str(void);
const char* opt_suffix_str(void);

#endif
