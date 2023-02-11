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
FILE *input_file;
FILE *output_file;

typedef struct {
  int *stack;
  int size;
  int top;
} Stack;

void stack_init(Stack *stack, int size) {
  stack->stack = (int *)malloc(sizeof(int) * size);
  stack->size = size;
  stack->top = -1;
}

int push(int number, Stack *stack) {
  stack->top += 1;

  if (stack->top >= stack->size)
    return 0; // fail

  stack->stack[stack->top] = number;
  return 1;
}

int pop(int *pop_val, Stack *stack) {

  if (stack->top == -1)
    return 0; // fail

  *pop_val = stack->stack[stack->top];
  stack->top -= 1;

  return 1;
}
// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of
// the lexer If an error occurs, the function should return 0 if everything goes
// well the function should return 1
int InitLexer(char *file_name) { return 0; }

// Get the next token from the source file
Token GetNextToken() {
  Token t;
  t.tp = ERR;
  return t;
}

// peek (look) at the next token in the source file without removing it from the
// stream
Token PeekNextToken() {
  Token t;
  t.tp = ERR;
  return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer() { return 0; }

// do not remove the next line
#ifndef TEST
int main() {
  // implement your main function here
  // NOTE: the autograder will not use your main function

  return 0;
}
// do not remove the next line
#endif
