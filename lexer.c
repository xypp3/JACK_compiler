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
int newline_flag;

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

int is_white_space(int next_char, int *newline_flag) {
  newline_flag += next_char == '\n';

  return next_char == ' ' || next_char == '\n' || next_char == '\t' ||
         next_char == '\r';
}

int is_lowercase(int character) {
  return 97 <= character && character <= 122; // a <= character <= z
}

int is_uppercase(int character) {
  return is_lowercase(character + 32); // uppercase char + 32 == lowercase char
}

int is_digit(int character) {
  return 48 <= character && character <= 57; // 0 <= character <= 9
}

int is_valid_identifier(int character, unsigned int position) {
  return (is_lowercase(character) || is_uppercase(character) ||
          character == '_') ||
         (position != 0 && is_digit(character));
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

    // TODO:
    if (next_char == EOF) {
      *comment_type = EOF_ERROR;
    }

    switch (*comment_type) {
    case ONELINE:
      if (next_char == '\n') {
        return FALSE;
      }

      return TRUE;
    case MULTILINE:
      if (next_char == '*' && peek_char(input_file) == '/') {
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
  newline_flag = 0;

  return TRUE;
}

// Get the next token from the source file
Token GetNextToken() {
  Token token;
  int next_char;

  next_char = fgetc(input_file);
  push(next_char, &peek_str);

  /* skip all white space (' ', '\n', '\t', '\r') */
  while (is_white_space(next_char, &newline_flag)) {
    next_char = fgetc(input_file);
    push(next_char, &peek_str);
  }
  /* skip all comments */
  // only consider this var for the following loop
  Comment_Types comment_type = NOT_STARTED;
  while (is_comment(next_char, &comment_type)) {
    if (comment_type == EOF_ERROR) {
      // TODO
      token.tp = ERR;
      strcpy(token.lx, "ERR: EOF in comment");
      token.ec = EofInCom;
      token.ln = newline_flag;
      strcpy(token.fl, input_filename);
      return token;
    }

    next_char = fgetc(input_file);
    push(next_char, &peek_str);
    newline_flag += next_char == '\n';
  }

  /* get EOF */
  if (next_char == EOF) {
    push(next_char, &peek_str);
    token.tp = EOFile;
  }

  token.tp = ERR;
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

  InitLexer(argv[1]);

  fclose(output_file);
  return TRUE;
}
// do not remove the next line
#endif
