#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef enum { false, true } Boolean;

typedef struct {
  Boolean hasRun;
  ParserInfo info;
} ParserWrapper;

// top level grammer
ParserWrapper classDeclar();
ParserWrapper memberDeclar();
ParserWrapper classVarDeclar();
ParserWrapper type();
ParserWrapper subroutineDeclar();
ParserWrapper paramList();
ParserWrapper subroutineBody();

// statements
ParserWrapper stmt();
ParserWrapper varStmt();
ParserWrapper letStmt();
ParserWrapper ifStmt();
ParserWrapper whileStmt();
ParserWrapper doStmt();
ParserWrapper subroutineCall();
ParserWrapper exprList();
ParserWrapper returnStmt();
// expressions
ParserWrapper expr();
ParserWrapper relationalExpr();
ParserWrapper arithmeticExpr();
ParserWrapper term();
ParserWrapper factor();
ParserWrapper operand();
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

// Tokens returned unchanged
ParserWrapper consumeTerminal(TokenType type, char **acceptCases,
                              SyntaxErrors potentialErr) {
  Token token = GetNextToken();

  if (type == token.tp && 0 == strcmpList(token.lx, acceptCases)) {
    return (ParserWrapper){true, none, token};
  }

  return (ParserWrapper){true, potentialErr, token};
}

// Tokens returned unchanged
ParserWrapper consumeNonTerminal(ParserWrapper (*func)(), TokenType type,
                                 char **acceptCases) {
  Token token = PeekNextToken();

  // check beginning for non-terminal
  if (type == token.tp && strcmpList(token.lx, acceptCases)) {
    // ( consume HAS run && IS error ) OR ( consume HAS run && NO error )
    return func();
  }
  // consume NOT run && MAYBE error
  //    Top lvl. func. is for determining if error or not
  return (ParserWrapper){false, (ParserInfo){none, token}};
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
