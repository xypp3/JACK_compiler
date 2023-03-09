#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef enum { false, true } Boolean;

typedef struct {
  Boolean isRunning;
  ParserInfo info;
} CompleteParserInfo;

typedef struct {

  enum {
    success,   // upon success, finish excecuting
    soft_fail, // upon failure, finish excecuting
    hard_fail  // upon failure, stop excecuting
  } FailState;

  // on soft_fail type null
  union {
    Boolean isNull;
    ParserInfo info;
  } Data;

} typeX;

// top level grammer
ParserInfo classDeclar();
CompleteParserInfo memberDeclar();
ParserInfo classVarDeclar();
ParserInfo type();
ParserInfo subroutineDeclar();
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

/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Helper functions ***************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

Boolean strcmpList(char *word, char **acceptCases) {
  int pos = 0;
  // if empty acceptCases return true (for accepting ID tokens)
  if (0 == strcmp(acceptCases[0], "\0"))
    return true;
  // else test acceptCases
  while (strcmp(acceptCases[pos], "\0") != 0) {
    if (0 == strcmp(word, acceptCases[pos]))
      return true;

    pos++;
  }

  return false;
}

void error(char *message, Token token) {
  printf("Expected: %s, got, %s, around line: %d", message, token.lx, token.ln);
  exit(1); // todo figure out better recovery
}

ParserInfo consumeTerminal(TokenType type, char **acceptCases,
                           SyntaxErrors potentialErr) {
  Token token = GetNextToken();

  if (type == token.tp && 0 == strcmpList(token.lx, acceptCases)) {
    return (ParserInfo){none, token};
  }

  return (ParserInfo){potentialErr, token};
}

CompleteParserInfo consumeNonTerminal(ParserInfo (*func)(), TokenType type,
                                      char **acceptCases) {
  Token token = PeekNextToken();
  // token->tp = type;
  // strncpy(token->lx, "class", 10);

  // check beginning for non-terminal
  if (type == token.tp && strcmpList(token.lx, acceptCases)) {
    ParserInfo info = func();
    if (info.er == none) {
      // parser IS running && is NOT error
      return (CompleteParserInfo){true, info};
    } else {
      // parser is NOT running && IS error
      return (CompleteParserInfo){false, info};
    }
  }
  // parser is NOT running && is NOT error
  return (CompleteParserInfo){false, (ParserInfo){none, token}};
}

/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Grammer functions **************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Parser functions ***************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

int InitParser(char *file_name) {
  if (false == InitLexer(file_name))
    return false;

  return true;
}

ParserInfo Parse() {
  ParserInfo pi;

  // implement the function

  pi.er = none;
  return pi;
}

int StopParser() {
  if (false == StopLexer())
    return false;

  return true;
}

#ifndef TEST_PARSER
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./parser filename.jack");
    return 1;
  }

  // InitParser(argv[1]);
  // Parse();
  // StopParser();

  return 1;
}
#endif
