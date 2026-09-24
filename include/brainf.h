#ifndef BRAINF_H
#define BRAINF_H

#include <stdio.h>
#include <stdlib.h>
#define BRAINF_TAPE_SIZE 30000

/* EOF leaves cells unchanged, 0: EOF == 0; 1: EOF == 1*/
#define BRAINF_EOF_BEHAVIOR 1

#define BRAINF_TOKEN_PLUS '+'
#define BRAINF_TOKEN_MINUS '-'
#define BRAINF_TOKEN_PREVIOUS '<'
#define BRAINF_TOKEN_NEXT '>'
#define BRAINF_TOKEN_OUTPUT '.'
#define BRAINF_TOKEN_INPUT ','
#define BRAINF_TOKEN_LOOP_START '['
#define BRAINF_TOKEN_LOOP_END ']'

#ifdef BRAINF_EXTENSION_DEBUG
#define BRAINF_TOKEN_DEBUG '#'
#else
#define BRAINF_TOKEN_BREAK -10
#endif

#define READLINE_HIST_SIZE 20

/*
    represents a brainf instruction
*/
typedef struct BrainfInstruction {

  /* The difference between the value of the byte at the current pointer and the
   * value we want */
  int difference;

  /* Type of this instruction */
  char type;

  /* Next instruction in the linked list */
  struct BrainfInstruction *next;

  /* Previous instruction in the linked list */
  struct BrainfInstruction *previous;

  /* The first instruction of a loop if this instruction is a loop, otherwise
   * NULL */
  struct BrainfInstruction *loop;

} BrainfInstruction;

/* Contain the root root and the head of the linked list containing instructions
 * for the program */
typedef struct BrainfState {

  /* The root instruction (first) */
  struct BrainfInstruction *root;

  /* The head instruction (last) */

  struct BrainfInstruction *head;

} BrainfState;

/* The callback that'll be invoked when BRAINF_TOKEN_OUTPUT is found
    @param chr is the value of the current cell
*/
typedef int (*BrainfOutputHandler)(int chr);

/* The callback that'll be invoked when BRAINF_TOKEN_INPUT is found
    @return The character that is read
*/
typedef char (*BrainfInputHandler)(void);

/* This struct is the layer between the brainfuck program and the outside. It
 * allows for control over input, output, and memory */
typedef struct BrainfExecutionContext {

  /* The callback that'll be invoked when BRAINF_TOKEN_INPUT is found */
  BrainfInputHandler input_handler;

  /* The callback that'll be invoked when BRAINF_TOKEN_OUTPUT is found */
  BrainfOutputHandler output_handler;

  /* an array containing the memory cells the program can use */
  unsigned char *tape;

  /* index into the tape. Modified during execution */
  int tape_index;

  /* size of tape in number of cells */
  size_t tape_size;

  /* A flag that if set to true, indicates that execution should stop. */
  int shouldStop;
} BrainfExecutionContext;

/* Creates a new state */
BrainfState *brainf_state();

/* Creates a new context
 * @param size The size of the tape
 */
BrainfExecutionContext *brainf_context(int);

/* Adds an instruction to the instruction list
 * @param state the State
 * @param instruction The instruction to add
 * @return The instruction that was added
 */
BrainfInstruction *brainf_add(struct BrainfState *, struct BrainfInstruction *);

/* Removes an instruction from the instruction list
 * @param state the State
 * @param instruction The instruction to remove
 * @return The instruction that was removed
 */
BrainfInstruction *brainf_remove(struct BrainfState *,
                                 struct BrainfInstruction *);

/* Adds an instruction to the front of the instruction list
 * @param state the State
 * @param instruction The instruction to add
 * @return The instruction that was added
 */
BrainfInstruction *brainf_add_first(struct BrainfState *,
                                    struct BrainfInstruction *);

/* Inserts an instruction to the instruction list before an instruction
 * @param state the State
 * @param before The instruction you want to add another instruction before
 * @param instruction The instruction to add
 * @return The instruction that was added
 */
BrainfInstruction *brainf_insert_before(struct BrainfState *,
                                        struct BrainfInstruction *,
                                        struct BrainfInstruction *);

/* Inserts an instruction to the instruction list after an instruction
 * @param state the State
 * @param after The instruction you want to add another instruction after
 * @param instruction The instruction to add
 * @return The instruction that was added
 */
BrainfInstruction *brainf_insert_after(struct BrainfState *,
                                       struct BrainfInstruction *,
                                       struct BrainfInstruction *);

BrainfInstruction *brainf_parse_stream(FILE *);

BrainfInstruction *brainf_parse_stream_until(FILE *, int);

BrainfInstruction *brainf_parse_string(char *);

BrainfInstruction *brainf_parse_substring(char *, int, int);

BrainfInstruction *brainf_parse_substring_incremental(char *, int *, int);

BrainfInstruction *brainf_parse_character(char);

/////////////////////////////////////////////
void brainf_destroy_instruction(struct BrainfInstruction *);

void brainf_destroy_instructions(struct BrainfInstruction *);

void brainf_destroy_state(struct BrainfState *);

void brainf_destroy_context(struct BrainfExecutionContext *);

void brainf_execute(struct BrainfInstruction *,
                    struct BrainfExecutionContext *);

void brainf_execution_stop(BrainfExecutionContext *);

/////////////////
char brainf_getchar(void);

#endif // BRAINF_H