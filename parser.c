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

typedef struct {
  unsigned int length;
  TokenType *set; // array
} TokenTypeSet;

// all return an EMPTY TOKEN upon success
// top level grammer
char *emtpyStart[] = {"\0"};
ParserWrapper classDeclar();
ParserWrapper memberDeclar();
ParserWrapper classVarDeclar();
char *classVarDeclarStart[] = {"static", "field", "\0"};
ParserWrapper type();
char *typeStart[] = {"int", "char", "boolean", "\0"};
ParserWrapper subroutineDeclar();
char *subroutineDeclarStart[] = {"constructor", "function", "method", "\0"};
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
ParserWrapper consumeTerminal(TokenTypeSet typeSet, char **acceptCases,
                              SyntaxErrors potentialErr) {
  Token token = GetNextToken();

  // check lexer error
  if (token.tp == ERR)
    return (ParserWrapper){true, lexerErr, token};

  // consume terminal token
  for (int i = 0; i < typeSet.length; i++) {
    switch (typeSet.set[i]) {
    case SYMBOL:
    case RESWORD:
      if (typeSet.set[i] == token.tp && strcmpList(token.lx, acceptCases))
        return (ParserWrapper){true, none, token};
      break;

    case ID:
    case INT:
    case STRING:
    case EOFile:
      if (typeSet.set[i] == token.tp)
        return (ParserWrapper){true, none, token};
      break;

    // error already processed (unless error hunting in future, hmh)
    case ERR:
      break;
    }
  }

  return (ParserWrapper){true, potentialErr, token};
}

// Tokens returned unchanged
ParserWrapper consumeNonTerminal(ParserWrapper (*func)(), TokenTypeSet typeSet,
                                 char **acceptCases) {
  Token token = PeekNextToken();

  // check lexer error
  if (token.tp == ERR)
    return (ParserWrapper){true, lexerErr, token};

  // check beginning for non-terminal
  for (int i = 0; i < typeSet.length; i++) {
    switch (typeSet.set[i]) {
    case SYMBOL:
    case RESWORD:
      if (typeSet.set[i] == token.tp && strcmpList(token.lx, acceptCases))
        // ( consume HAS run && IS error ) OR ( consume HAS run && NO error )
        /*    HAS run in this context means correct start of non-terminal but
           encountered error in middle */
        return func();
      break;

    case ID:
    case INT:
    case STRING:
    case EOFile:
      if (typeSet.set[i] == token.tp)
        // ( consume HAS run && IS error ) OR ( consume HAS run && NO error )
        /*    HAS run in this context means correct start of non-terminal but
           encountered error in middle */
        return func();
      break;

    // error already processed (unless error hunting in future, hmh)
    case ERR:
      break;
    }
  }

  // consume NOT run && MAYBE error
  /*    Top lvl. func. is for determining if error or not   */
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

ParserWrapper classDeclar() {
  ParserWrapper info;

  // 'class'
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){RESWORD}},
                         (char *[]){"class", "\0"}, classExpected);
  if (info.info.er != none)
    return info;

  // id
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){ID}}, (char *[]){"\0"},
                         idExpected);
  if (info.info.er != none)
    return info;

  // '{'
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){SYMBOL}},
                         (char *[]){"{", "\0"}, openBraceExpected);
  if (info.info.er != none)
    return info;

  // { memberDeclar() }
  while (true) {
    info = memberDeclar();

    if (false == info.hasRun)
      break;

    if (none != info.info.er)
      return info;
  }

  // '}'
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){SYMBOL}},
                         (char *[]){"}", "\0"}, closeBraceExpected);
  if (info.info.er != none)
    return info;

  // empty token returned
  Token token;
  return (ParserWrapper){true, (ParserInfo){none, token}};
}

// classVarDeclar() || subroutineDeclar()
ParserWrapper memberDeclar() {
  ParserWrapper info;

  // classVarDeclar()
  info = consumeNonTerminal(&classVarDeclar,
                            (TokenTypeSet){1, (TokenType[]){RESWORD}},
                            classVarDeclarStart);
  if (info.hasRun)
    return info;

  // subroutineDeclar()
  info = consumeNonTerminal(&subroutineDeclar,
                            (TokenTypeSet){1, (TokenType[]){RESWORD}},
                            subroutineDeclarStart);
  if (info.hasRun)
    return info;

  // check for error, memberDeclarErr
  Token token = PeekNextToken();
  if (0 != strncmp("}", token.lx, 1))
    return (ParserWrapper){true, (ParserInfo){memberDeclarErr, token}};

  // hasn't run any of the internals so it can end
  return (ParserWrapper){false, (ParserInfo){none, token}};
}

ParserWrapper classVarDeclar() {
  ParserWrapper info;

  // 'static' || 'field'
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){RESWORD}},
                         subroutineDeclarStart, classVarErr);
  if (info.info.er != none)
    return info;

  // type()
  info = consumeNonTerminal(
      &type, (TokenTypeSet){2, (TokenType[]){ID, RESWORD}}, typeStart);
  if (false == info.hasRun || info.info.er != none)
    // PRAYING THIS WORKS
    return (ParserWrapper){info.hasRun,
                           (ParserInfo){illegalType, info.info.tk}};

  // identifier
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){ID}}, emtpyStart,
                         idExpected);
  if (info.info.er != none)
    return info;

  // {',' identifier}
  Token token;
  while (true) {
    token = PeekNextToken();

    if (SYMBOL != token.tp && 0 != strncmp(",", token.lx, 128))
      break;

    token = GetNextToken(); // gets ','

    // identifier
    info = consumeTerminal((TokenTypeSet){1, (TokenType[]){ID}}, emtpyStart,
                           idExpected);
    if (info.info.er != none)
      return info;
  }

  // ';'
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){SYMBOL}},
                         (char *[]){";", "\0"}, semicolonExpected);
  if (info.info.er != none)
    return info;

  // empty token returned
  return (ParserWrapper){true, (ParserInfo){none, token}};
}

ParserWrapper type() {
  return consumeTerminal((TokenTypeSet){2, (TokenType[]){ID, RESWORD}},
                         typeStart, illegalType);
}

ParserWrapper subroutineDeclar() {
  ParserWrapper info;

  // 'constructor' | 'function' | 'method'
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){RESWORD}},
                         subroutineDeclarStart, subroutineDeclarErr);
  if (info.info.er != none)
    return info;

  // type() | 'void'
  //    type()
  info = consumeNonTerminal(
      &type, (TokenTypeSet){2, (TokenType[]){ID, RESWORD}}, typeStart);
  if (info.hasRun) {
    if (info.info.er != none)
      return info;

  } else {
    // 'void'
    info = consumeTerminal((TokenTypeSet){1, (TokenType[]){RESWORD}},
                           (char *[]){"void", "\0"}, syntaxError);
    if (info.info.er != none)
      return info;

    // if this part errors (type() | 'void') then should result in syntaxError
  }

  // identifier
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){ID}}, emtpyStart,
                         idExpected);
  if (info.info.er != none)
    return info;

  // '('
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){SYMBOL}},
                         (char *[]){"(", "\0"}, openParenExpected);
  if (info.info.er != none)
    return info;

  // paramlist()
  info = consumeNonTerminal(
      &paramList, (TokenTypeSet){2, (TokenType[]){ID, RESWORD}}, typeStart);
  if (!info.hasRun || info.info.er != none) {
    Token t = PeekNextToken();
    // if not empty paramList
    if (SYMBOL != t.tp && 0 != strncmp(")", t.lx, 128))
      return info;
  }

  // ')'
  info = consumeTerminal((TokenTypeSet){1, (TokenType[]){SYMBOL}},
                         (char *[]){")", "\0"}, closeParenExpected);
  if (info.info.er != none)
    return info;

  // subroutineBody()
  info = consumeNonTerminal(&subroutineBody,
                            (TokenTypeSet){1, (TokenType[]){SYMBOL}},
                            (char *[]){"{", "\0"});
  // if errors before body runs
  Token token;
  if (!info.hasRun)
    return (ParserWrapper){false, (ParserInfo){syntaxError, token}};

  // if errors within subroutineBody
  if (info.info.er != none)
    return info;

  return (ParserWrapper){true, (ParserInfo){none, token}};
}

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
