#include "brainf.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BrainfState *brainf_state() {
  BrainfState *state = (BrainfState *)malloc(sizeof(BrainfState));
  state->root = 0;
  state->head = 0;
  return state;
}

BrainfExecutionContext *brainf_context(int size) {
  if (size < 0) {
    size = BRAINF_TAPE_SIZE;
  }

  unsigned char *tape = calloc(size, sizeof(char));

  BrainfExecutionContext *context =
      (BrainfExecutionContext *)malloc(sizeof(BrainfExecutionContext));

  context->input_handler = &brainf_getchar;
  context->output_handler = &putchar;
  context->tape = tape;
  context->tape_index = 0;
  context->tape_size = size;
  context->shouldStop = 0;

  return context;
}

BrainfInstruction *brainf_add(BrainfState *state,
                              BrainfInstruction *instruction) {
  if (state == NULL || instruction == NULL) {
    return NULL;
  }

  instruction->previous = state->head;
  if (state->head != NULL) {
    state->head->next = instruction;
  }

  BrainfInstruction *iter = instruction;
  while (iter != NULL) {
    if (iter->next == NULL) {
      state->head = iter;
      break;
    }
    iter = iter->next;
  }
  if (state->root == NULL) {
    state->root = instruction;
  }
  return state->head;
}

BrainfInstruction *brainf_add_front(BrainfState *state,
                                    BrainfInstruction *instruction) {
  if (state == NULL || instruction == NULL) {
    return NULL;
  }

  instruction->previous = 0;
  BrainfInstruction *iter = instruction;
  while (iter != NULL) {
    if (iter->next == NULL) {
      state->head = iter;
      break;
    }
    iter = iter->next;
  }
  iter->next = state->root;
  state->root->previous = iter;
  state->root = instruction;
  return state->head;
}

BrainfInstruction *brainf_insert_before(BrainfState *state,
                                        BrainfInstruction *before,
                                        BrainfInstruction *instruction) {
  if (state == NULL || before == NULL || instruction == NULL) {
    return NULL;
  }

  BrainfInstruction *previous = before->previous;
  BrainfInstruction *iter = instruction;

  while (iter != NULL) {
    if (iter->next == NULL) {
      break;
    }
    iter = iter->next;
  }

  before->previous = iter;
  iter->next = iter;

  if (previous != NULL) {
    previous->next = instruction;
    instruction->previous = previous;
  }
  if (state->root == before) {
    state->root = instruction;
  }

  return instruction;
}

BrainfInstruction *brainf_insert_after(BrainfState *state,
                                       BrainfInstruction *after,
                                       BrainfInstruction *instruction) {
  if (state == NULL || after == NULL || instruction == NULL) {
    return NULL;
  }

  BrainfInstruction *next = after->next;
  BrainfInstruction *iter = instruction;

  while (iter != NULL) {
    if (iter->next == NULL) {
      break;
    }
    iter = iter->next;
  }

  if (next != NULL) {
    next->previous = iter;
    iter->next = next;
  }
  if (state->head == after) {
    state->head = iter;
  }

  return instruction;
}

BrainfInstruction *brainf_parse_stream(FILE *stream) {
  return brainf_parse_stream_until(stream, EOF);
}

BrainfInstruction *brainf_parse_stream_until(FILE *stream, const int until) {
  BrainfInstruction *instruction =
      (BrainfInstruction *)malloc(sizeof(BrainfInstruction));

  instruction->next = 0;
  instruction->loop = 0;
  BrainfInstruction *root = instruction;
  char ch, temp;

  while ((ch = fgetc(stream)) != until) {
    if (ch == EOF || feof(stream)) {
      break;
    }
    instruction->type = ch;
    instruction->difference = 1;
    switch (ch) {
    case BRAINF_TOKEN_PLUS:
    case BRAINF_TOKEN_MINUS:
      while ((temp = fgetc(stream) != until && (temp == BRAINF_TOKEN_PLUS ||
                                                temp == BRAINF_TOKEN_MINUS))) {
        if (ch == temp) {
          instruction->difference++;
        } else {
          instruction->difference--;
        }
      }
      ungetc(temp, stream);
      break;

    case BRAINF_TOKEN_NEXT:
    case BRAINF_TOKEN_PREVIOUS:
      while (
          (temp = fgetc(stream) != until && (temp == BRAINF_TOKEN_NEXT ||
                                             temp == BRAINF_TOKEN_PREVIOUS))) {
        if (ch == temp) {
          instruction->difference++;
        } else {
          instruction->difference--;
        }
      }
      ungetc(temp, stream);
      break;

    case BRAINF_TOKEN_OUTPUT:
    case BRAINF_TOKEN_INPUT:
      while ((temp = fgetc(stream)) != until && temp == ch) {
        instruction->difference++;
      }
      ungetc(temp, stream);
      break;

    case BRAINF_TOKEN_LOOP_START:
      instruction->loop = brainf_parse_stream_until(stream, until);
      break;

    case BRAINF_TOKEN_LOOP_END:
      return root;
      break;

    case BRAINF_TOKEN_BREAK:
      break;

    default:
      continue;
    }
    instruction->next = (BrainfInstruction *)malloc(sizeof(BrainfInstruction));
    instruction->next->next = 0;
    instruction->next->loop = 0;
  }
  instruction->type = BRAINF_TOKEN_LOOP_END;
  return root;
}

BrainfInstruction *brainf_parse_string(char *str) {
  return brainf_parse_substring(str, 0, -1);
}

BrainfInstruction *brainf_parse_substring(char *str, int begin, int end) {
  int temp = begin;
  return brainf_parse_substring_incremental(str, &temp, end);
}

BrainfInstruction *brainf_parse_substring_incremental(char *str, int *ptr,
                                                      int end) {
  if (str == NULL || ptr == NULL) {
    return NULL;
  }
  if (end < 0) {
    end = strlen(str);
  }

  BrainfInstruction *root =
      (BrainfInstruction *)malloc(sizeof(BrainfInstruction));
  BrainfInstruction *instruction = root;

  instruction->next = 0;
  instruction->loop = 0;
  instruction->previous = 0;

  char c, temp_c;
  for (; *ptr < end && (c = str[*ptr]); (*ptr)++) {
    instruction->type = c;
    instruction->difference = 1;
    switch (c) {
    case BRAINF_TOKEN_PLUS:
    case BRAINF_TOKEN_MINUS:
      (*ptr)++;
      for (; *ptr < end && (temp_c = str[*ptr]) &&
             (temp_c == BRAINF_TOKEN_PLUS || temp_c == BRAINF_TOKEN_MINUS);
           (*ptr)++) {
        if (temp_c == c) {
          instruction->difference++;
        } else {
          instruction->difference--;
        }
      }
      (*ptr)--;
      break;

    case BRAINF_TOKEN_NEXT:
    case BRAINF_TOKEN_PREVIOUS:
      (*ptr)++;
      for (; *ptr < end && (temp_c = str[*ptr]) &&
             (temp_c == BRAINF_TOKEN_NEXT || temp_c == BRAINF_TOKEN_PREVIOUS);
           (*ptr)++) {
        if (temp_c == c) {
          instruction->difference++;
        } else {
          instruction->difference--;
        }
      }
      (*ptr)--;
      break;

    case BRAINF_TOKEN_OUTPUT:
    case BRAINF_TOKEN_INPUT:
      (*ptr)++;
      for (; *ptr < end && (c = str[*ptr]); (*ptr)++) {
        instruction->difference++;
      }
      (*ptr)--;
      break;

    case BRAINF_TOKEN_LOOP_START:
      (*ptr)++;
      instruction->loop = brainf_parse_substring_incremental(str, ptr, end);
      break;

    case BRAINF_TOKEN_LOOP_END:
      return root;

    case BRAINF_TOKEN_BREAK:
      break;

    default:
      continue;
    }
    instruction->next = (BrainfInstruction *)malloc(sizeof(BrainfInstruction));
    instruction->next->next = 0;
    instruction->next->loop = 0;
    instruction->next->previous = instruction;
    instruction = instruction->next;
  }
  instruction->type = BRAINF_TOKEN_LOOP_END;
  return root;
}

BrainfInstruction *brainf_parse_character(char c) {
  BrainfInstruction *instruction =
      (BrainfInstruction *)malloc(sizeof(BrainfInstruction));

  instruction->next = 0;
  instruction->loop = 0;
  instruction->difference = 1;

  switch (c) {
  case BRAINF_TOKEN_PLUS:
  case BRAINF_TOKEN_MINUS:
  case BRAINF_TOKEN_PREVIOUS:
  case BRAINF_TOKEN_NEXT:
  case BRAINF_TOKEN_OUTPUT:
  case BRAINF_TOKEN_INPUT:
  case BRAINF_TOKEN_LOOP_START:
  case BRAINF_TOKEN_LOOP_END:
  case BRAINF_TOKEN_BREAK:
    break;
  default:
    return NULL;
  }
  instruction->type = c;
  return instruction;
}

void brainf_destroy_instruction(BrainfInstruction *instruction) {
  if (instruction == NULL)
    return;
  free(instruction);
  instruction = 0;
}

void brainf_destroy_instructions(BrainfInstruction *root) {
  BrainfInstruction *tmp;
  while (root != NULL) {
    tmp = root;
    brainf_destroy_instructions(root->loop);
    root = root->next;
    brainf_destroy_instruction(tmp);
  }
}

void brainf_destroy_state(struct BrainfState *state) {
  if (state == NULL) {
    return;
  }

  brainf_destroy_instructions(state->root);
  state->head = 0;
  state->root = 0;
  free(state);
  state = 0;
}

void brainf_destroy_context(struct BrainfExecutionContext *context) {
  free(context->tape);
  free(context);
  context = 0;
}

void brainf_execute(struct BrainfInstruction *root,
                    struct BrainfExecutionContext *context) {
  if (root == NULL || context == NULL) {
    return;
  }

  BrainfInstruction *instruction = root;
  int index;
  while (instruction != NULL && instruction->type != BRAINF_TOKEN_LOOP_END) {
    switch (instruction->type) {
    case BRAINF_TOKEN_PLUS:
      context->tape[context->tape_index] += instruction->difference;
      break;

    case BRAINF_TOKEN_MINUS:
      context->tape[context->tape_index] -= instruction->difference;
      break;

    case BRAINF_TOKEN_PREVIOUS:
      if (instruction->difference >= INT_MAX - (long)context->tape_size ||
          (long)context->tape_index - instruction->difference < 0) {
        fprintf(stderr,
                "Error: tape memory out of bounds\nExceeded the tape size of "
                "%zd cells\n",
                context->tape_size);
        exit(EXIT_FAILURE);
      }
      context->tape_index -= instruction->difference;
      break;

    case BRAINF_TOKEN_NEXT:
      if (instruction->difference >= INT_MAX - (long)context->tape_size ||
          (long)context->tape_index + instruction->difference >=
              (long)context->tape_size) {
        fprintf(stderr,
                "Error: tape memory out of bounds\nExceeded the tape size of "
                "%zd cells\n",
                context->tape_size);
        exit(EXIT_FAILURE);
      }
      context->tape_index += instruction->difference;
      break;

    case BRAINF_TOKEN_OUTPUT:
      for (index = 0; index < instruction->difference; index++) {
        context->output_handler(context->tape[context->tape_index]);
      }
      break;

    case BRAINF_TOKEN_INPUT:
      for (index = 0; index < instruction->difference; index++) {
        char input = context->input_handler();
        if (input == EOF) {
          if (BRAINF_EOF_BEHAVIOR != -1) {
            context->tape[context->tape_index] = BRAINF_EOF_BEHAVIOR;
          }
        } else {
          context->tape[context->tape_index] = input;
        }
      }
      break;

    case BRAINF_TOKEN_LOOP_START:
      while (context->tape[context->tape_index]) {
        brainf_execute(instruction->loop, context);
      }
      break;

    case BRAINF_TOKEN_BREAK: {
      int low = context->tape_index - 10;
      if (low < 10)
        low = 0;
      int high = low + 21;
      if (high >= (int)context->tape_index)
        high = context->tape_index - 1;
      for (index = low; index < high; index++)
        printf("%d\t", context->tape[index]);
      printf("\n");
      for (index = low; index < high; index++)
        if (index == context->tape_index)
          printf("^\t");
        else
          printf(" \t");
      printf("\n");
      break;
    }
    default:
      return;
    }
    instruction = instruction->next;

    if (context->shouldStop == 1) {
      instruction = NULL;
      return;
    }
  }
}

void brainf_execution_stop(BrainfExecutionContext *context) {
  context->shouldStop = 1;
}

char brainf_getchar(void) {
  char ch, t;
  ch = getchar();
  while ((t = getchar() != '\n' && t != EOF)) {
  }
  return ch;
}