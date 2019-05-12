#ifndef CONFCONF_ERR_H
#define CONFCONF_ERR_H

#include <stdio.h>
#include <stdlib.h>

#define ERR(...) \
	do { \
		fprintf(stderr, "\e[1;31merror:\e[0m " __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(EXIT_FAILURE); \
	} while (0)

#define WARN(...) \
	do { \
		fprintf(stderr, "\e[1;35mwarning:\e[0m " __VA_ARGS__); \
		fprintf(stderr, "\n"); \
	} while (0)

#define TRY(cond, ...) \
	do { \
		if (!(cond)) { \
			ERR(__VA_ARGS__); \
		} \
	} while (0)

#define TRYALLOC(dest, count) \
	do { \
		(dest) = malloc((count) * sizeof(*(dest))); \
		TRY((dest) != NULL, "could not allocate memory"); \
	} while (0)

#define TRYREALLOC(dest, count) \
	do { \
		(dest) = realloc((dest), (count) * sizeof(*(dest))); \
		TRY((dest) != NULL, "could not allocate memory"); \
	} while (0)

#endif
