/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is
entirely the result of my own work. I also confirm that I have not copied any
parts of this program from another person or any other source or facilitated
someone to copy this program from me. I confirm that I will not publish the
program online or share it with anyone without permission of the module leader.

Student Name: Petr-Konstantin Milev
Student ID: sc21pkm
Email: sc21pkm@leeds.ac.uk
Date Work Commenced: 2023-Feb-09
*************************************************************************/

#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE
#define MAX_LEXEM_SIZE 128
#define TRUE 1
#define FALSE 0

typedef struct {
  int *stack;
  int size;
  int top;
} Stack;

typedef enum comment_types {
  NOT_STARTED,
  NOT_COMMENT,
  ONELINE,
  MULTILINE,
  EOF_ERROR
} Comment_Types;

FILE *input_file;
char *input_filename;
Stack peek_str;
int token_line;
int previous_token_line_num;

/**************************************
 **************************************
 ***************stack******************
 **************************************
 **************************************/

int static stack_init(Stack *stack, int size) {
  stack->stack = (int *)malloc(sizeof(int) * size);
  if (stack->stack == NULL)
    return FALSE;
  stack->size = size;
  stack->top = -1;
  return TRUE;
}

int push(int number, Stack *stack) {
  stack->top += 1;

  if (stack->top >= stack->size) {
    stack->stack =
        (int *)realloc(stack->stack, (sizeof(int) * stack->size * 2));
    if (stack->stack == NULL)
      return FALSE;
  }

  stack->stack[stack->top] = number;
  return TRUE;
}

int pop(int *pop_val, Stack *stack) {

  if (stack->top == -1)
    return FALSE; // fail

  *pop_val = stack->stack[stack->top];
  stack->top -= 1;

  return TRUE;
}

/**************************************
 **************************************
 ***************lexer******************
 **************************************
 **************************************/
int peek_char(FILE *fptr) {
  int peek_char = fgetc(fptr);
  return (peek_char == EOF) ? EOF : ungetc(peek_char, fptr);
}

int is_symbol(int character) {
  return character == '(' || character == ')' || character == '[' ||
         character == ']' || character == '{' || character == '}' ||
         character == ',' || character == ';' || character == '.' ||
         character == '=' || character == '+' || character == '-' ||
         character == '*' || character == '/' || character == '&' ||
         character == '|' || character == '~' || character == '<' ||
         character == '>';
}

int is_valid_identifier(int character, unsigned int position) {
  return (islower(character) || isupper(character) || character == '_') ||
         (position != 0 && isdigit(character));
}

int is_comment(int next_char, Comment_Types *comment_type) {
  if (comment_type == NOT_STARTED) {
    if (next_char == '/') {
      switch (peek_char(input_file)) {
      case ('/'):
        *comment_type = ONELINE;
        return TRUE;
      case ('*'):
        *comment_type = MULTILINE;
        return TRUE;
      default:
        *comment_type = NOT_COMMENT;
        return FALSE; // means is division/ multiplication or user error
      }
    } else {
      // cause first char isn't a backslash
      *comment_type = NOT_COMMENT;
      return FALSE;
    }
  } else {

    if (next_char == EOF) {
      *comment_type = EOF_ERROR;
    }

    switch (*comment_type) {
    case ONELINE:
      if (next_char == '\n') {
        *comment_type = NOT_STARTED;
        return FALSE;
      }

      return TRUE;
    case MULTILINE:
      // previous character is * and current is /
      if (peek_str.stack[peek_str.top] == '*' && next_char == '/') {
        *comment_type = NOT_STARTED;
        return FALSE;
      }

      return TRUE;
    default:
      printf("Error has occured, in is_comment()");
      return FALSE;
    }
  }
  return FALSE;
}
// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of
// the lexer If an error occurs, the function should return 0 if everything goes
// well the function should return 1
int InitLexer(char *file) {
  // open input file
  if ((input_file = fopen(file, "r")) == NULL) {
    printf("Error! opening file");

    // Program exits if the file pointer returns NULL.
    return FALSE;
  }

  strcpy(input_filename, file);

  // init stack for PeekNextToken
  stack_init(&peek_str, MAX_LEXEM_SIZE);
  token_line = 0;
  previous_token_line_num = 0;

  return TRUE;
}

// Get the next token from the source file
Token GetNextToken() {
  Token token;
  int next_char;
  previous_token_line_num = token_line;

  next_char = fgetc(input_file);
  push(next_char, &peek_str);

  /* skip all white space (' ', '\f', '\n', '\t', '\r', '\v') */
  /* AND skip all comments */
  /* REUTNRS 1 ERR token */
  Comment_Types comment_type = NOT_STARTED; // tmp var for this loop
  // check comment first so condition doesn't short circuit in wrong way
  while (is_comment(next_char, &comment_type) || isspace(next_char)) {
    // TODO: verify line number not double counted
    token_line += next_char == '\n';

    if (comment_type == EOF_ERROR) {
      token.tp = ERR;
      strcpy(token.lx, "ERR: EOF in comment");
      token.ec = EofInCom;
      token.ln = token_line;
      strcpy(token.fl, input_filename);
      return token;
    }

    next_char = fgetc(input_file);
    push(next_char, &peek_str);
  }

  /* get EOF */
  /* RETURNS 1 EOFile token */
  if (next_char == EOF) {
    push(next_char, &peek_str);
    token.tp = EOFile;
    strcpy(token.lx, "EOF");
    token.ln = token_line;
    strcpy(token.fl, input_filename);

    return token;
  }

  /* string constants */
  /* RETURNS 2 ERR tokens, 1 STRING token */
  if (next_char == '"') {
    next_char = fgetc(input_file);
    push(next_char, &peek_str);
    // while string not ended get string
    unsigned short pos = 0;
    token.lx[pos] = '\0';
    while (next_char != '"') {
      // guards
      if (next_char == EOF) {

        return token;
      } else if (next_char == '\n') {

        return token;
      }

      // max store length of string is 127 char
      if (pos < 128) {
        token.lx[pos] = next_char;
        token.lx[pos + 1] = '\0';
      }

      // iteration var
      pos++;
      next_char = fgetc(input_file);
      push(next_char, &peek_str);
    }
    // tokenize string
    token.tp = STRING;
    token.ln = token_line;
    strcpy(token.fl, input_filename);
  }

  /* numeric constants */
  /* RETURNS 1 INT token */
  if (isdigit(next_char)) {
    next_char = fgetc(input_file);
    push(next_char, &peek_str);
    // while string not ended get string
    unsigned short pos = 0;
    token.lx[pos] = '\0';
    while (isdigit(next_char)) {
      // max store length of number of 127 digits
      if (pos < 128) {
        token.lx[pos] = next_char;
        token.lx[pos + 1] = '\0';
      }
      // iteration var
      pos++;
      next_char = fgetc(input_file);
      push(next_char, &peek_str);
    }
    // tokenize number
    token.tp = INT;
    token.ln = token_line;
    strcpy(token.fl, input_filename);
  }

  /* symbols */
  /* RETURNS 1 SYMBOL token */
  if (is_symbol(next_char)) {
    token.tp = SYMBOL;
    token.lx[0] = next_char;
    token.lx[1] = '\0';
    token.ln = token_line;
    strcpy(token.fl, input_filename);

    return token;
  }

  // TODO: LOOK INTO isalpha() in types.c
  /* reserved words */
  /* boolean/null constants */
  /* identifiers */

  return token;
}

// peek (look) at the next token in the source file without removing it from the
// stream
Token PeekNextToken() {
  Token token;
  // all chars should be on stack
  token = GetNextToken();
  // place back all characters in the right order
  while (peek_str.top > -1) {
    int prev_char; // tmp character
    pop(&prev_char, &peek_str);
    // return character to source file
    ungetc(prev_char, input_file);
  }
  // reset token line counter to line number of previous/unconsumed token
  token_line = previous_token_line_num;

  return token;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer() {

  // FREE global vars
  fclose(input_file);
  free(peek_str.stack);

  return TRUE;
}

// do not remove the next line
#ifndef TEST
// NOTE: main() NOT tested by autograder
int main(int argc, char **argv) {

  if (argc != 2) {
    printf("Usage: lexer \"filename\"");
    return FALSE;
  }

  FILE *output_file;
  char *output_filename = strcat(argv[1], "_tokens_mine.txt");

  if ((output_file = fopen(output_filename, "w")) == NULL) {
    printf("Error! opening file");

    fclose(output_file);
    // Program exits if the file pointer returns NULL.
    return FALSE;
  }

  // InitLexer(argv[1]);

  fclose(output_file);
  return TRUE;
}
// do not remove the next line
#endif
