#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef enum { false, true } Boolean;

typedef struct {
  Boolean wasConsumed;
  ParserInfo info;
} ParserWrapper;

typedef struct {
  unsigned int length;
  TokenType *set; // array
} TokenTypeSet;

// predefined type sets
TokenTypeSet idSet = {1, (TokenType[]){ID}};
TokenTypeSet symbolSet = {1, (TokenType[]){SYMBOL}};
TokenTypeSet reswordSet = {1, (TokenType[]){RESWORD}};
TokenTypeSet typeSet = {1, (TokenType[]){ID, RESWORD}};
TokenTypeSet operandSet = {5, (TokenType[]){INT, ID, STRING, SYMBOL, RESWORD}};
// all return an EMPTY TOKEN upon success
// top level grammer
char *emtpyStart[] = {"\0"};
void classDeclar();
// void memberDeclar();
void classVarDeclar();
char *classVarDeclarStart[] = {"static", "field", "\0"};
void type();
char *typeStart[] = {"int", "char", "boolean", "\0"};
void subroutineDeclar();
char *subroutineDeclarStart[] = {"constructor", "function", "method", "\0"};
void paramList();
void subroutineBody();

// statements
void stmt();
char *stmtStart[] = {"var", "let", "if", "while", "do", "return", "\0"};
void varStmt();
void letStmt();
void ifStmt();
void whileStmt();
void doStmt();
void subroutineCall();
void exprList();
void returnStmt();
// expressions
void expr();
void relationalExpr();
void arithmeticExpr();
void term();
void factor();
// id, int, or strings below
char *factorStart[] = {"-", "~", "(", "true", "false", "null", "this"};
void operand();
// id, int, or strings below
char *operandStart[] = {"(", "true", "false", "null", "this"};

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
    if (0 == strncmp(word, acceptCases[pos], 128))
      return true;

    pos++;
  }

  return false;
}

// We get to exit() !!!!!!!!!! Wooohoooo
void error(Token token, char *err, SyntaxErrors exitCode) {
  printf("< Token found: %s, on line: %d >< Expected: %s >\n", token.lx,
         token.ln, err);
  exit(exitCode);
}

// consumption wrapper
void eatTerminal(TokenTypeSet typeSet, char **acceptCases,
                 SyntaxErrors potentialErr, char *errMsg) {
  Token token = GetNextToken();

  // check lexer error
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  // consume terminal token
  for (int i = 0; i < typeSet.length; i++) {
    switch (typeSet.set[i]) {
    case SYMBOL:
    case RESWORD:
      if (typeSet.set[i] == token.tp && strcmpList(token.lx, acceptCases))
        return;

    case ID:
    case INT:
    case STRING:
    case EOFile:
      if (typeSet.set[i] == token.tp)
        return;

    // error already processed (unless error hunting in future, hmh)
    case ERR:
      break;
    }
  }

  error(token, errMsg, potentialErr);
}

/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Grammer functions **************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

void classDeclar() {
  ParserWrapper info;
  Token token;
  // class
  eatTerminal(reswordSet, (char *[]){"class", "\0"}, classExpected,
              "'class' resword");

  // ID
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  // '{'
  eatTerminal(symbolSet, (char *[]){"{", "\0"}, openBraceExpected,
              "'{' symbol");

  // { memberDeclar }
  while (true) {

    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (RESWORD != token.tp && !strcmpList(token.lx, classVarDeclarStart) &&
        !strcmpList(token.lx, subroutineDeclarStart))
      break;

    // classVarDeclar()
    if (strcmpList(token.lx, classVarDeclarStart)) {
      classVarDeclar();
      continue;
    }

    // subroutineDeclar()
    if (strcmpList(token.lx, subroutineDeclarStart)) {
      subroutineDeclar();
      continue;
    }
  }

  // '}'
  eatTerminal(symbolSet, (char *[]){"}", "\0"}, closeBraceExpected,
              "'}' symbol");
}

void classVarDeclar() {
  ParserWrapper info;
  Token token;

  // 'static' | 'field'
  eatTerminal(reswordSet, (char *[]){"static", "field", "\0"}, classVarErr,
              "'static' or 'field' resword");

  // type()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (ID == token.tp ||
      (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
    type();
    // should i check even though is already checked above??
  } else {
    error(token, "valid type token", illegalType);
  }

  // identifier
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  // {, identifier}
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (SYMBOL != token.tp && !strcmpList(token.lx, (char *[]){",", "\0"})) {
      break;
    }

    token = GetNextToken(); // get ','
    eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

    // (to stretch whitespace in formatter)
  }

  // ';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected,
              "';' symbol");
}

void type() {
  eatTerminal(typeSet, typeStart, illegalType, "valid type token");
}

void subroutineDeclar() {
  Token token;

  // 'constructor' | 'function' | 'method'
  eatTerminal(reswordSet, (char *[]){"constructor", "function", "method", "\0"},
              subroutineDeclarErr,
              "'constructor' or 'function' or 'method' resword");

  // type() | 'void'
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (ID == token.tp ||
      (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
    type();
  } else {
    // 'void'
    eatTerminal(reswordSet, (char *[]){"void", "\0"}, illegalType,
                "expected type or 'void' token");
  }

  // identifier
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  // '('
  eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
              "'(' symbol");

  // paramList()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  // if paramList() is NOT empty
  if (SYMBOL != token.tp && !strcmpList(token.lx, (char *[]){")", "\0"})) {

    if (ID == token.tp ||
        (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
      paramList();
    } else {
      error(token, "paramiter list", syntaxError);
    }
  }

  // ')'
  eatTerminal(symbolSet, (char *[]){")", "\0"}, closeParenExpected,
              "')' symbol");

  // subroutineBody()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"{", "\0"})) {
    subroutineBody();
  } else {
    error(token, "'{' to start a subroutine body", syntaxError);
  }
}

void paramList() {
  Token token;

  // type()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (ID == token.tp ||
      (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
    type();
    // should i check even though is already checked above??
  } else {
    error(token, "valid type token", illegalType);
  }

  // identifier
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  // {',' type() identifier}
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (SYMBOL != token.tp && !strcmpList(token.lx, (char *[]){",", "\0"})) {
      break;
    }

    token = GetNextToken(); // get ','

    // type()
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);
    if (ID == token.tp ||
        (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
      type();
      // should i check even though is already checked above??
    } else {
      error(token, "valid type token", illegalType);
    }

    // identifier
    eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");
  }
}

void subroutineBody() {
  Token token;

  // '{'
  eatTerminal(symbolSet, (char *[]){"{", "\0"}, openBraceExpected,
              "'{' symbol");

  // { stmt() }
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    if (RESWORD != token.tp && !strcmpList(token.lx, stmtStart))
      break;

    stmt();
  }

  // '}'
  eatTerminal(symbolSet, (char *[]){"}", "\0"}, closeBraceExpected,
              "'}' symbol");
}

void stmt() {
  Token token;

  token = PeekNextToken();

  if (0 == strncmp(token.lx, "var", 128)) {
    varStmt();
  } else if (0 == strncmp(token.lx, "let", 128)) {
    letStmt();
  } else if (0 == strncmp(token.lx, "if", 128)) {
    ifStmt();
  } else if (0 == strncmp(token.lx, "while", 128)) {
    whileStmt();
  } else if (0 == strncmp(token.lx, "do", 128)) {
    doStmt();
  } else if (0 == strncmp(token.lx, "return", 128)) {
    returnStmt();
  } else {
    error(token, "valid statement start token", syntaxError);
  }
}

void varStmt() {
  Token token;

  // 'var'
  eatTerminal(reswordSet, (char *[]){"var", "\0"}, syntaxError,
              "'var' resword expected");

  // type()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (ID == token.tp ||
      (RESWORD == token.tp && strcmpList(token.lx, typeStart))) {
    type();
    // should i check even though is already checked above??
  } else {
    error(token, "valid type token", illegalType);
  }

  // identifier
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  // {, identifier}
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (SYMBOL != token.tp && !strcmpList(token.lx, (char *[]){",", "\0"})) {
      break;
    }

    token = GetNextToken(); // get ','
    eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

    // (to stretch whitespace in formatter)
  }

  // ';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected,
              "';' symbol");
}

void letStmt() {
  Token token;

  // 'let'
  eatTerminal(reswordSet, (char *[]){"let", "\0"}, syntaxError,
              "'let' resword expected");

  // identifier
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  // [ '[' expr() ']' ]
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  // if not equals THEN THERE is a'[' expr() ']'
  if (SYMBOL != token.tp && !strcmpList(token.lx, (char *[]){"=", "\0"})) {

    // '['
    eatTerminal(symbolSet, (char *[]){"[", "\0"}, syntaxError, "'[' symbol");

    // expr()
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);
    if (RESWORD == token.tp && strcmpList(token.lx, factorStart)) {
      expr();
    } else {
      error(token, "a expression", syntaxError);
    }

    // ']'
    eatTerminal(symbolSet, (char *[]){"]", "\0"}, closeBracketExpected,
                "']' symbol");
  }

  // '='
  eatTerminal(symbolSet, (char *[]){"=", "\0"}, equalExpected, "'=' symbol");

  // expr()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (RESWORD == token.tp && strcmpList(token.lx, factorStart)) {
    expr();
  } else {
    error(token, "a expression", syntaxError);
  }

  // ';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected,
              "';' symbol");
}

void ifStmt() {
  Token token;

  // 'if'
  eatTerminal(reswordSet, (char *[]){"if", "\0"}, syntaxError,
              "'if' resword expected");

  // '('
  eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
              "'(' symbol");

  // expr()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (RESWORD == token.tp && strcmpList(token.lx, factorStart)) {
    expr();
  } else {
    error(token, "a expression", syntaxError);
  }

  // ')'
  eatTerminal(symbolSet, (char *[]){")", "\0"}, closeParenExpected,
              "')' symbol");

  // '{'
  eatTerminal(symbolSet, (char *[]){"{", "\0"}, openBraceExpected,
              "'{' symbol");

  // { stmt() }
  while (true) {
    // stmt()
    token = PeekNextToken();
    // error case
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);
    // break case
    if (RESWORD != token.tp && !strcmpList(token.lx, stmtStart)) {
      break;
    }
    // if statement has begun do it
    stmt();
  }

  // '}'
  eatTerminal(symbolSet, (char *[]){"}", "\0"}, closeBraceExpected,
              "'}' symbol");

  // [ else '{' {stmt()} '}' ]
  // 'else'
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (RESWORD == token.tp && strcmpList(token.lx, (char *[]){"else", "\0"})) {
    token = GetNextToken(); // get the 'else' token
    // '{'
    eatTerminal(symbolSet, (char *[]){"{", "\0"}, openBraceExpected,
                "'{' symbol");

    // { stmt() }
    while (true) {
      token = PeekNextToken();
      if (ERR == token.tp)
        error(token, "valid lexical token", lexerErr);
      if (RESWORD != token.tp && !strcmpList(token.lx, stmtStart))
        break;

      stmt();
    }

    // '}'
    eatTerminal(symbolSet, (char *[]){"}", "\0"}, closeBraceExpected,
                "'}' symbol");
  }
}

void whileStmt() {
  Token token;

  // while
  eatTerminal(reswordSet, (char *[]){"while", "\0"}, syntaxError,
              "'while' resword expected");

  // '('
  eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
              "'(' symbol");

  // expr()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (RESWORD == token.tp && strcmpList(token.lx, factorStart)) {
    expr();
  } else {
    error(token, "a expression", syntaxError);
  }

  // ')'
  eatTerminal(symbolSet, (char *[]){")", "\0"}, closeParenExpected,
              "')' symbol");

  // '{'
  eatTerminal(symbolSet, (char *[]){"{", "\0"}, openBraceExpected,
              "'{' symbol");

  // { stmt() }
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);
    if (RESWORD != token.tp && !strcmpList(token.lx, stmtStart))
      break;

    stmt();
  }

  // '}'
  eatTerminal(symbolSet, (char *[]){"}", "\0"}, closeBraceExpected,
              "'}' symbol");
}

void doStmt() {
  Token token;

  // 'do'
  eatTerminal(reswordSet, (char *[]){"do", "\0"}, syntaxError,
              "'do' resword expected");

  // subroutineCall();
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (ID == token.tp) {
    subroutineCall();
  } else {
    error(token, "valid subroutineCall", syntaxError);
  }

  // ';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected,
              "';' symbol");
}

/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Parser functions ***************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

int InitParser(char *file_name) { return InitParser(file_name); }

ParserInfo Parse() {
  ParserInfo pi;

  // implement the function

  pi.er = none;
  return pi;
}

int StopParser() { return StopLexer(); }

#ifndef TEST_PARSER
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./parser filename.jack");
    return 1;
  }

  InitParser(argv[1]);
  // Parse();

  // ParserWrapper info =
  //     consumeTerminal(RESWORD, (char *[]){"class", "\0"}, classExpected);
  //
  // printf("Has run: %d, data: %d, %s\n", info.hasRun, info.info.er,
  //        info.info.tk.lx);
  //
  // info = consumeTerminal(ID, (char *[]){"\0"}, idExpected);
  //
  // printf("Has run: %d, data: %d, %s\n", info.hasRun, info.info.er,
  //        info.info.tk.lx);

  StopParser();

  return 1;
}
#endif
