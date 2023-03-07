#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

#define TRUE 1
#define FALSE 0

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
  if (token.tp == RESWORD && strcmp(token.lx, "class")) {
  } else {
    error("'class' keyword", token);
  }

  token = GetNextToken();
  if (token.tp == ID) {
  } else {
    error("identifier", token);
  }

  token = GetNextToken();
  if (token.tp == SYMBOL && strcmp(token.lx, "{")) {
  } else {
    error("symbol '{'", token);
  }

  memberDeclar();

  token = GetNextToken();
  if (token.tp == SYMBOL && strcmp(token.lx, "}")) {
  } else {
    error("symbol '}'", token);
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
