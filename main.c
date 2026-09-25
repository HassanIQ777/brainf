#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef BRAINF_EDITLINE_LIB
#include <editline/readline.h>
#endif

#if defined(_WIN32) || defined(WIN32) ||                                       \
    defined(__WIN32) && !defined(__CYGWIN__)
#include <io.h>
#define isatty _isatty
#define STDIN_FILENO 0
#else
#include <unistd.h>
#endif

#include "src/brainf.c"

void print_usage(char *name) {
  fprintf(stderr, "Brainf version %s (%s, %s)\n", VERSION, __DATE__, __TIME__);
}

int main() {}