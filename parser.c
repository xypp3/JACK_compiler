#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef enum { false, true } Boolean;

typedef union {
  Boolean isRunning;
  ParserInfo info;
} CompleteParserInfo;
// helper func

// ONLY WORKS FOR LIST < 10
Boolean strcmpList(char *word, char **acceptCases) {
  int pos = 0;
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

ParserInfo consumeTerminal(Token token, TokenType type, char **acceptCases,
                           SyntaxErrors potentialErr) {
  token = GetNextToken();

  if (type == token.tp && 0 == strcmpList(token.lx, acceptCases)) {
    return (ParserInfo){none, token};
  }

  return (ParserInfo){potentialErr, token};
}

CompleteParserInfo consumeNonTerminal(Token token, ParserInfo (*func)(),
                                      TokenType type, char **acceptCases) {
  // token = PeekNextToken();
  token.tp = type;
  strncpy(token.lx, "class", 10);

  // check beginning for non-terminal
  if (type == token.tp && strcmpList(token.lx, acceptCases)) {
    return (CompleteParserInfo)func();
  }

  return (CompleteParserInfo){false};
}

// top level grammer
ParserInfo classDeclar();
CompleteParserInfo memberDeclar();
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

ParserInfo classDeclar() {

  Token token;
  //   if (!consumeTerminal(&token, RESWORD, (char *[]){"class", "\0"})) {
  //     error("'class' keyword", token);
  //   }
  //   // Token token = GetNextToken();
  //   // if (RESWORD == token.tp && 0 == strcmp(token.lx, "class")) {
  //   // } else {
  //   //   error("'class' keyword", token);
  //   // }
  //
  //   token = GetNextToken();
  //   if (ID == token.tp) {
  //   } else {
  //     error("identifier", token);
  //   }
  //
  //   if (!consumeTerminal(&token, SYMBOL, (char *[]){"{", "\0"})) {
  //     error("symbol '{'", token);
  //   }
  //   // token = GetNextToken();
  //   // if (SYMBOL == token.tp && 0 == strcmp(token.lx, "{")) {
  //   // } else {
  //   //   error("symbol '{'", token);
  //   // }
  //
  //   while (1 == memberDeclar())
  //     ;
  //
  //   if (!consumeTerminal(&token, SYMBOL, (char *[]){"}", "\0"})) {
  //     error("symbol '}'", token);
  //   }
  //   // token = GetNextToken();
  //   // if (SYMBOL == token.tp && 0 == strcmp(token.lx, "}")) {
  //   // } else {
  //   //   error("symbol '}'", token);
  //   // }
  //
  return (ParserInfo){none, token};
}
//
// CompleteParserInfo memberDeclar() {
//   Token token;
//   // Token peekT = PeekNextToken();
//   // variable
//   char *variableStart[] = {"static", "field", "\0"};
//   // if (RESWORD == peekT.tp && strcmpList(peekT.lx, variableStart)) {
//   //   classVarDeclar();
//   //   return TRUE;
//   // }
//   if (consumeNonTerminal(&token, &classVarDeclar, RESWORD, variableStart))
//   {
//     return TRUE;
//   }
//
//   // subroutine
//   char *subroutineStart[] = {"constructor", "function", "method", "\0"};
//   // if (RESWORD == peekT.tp && strcmpList(peekT.lx, subroutineStart)) {
//   //   subroutineDeclar();
//   //   return TRUE;
//   // }
//   if (consumeNonTerminal(&token, &subroutineDeclar, RESWORD,
//   subroutineStart)) {
//     return TRUE;
//   }
//
// return (CompleteParserInfo){false};
// }
//
// int classVarDeclar() {
//   Token token;
//   // Token token = GetNextToken();
//
//   char *variableStart[] = {"static", "field", "\0"};
//   // if (RESWORD == token.tp && strcmpList(token.lx, variableStart)) {
//   // } else {
//   //   error("class variable declaration, starting with 'static' or 'field'
//   ",
//   //         token);
//   // }
//   if (!consumeTerminal(&token, RESWORD, variableStart)) {
//     error("class var declaration, 'static' or 'field'", token);
//   }
//
//   // Token token = PeekNextToken();
//   char *typeStart[] = {"int", "char", "boolean", "identifier", "\0"};
//   // if (RESWORD == token.tp && strcmpList(token.lx, typeStart)) {
//   //   type();
//   // } else {
//   //   error("type declaration", token);
//   // }
//   if (!consumeNonTerminal(&token, &type, RESWORD, typeStart)) {
//     error("type declaration", token);
//   }
//
//   token = GetNextToken();
//   if (ID == token.tp) {
//   } else {
//     error("variable identifier", token);
//   }
//
//   // for the var declaration list
//   do {
//     token = PeekNextToken();
//
//     if (SYMBOL == token.tp && 0 == strcmp(token.lx, ",")) {
//     } else {
//       break; // !!!!!!!!!!! EXIT CONDITION
//     }
//
//     token = GetNextToken(); // got the ","
//     token = GetNextToken(); // got the identifier
//     if (ID == token.tp) {
//     } else {
//       error("variable declaration list", token);
//     }
//
//   } while (TRUE); // exit if no comma
//
//   // token = GetNextToken();
//   // if (SYMBOL == token.tp && strcmp(token.lx, ";")) {
//   // } else {
//   //   error("symbol ';'", token);
//   // }
//   if (!consumeTerminal(&token, SYMBOL, (char *[]){";", "\0"})) {
//     error("symbol ';'", token);
//   }
//
//   return TRUE;
// }
//
// int type() {
//   // Token token = GetNextToken();
//
//   char *typeStart[] = {"int", "char", "boolean", "identifier", "\0"};
//   // if (RESWORD == token.tp && strcmpList(token.lx, typeStart)) {
//   // } else {
//   //   error("type declaration", token);
//   // }
//   return consumeTerminal(&token, RESWORD, typeStart);
// }
//
// int subroutineDeclar() {
//   Token token = GetNextToken();
//
//   char *subroutineStart[] = {"constructor", "function", "method", "\0"};
//   if (RESWORD == token.tp && strcmpList(token.lx, subroutineStart)) {
//   } else {
//     error("subroutine start keyword", token);
//   }
//
//   token = PeekNextToken();
//   char *typeStart[] = {"int", "char", "boolean", "identifier", "\0"};
//   // if (RESWORD == token.tp && strcmpList(token.lx, typeStart)) {
//   //   type();
//   // } else {
//   //   // if not type() THEN void OR err
//   //   token = GetNextToken();
//   //   if (RESWORD == token.tp && 0 == strcmp(token.lx, "void")) {
//   //   } else {
//   //     error("type or void", token);
//   //   }
//   // }
//   if (RESWORD == token.tp && strcmp(token.lx, "void")) {
//   } else if (TRUE == consumeNonTerminal(&type, RESWORD, typeStart)) {
//   } else {
//     error("either void, int, char, bool, id required", token);
//   }
//
//   return TRUE;
// }
//
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

  Token token;
  CompleteParserInfo info = consumeNonTerminal(token, classDeclar, RESWORD,
                                               (char *[]){"claaaass", "\0"});
  printf("%d, %s\n", info.isRunning, "hi");

  return 1;
}
#endif
