#include "brainf.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

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
  fprintf(stderr, "usage: %s [-evh] [file...]\n", name);
  fprintf(stderr, "\t-e --eval\t\trun code directly\n");
  fprintf(stderr, "\t-v --version\t\tshow version information\n");
  fprintf(stderr, "\t-h --help\t\tshow a help message\n");
}

void print_version() {
  fprintf(stderr, "Brainf version %s (%s, %s)\n", VERSION, __DATE__, __TIME__);
  fprintf(stderr, "Distributed under the MIT License.\n");
}

#ifdef BRAINF_EDITLINE_LIB

void initialize_readline() {
  rl_bind_key('\t', rl_insert); // disable tab autocompletion
  stifle_history(READLINE_HIST_SIZE);
}

#endif

int run_file(FILE *file) {
  BrainfState *state = brainf_state();
  BrainfExecutionContext *context = brainf_context(BRAINF_TAPE_SIZE);

  if (file == NULL) {
    brainf_destroy_context(context);
    brainf_destroy_state(state);
    return EXIT_FAILURE;
  }

  brainf_add(state, brainf_parse_stream(file));
  brainf_execute(state->root, context);
  brainf_destroy_context(context);
  brainf_destroy_state(state);
  fclose(file);

  return EXIT_SUCCESS;
}

int run_string(char *code) {
  BrainfState *state = brainf_state();
  BrainfExecutionContext *context = brainf_context(BRAINF_TAPE_SIZE);
  BrainfInstruction *instruction = brainf_parse_string(code);

  brainf_add(state, instruction);
  brainf_execute(state->root, context);
  brainf_destroy_context(context);
  brainf_destroy_state(state);

  return EXIT_SUCCESS;
}

void run_interactive_console() {
  fprintf(stderr, "Brainf version %s (%s, %s)\n", VERSION, __DATE__, __TIME__);

  BrainfState *state = brainf_state();
  BrainfExecutionContext *context = brainf_context(BRAINF_TAPE_SIZE);
  BrainfInstruction *instruction;

#ifdef BRAINFUCK_EDITLINE_LIB

  char *line;
  initialize_readline();
  while (1) {
    line = readline(">> ");
    if (line) {
      if (line[0] == '\0') {
        free(line);
        continue;
      }

      char *expansion;
      int result;

      result = history_expand(line, &expansion);
      if (result >= 0 && result != 2) {
        add_history)(expansion);
      }
      free(expansion);
    } else {
      /* EOF */
      break;
    }
    instruction = brainf_parse_string(line);
    free(line);
    brainf_add(state, instruction);
    brainf_execute(instruction, context);
  }
#else
  printf(">> ");
  while (1) {
    fflush(stdout);
    instruction = brainf_parse_stream_until(stdin, '\n');
    if (feof(stdin))
      break;
    fflush(stdin);
    brainf_add(state, instruction);
    brainf_execute(instruction, context);
    printf(">> ");
  }
#endif
}

static struct option long_options[] = {{"help", no_argument, 0, 'h'},
                                       {"eval", required_argument, 0, 'e'},
                                       {"version", no_argument, 0, 'v'},
                                       {0, 0, 0, 0}};

int main(int argc, char *argv[]) {
  int c, i = 1, option_index = 0;

  while (1) {
    option_index = 0;
    c = getopt_long(argc, argv, "vhe:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
      break;

    case 'h':
      print_usage(argv[0]);
      return EXIT_SUCCESS;

    case 'v':
      print_version();
      return EXIT_SUCCESS;

    case 'e':
      return run_string((char *)optarg);

    case '?':
      print_usage(argv[0]);
      return EXIT_FAILURE;

    default:
      abort();
    }
  }

  if (argc > 1) {
    while (i < argc) {
      if (run_file(fopen(argv[i++], "r")) == EXIT_FAILURE) {
        fprintf(stderr, "Error: failed to read file: %s\n", argv[i - 1]);
      }
    }
  } else {
    if (isatty(STDIN_FILENO)) {
      run_interactive_console();
    } else {
      if (run_file(stdin) == EXIT_FAILURE) {
        fprintf(stderr, "Error: failed to read file: %s\n", argv[i - 1]);
      }
    }
  }
  return EXIT_SUCCESS;
}