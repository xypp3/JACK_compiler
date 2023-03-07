#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

#define TRUE 1
#define FALSE 0

// helper func
int strcmpList(char *word, char **acceptCases, unsigned int numOfAccept) {
  for (int i = 0; i < numOfAccept; i++) {
    if (0 == strcmp(word, acceptCases[i]))
      return TRUE;
  }

  return FALSE;
}

void error(char *message, Token token) {
  printf("Expected: %s, got, %s, around line: %d", message, token.lx, token.ln);
  exit(1); // todo figure out better recovery
}

// top level grammer
int classDeclar();
int memberDeclar();
int classVarDeclar();
int type();
int subroutineDeclar();
int paramList();
int subroutineBody();

// statements
int stmt();
int varStmt();
int letStmt();
int ifStmt();
int whileStmt();
int doStmt();
int subroutineCall();
int exprList();
int returnStmt();
// expressions
int expr();
int relationalExpr();
int arithmeticExpr();
int term();
int factor();
int operand();
// function stubs above

int classDeclar() {

  Token token = GetNextToken();
  if (RESWORD == token.tp && 0 == strcmp(token.lx, "class")) {
  } else {
    error("'class' keyword", token);
  }

  token = GetNextToken();
  if (ID == token.tp) {
  } else {
    error("identifier", token);
  }

  token = GetNextToken();
  if (SYMBOL == token.tp && 0 == strcmp(token.lx, "{")) {
  } else {
    error("symbol '{'", token);
  }

  while (1 == memberDeclar())
    ;

  token = GetNextToken();
  if (SYMBOL == token.tp && 0 == strcmp(token.lx, "}")) {
  } else {
    error("symbol '}'", token);
  }

  return TRUE;
}

int memberDeclar() {
  Token peekT = PeekNextToken();
  // variable
  char *variableStart[] = {"static", "field"};
  if (RESWORD == peekT.tp && strcmpList(peekT.lx, variableStart, 2)) {
    classVarDeclar();
    return TRUE;
  }

  // subroutine
  char *subroutineStart[] = {"constructor", "function", "method"};
  if (RESWORD == peekT.tp && strcmpList(peekT.lx, subroutineStart, 3)) {
    subroutineDeclar();
    return TRUE;
  }

  return FALSE;
}

int classVarDeclar() {

  Token token = GetNextToken();

  char *variableStart[] = {"static", "field"};
  if (RESWORD == token.tp && strcmpList(token.lx, variableStart, 2)) {
  } else {
    error("class variable declaration, starting with 'static' or 'field' ",
          token);
  }

  token = PeekNextToken();
  char *typeStart[] = {"int", "char", "boolean", "identifier"};
  if (RESWORD == token.tp && strcmpList(token.lx, typeStart, 4)) {
    type();
  } else {
    error("type declaration", token);
  }

  token = GetNextToken();
  if (ID == token.tp) {
  } else {
    error("variable identifier", token);
  }

  // for the var declaration list
  do {
    token = PeekNextToken();

    if (SYMBOL == token.tp && 0 == strcmp(token.lx, ",")) {
    } else {
      break; // !!!!!!!!!!! EXIT CONDITION
    }

    token = GetNextToken(); // got the ","
    token = GetNextToken(); // got the identifier
    if (ID == token.tp) {
    } else {
      error("variable declaration list", token);
    }

  } while (TRUE); // exit if no comma

  token = GetNextToken();
  if (SYMBOL == token.tp && strcmp(token.lx, ";")) {
  } else {
    error("symbol ';'", token);
  }

  return TRUE;
}

int InitParser(char *file_name) { return 1; }

ParserInfo Parse() {
  ParserInfo pi;

  // implement the function

  pi.er = none;
  return pi;
}

int StopParser() { return 1; }

#ifndef TEST_PARSER
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./parser filename.jack");
    return 1;
  }

  InitParser(argv[1]);
  Parse();
  StopParser();

  return 1;
}
#endif
