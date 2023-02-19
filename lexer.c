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
#define NUM_OF_RESWORDS 21

typedef struct {
  int *stack;
  int size;
  int top;
} Stack;

typedef enum comment_types {
  NOT_STARTED,
  ONELINE,
  MULTILINE,
  EOF_ERROR
} Comment_Types;

/* reserved words */
char *all_reserved_words[] = {
    "class", "constructor", "method", "function", "int",   "boolean", "char",
    "void",  "var",         "static", "field",    "let",   "do",      "if",
    "else",  "while",       "return", "true",     "false", "null",    "this"};

FILE *input_file;
char input_filename[32]; // defined by header file
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

    if (stack->stack == NULL) {
      free(stack->stack);
      return FALSE;
    }

    stack->size *= 2;
  }

  stack->stack[stack->top] = number;
  return TRUE;
}

int pop(int *pop_val, Stack *stack) {
  if (stack->top == -1)
    return FALSE; // fail

  if (pop_val != NULL)
    *pop_val = stack->stack[stack->top];
  stack->top -= 1;

  return TRUE;
}

int clear_stack(Stack *stack) {

  if (stack->top == -1)
    return FALSE; // fail

  while (stack->top > -1) {
    pop(NULL, stack);
  }

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

int is_reserved_word(char *word) {
  for (int i = 0; i < NUM_OF_RESWORDS; i++) {
    if (strcmp(word, all_reserved_words[i]) == 0)
      return TRUE;
  }
  return FALSE;
}

int is_comment(int next_char, Comment_Types *comment_type) {
  if (*comment_type == NOT_STARTED) {
    if (next_char == '/') {
      switch (peek_char(input_file)) {
      case ('/'):
        *comment_type = ONELINE;
        return TRUE;
      case ('*'):
        *comment_type = MULTILINE;
        return TRUE;
      default:
        return FALSE;
      }
    } else {
      return FALSE;
    }
  } else {

    if (next_char == EOF) {
      *comment_type = EOF_ERROR;
      return TRUE; // cause it is a comment just a faulty one
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
      if (peek_str.stack[peek_str.top - 2] == '*' &&
          peek_str.stack[peek_str.top - 1] == '/') {
        *comment_type = NOT_STARTED;

        // if multiline comment right after multiline comment do this
        if (peek_str.stack[peek_str.top] == '/') {
          return is_comment(next_char, comment_type);
        }

        return FALSE;
      }

      return TRUE;

    default:
      // printf("Error has occured, in is_comment()");
      return FALSE;
    }
  }
  return FALSE;
}

char *enumToStr(TokenType type) {
  switch (type) {
  case RESWORD:
    return "RESWORD";
  case ID:
    return "ID";
  case INT:
    return "INT";
  case SYMBOL:
    return "SYMBOL";
  case STRING:
    return "STRING";
  case EOFile:
    return "EOFile";
  case ERR:
    return "ERR";
  }
  return "null";
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

    // Program exits if the file pointer returns NUL
    return FALSE;
  }

  strncpy(input_filename, file + 2, strlen(file));

  // init stack for PeekNextToken
  stack_init(&peek_str, MAX_LEXEM_SIZE);
  token_line = 1;
  previous_token_line_num = 1;

  return TRUE;
}

// Get the next token from the source file
Token GetNextToken() {
  Token token;
  int next_char;
  previous_token_line_num = token_line;
  clear_stack(&peek_str);

  next_char = fgetc(input_file);
  push(next_char, &peek_str);

  /* skip all white space (' ', '\f', '\n', '\t', '\r', '\v') */
  /* AND skip all comments */
  /* REUTNRS 1 ERR token */
  Comment_Types comment_type = NOT_STARTED; // tmp var for this loop
  // check comment first so condition doesn't short circuit in wrong way
  while (is_comment(next_char, &comment_type) || isspace(next_char)) {
    // printf("%c, %d, ln:%d  ", next_char, comment_type, token_line);

    token_line += next_char == '\n';

    if (comment_type == EOF_ERROR) {
      token.tp = ERR;
      strcpy(token.lx, "Error: unexpected eof in comment");
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
    token.tp = EOFile;
    strcpy(token.lx, "End of File");
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
        token.tp = ERR;
        token.ec = EofInStr;
        strcpy(token.lx, "Error: unexpected eof in string constant");
        token.ln = token_line;
        strcpy(token.fl, input_filename);

        return token;
      } else if (next_char == '\n') {
        token.tp = ERR;
        token.ec = NewLnInStr;
        strcpy(token.lx, "Doesn't seem to have a thing");
        token.ln = token_line;
        strcpy(token.fl, input_filename);

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

    return token;
  }

  /* numeric constants */
  /* RETURNS 1 INT token */
  if (isdigit(next_char)) {
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
    // return non digit character to input stream
    ungetc(next_char, input_file);
    pop(NULL, &peek_str);
    // tokenize number
    token.tp = INT;
    token.ln = token_line;
    strcpy(token.fl, input_filename);

    return token;
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

  /* identifiers/ reserved words */
  /* RETURNS 1 ID or RESWORD token */
  unsigned short char_pos = 0;
  if (is_valid_identifier(next_char, char_pos)) {
    // set first char
    token.lx[char_pos] = next_char;
    token.lx[char_pos + 1] = '\0';
    // setup loop
    char_pos++;
    next_char = fgetc(input_file);
    push(next_char, &peek_str);
    // loop
    while (is_valid_identifier(next_char, char_pos)) {
      // max store length of lexem of 127 characters
      if (char_pos < 128) {
        token.lx[char_pos] = next_char;
        token.lx[char_pos + 1] = '\0';
      }

      char_pos++;
      next_char = fgetc(input_file);
      push(next_char, &peek_str);
    }
    // return non digit character to input stream
    ungetc(next_char, input_file);
    pop(NULL, &peek_str);
    /* tokenize if reserved word or if identifer */
    if (is_reserved_word(token.lx)) {
      token.tp = RESWORD;
    } else {
      token.tp = ID;
    }

    token.ln = token_line;
    strcpy(token.fl, input_filename);

    return token;
  }

  /* illegal char */
  token.tp = ERR;
  token.ec = IllSym;
  strcpy(token.lx, "Error: illegal symbol in source file");
  token.ln = token_line;
  strcpy(token.fl, input_filename);

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

  if (argc != 2 && argc != 3) {
    printf("Usage: lexer \"filename\"");
    return FALSE;
  }

  int is_not_err = InitLexer(argv[1]);
  if (is_not_err == FALSE) {
    printf("init err");
    return FALSE;
  }

  // init output file
  FILE *output_file;
  char output_filename[50];
  strcat(output_filename, argv[1]);
  strcat(output_filename, "_tokens_mine.txt");

  if ((output_file = fopen(output_filename, "w")) == NULL) {
    printf("Error! opening file");

    fclose(output_file);
    // Program exits if the file pointer returns NULL.
    return FALSE;
  }

  PeekNextToken();
  PeekNextToken();
  Token p = PeekNextToken();
  Token t = GetNextToken();
  // printf("< %s, %d, %s, %s >\n", t.fl, t.ln, t.lx, enumToStr(t.tp));
  if (argc == 3 && strcmp(argv[2], "peek") == 0) {
    fprintf(output_file, "< %s, %d, %s, %s >\n", p.fl, p.ln, p.lx,
            enumToStr(p.tp));
  } else {
    fprintf(output_file, "< %s, %d, %s, %s >\n", t.fl, t.ln, t.lx,
            enumToStr(t.tp));
  }
  while (t.tp != EOFile && t.tp != ERR) {
    PeekNextToken();
    PeekNextToken();
    p = PeekNextToken();
    t = GetNextToken();
    if (argc == 3 && strcmp(argv[2], "peek") == 0) {
      fprintf(output_file, "< %s, %d, %s, %s >\n", p.fl, p.ln, p.lx,
              enumToStr(p.tp));
    } else {
      fprintf(output_file, "< %s, %d, %s, %s >\n", t.fl, t.ln, t.lx,
              enumToStr(t.tp));
    }
  }

  StopLexer();

  fclose(output_file);
  return TRUE;
}
// do not remove the next line
#endif
