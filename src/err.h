#ifndef CONFCONF_ERR_H
#define CONFCONF_ERR_H

#include <stdio.h>
#include <stdlib.h>

#define ERR(...) \
	do { \
		fprintf(stderr, "error: " __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		exit(EXIT_FAILURE); \
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

#endif
