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
Email:
sc21pkm@leeds.ac.uk
Date Work Commenced:
2023-Feb-09
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

typedef enum comment_types { NOT_COMMENT, ONELINE, MULTILINE } Comment_Types;

FILE *input_file;
FILE *output_file;
Stack peek_str;

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
int peek(FILE *fptr) {
  int peek_char = fgetc(fptr);
  return (peek_char == EOF) ? EOF : ungetc(peek_char, fptr);
}

int is_white_space(int next_char) {
  return next_char == ' ' || next_char == '\n' || next_char == '\t' ||
         next_char == '\r';
}

int is_comment(int next_char, Comment_Types *is_comment_started) {
  if (is_comment_started == NOT_COMMENT) {
    if (next_char == '/') {
      switch (peek(input_file)) {
      case ('/'):
        *is_comment_started = ONELINE;
        return TRUE;
      case ('*'):
        *is_comment_started = MULTILINE;
        return TRUE;
      default:
        return FALSE;
      }
    } else {
      return FALSE;
    }
  } else {
    // TODO: multiline case
    // TODO: oneline case
    // TODO: error case
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
int InitLexer(char *file_name) {
  // open input file
  if ((input_file = fopen(file_name, "r")) == NULL) {
    printf("Error! opening file");

    // Program exits if the file pointer returns NULL.
    return FALSE;
  }
  // create output file

  // init stack for PeekNextToken
  stack_init(&peek_str, MAX_LEXEM_SIZE);

  return TRUE;
}

// Get the next token from the source file
Token GetNextToken() {
  Token token;
  int next_char;

  next_char = fgetc(input_file);
  push(next_char, &peek_str);

  /* skip all white space (' ', '\n', '\t', '\r') */
  while (is_white_space(next_char)) {
    push(next_char, &peek_str);
    next_char = fgetc(input_file);
  }
  /* skip all comments */
  // only consider this var for the following loop
  Comment_Types comment_has_begun = NOT_COMMENT;
  while (1) {
    int comment_code = is_comment(next_char, &comment_has_begun);
    if (comment_code == TRUE) {
      push(next_char, &peek_str);
      next_char = fgetc(input_file);
    } else if (comment_code == EOF &&
               comment_has_begun !=
                   NOT_COMMENT) { // second condition may be redundant
      // TODO: EOF IN COMMENT ERROR
      break;
    } else {
      break;
    }
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
  while (peek_str.top > -1) {
    int prev_char; // tmp character
    pop(&prev_char, &peek_str);
    // return character to source file
    ungetc(prev_char, input_file);
  }

  return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer() {

  // FREE global vars
  free(input_file);
  free(output_file);
  free(peek_str.stack);

  return TRUE;
}

// do not remove the next line
#ifndef TEST
int main() {
  // implement your main function here
  // NOTE: the autograder will not use your main function

  return TRUE;
}
// do not remove the next line
#endif
