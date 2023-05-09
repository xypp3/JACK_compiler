#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "symbols.h"

typedef enum { false, true } Boolean;

typedef struct {
  unsigned int length;
  TokenType *set; // array
} TokenTypeSet;

// error handling
jmp_buf buf;
ParserInfo errInfo;

// predefined type sets
TokenTypeSet idSet = {1, (TokenType[]){ID}};
TokenTypeSet symbolSet = {1, (TokenType[]){SYMBOL}};
TokenTypeSet reswordSet = {1, (TokenType[]){RESWORD}};
TokenTypeSet typeSet = {2, (TokenType[]){ID, RESWORD}};
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
// id, int, string literal or reswords below
char *factorStart[] = {"-", "~", "(", "true", "false", "null", "this", "\0"};
void operand();
// id, int, string literal or reswords below
char *operandStart[] = {"(", "true", "false", "null", "this", "\0"};

// function stubs above
HashTable *classHashTable = NULL;
HashTable *subroutineHashTable = NULL;
char *redefineMsg = "redeclaration of identifier";
char *undefineMsg = "undeclared identifier";

// code gen data
char *vmComm[17] = {"add",  "sub",     "neg",      "eq",   "gt",    "lt",
                    "and",  "or",      "not",      "push", "pop",   "label",
                    "goto", "if-goto", "function", "call", "return"};
typedef enum {
  ADD,
  SUB,
  NEG,
  EQ,
  GT, // >
  LT, // <
  AND,
  OR,
  NOT, // ~
  PUSH,
  POP,
  LABEL,
  GOTO,
  IFGOTO,
  FUNC,
  CALL,
  RETURN
} vmC;

char *vmMem[9] = {"static",    "argument",  "local", "this",    "that",
                  "pointer 0", "pointer 1", "temp",  "constant"};
typedef enum {
  STATIC_MEM,
  ARG_MEM,
  LOCAL_MEM,
  THIS_MEM,
  THAT_MEM,
  PTR_THIS,
  PTR_THAT,
  TEMP_MEM,
  CONST_MEM
} vmMemEnum;

FILE *vmFptr = NULL;
/**********************************************************************
 **********************************************************************
 **********************************************************************
 ************************* Helper functions ***************************
 **********************************************************************
 **********************************************************************
 **********************************************************************
 */

Boolean strcmpList(char *word, char **acceptCases) {
  // if empty acceptCases return true (for accepting ID tokens)
  if (0 == strcmp(acceptCases[0], "\0"))
    return true;

  int pos = 0;
  // else test acceptCases
  while (strncmp(acceptCases[pos], "\0", 128) != 0) {
    if (0 == strncmp(word, acceptCases[pos], 128))
      return true;

    pos++;
  }

  return false;
}

Boolean isType() {
  Token token = PeekNextToken();
  return ID == token.tp ||
         (RESWORD == token.tp && strcmpList(token.lx, typeStart));
}

Boolean isExpr() {
  Token token = PeekNextToken();
  switch (token.tp) {
  case ID:
  case INT:
  case STRING:
    return true;
  case RESWORD:
  case SYMBOL:
    return strcmpList(token.lx, factorStart);
  default:
    return false;
  }
}

void error(Token token, char *err, SyntaxErrors exitCode) {
  // communicate error
  // printf("error type: %s expected, line: %d,token: %s,\n", err, token.ln,
  // token.lx);

  errInfo.er = exitCode;
  errInfo.tk = token;

  // exit
  longjmp(buf, true);
}

// consumption wrapper
Token eatTerminal(TokenTypeSet typeSet, char **acceptCases,
                  SyntaxErrors potentialErr, char *errMsg) {
  Token token = PeekNextToken();

  // check lexer error
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  // consume terminal token
  for (int i = 0; i < typeSet.length; i++) {
    switch (typeSet.set[i]) {
    case SYMBOL:
    case RESWORD:
      if (typeSet.set[i] == token.tp && strcmpList(token.lx, acceptCases)) {
        return GetNextToken();
      }
      // TODO: FIGURE OUT END OF SWTICH CLASS BEHAVRIOUR
      break;

    case ID:
    case INT:
    case STRING:
      if (typeSet.set[i] == token.tp) {
        return GetNextToken();
      }
      // TODO: FIGURE OUT END OF SWTICH CLASS BEHAVRIOUR
      break;

    // error already processed (unless error hunting in future, hmh)
    case EOFile:
    case ERR:
      break;
    }
  }

  error(token, errMsg, potentialErr);
  return GetNextToken();
}

Boolean eatNonTerminal(void (*nonTerminal)(), int conditional) {
  Token token = PeekNextToken();
  // check lexer error
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  // on conditional
  if (conditional) {
    nonTerminal();
    return true;
  }

  return false;
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
  // class
  eatTerminal(reswordSet, (char *[]){"class", "\0"}, classExpected,
              "'class' resword");

  // ID
  Token classID =
      eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  if (!isCodeGenning()) {
    classHashTable = createHashTable(CLASS_SCOPE, classID.lx);
    if (!isCodeGenning() &&
        !insertHashTable(classID, rootHT(), CLASS, "class", classHashTable))
      error(classID, redefineMsg, redecIdentifier);
  } else {
    HashRow *row = findHashRow(classID.lx, rootHT());
    assert(NULL != row);

    classHashTable = row->deeperTable;
  }

  // '{'
  eatTerminal(symbolSet, (char *[]){"{", "\0"}, openBraceExpected,
              "'{' symbol");

  // { classVarDeclar() | subroutineDeclar() }
  while (true) {

    Token token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(token.lx, classVarDeclarStart) &&
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
  Token token;

  // 'static' | 'field'
  Token statFiel = eatTerminal(reswordSet, (char *[]){"static", "field", "\0"},
                               classVarErr, "'static' or 'field' resword");
  SymbolKind statFielKind =
      (0 == strncmp(statFiel.lx, "static", 128)) ? STATIC : FIELD;

  // type()
  token = PeekNextToken();
  Token typeID = PeekNextToken();
  if (!eatNonTerminal(&type, isType()))
    error(token, "valid type token", illegalType);

  // identifier
  Token id = eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");
  if (!isCodeGenning() &&
      !insertHashTable(id, classHashTable, statFielKind, typeID.lx, NULL))
    error(id, redefineMsg, redecIdentifier);

  // {, identifier}
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(token.lx, (char *[]){",", "\0"})) {
      break;
    }

    // ','
    eatTerminal(symbolSet, (char *[]){",", "\0"}, syntaxError, "',' symbol");

    // identifier
    Token id = eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");
    if (!isCodeGenning() &&
        !insertHashTable(id, classHashTable, statFielKind, typeID.lx, NULL))
      error(id, redefineMsg, redecIdentifier);

    // (to stretch whitespace in formatter)
  }

  // ';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected, ";");
}

void type() {
  Token t = PeekNextToken();
  if (!isCodeGenning() && ID == t.tp) {
    if (NULL == findHashRow(t.lx, rootHT()))
      addUndeclar(t, "");
  }

  eatTerminal(typeSet, typeStart, illegalType, "valid type token");
}

void subroutineDeclar() {
  Token token;

  // 'constructor' | 'function' | 'method'
  Token subToken = eatTerminal(
      reswordSet, (char *[]){"constructor", "function", "method", "\0"},
      subroutineDeclarErr, "'constructor' or 'function' or 'method' resword");
  SymbolKind subKind;
  if (0 == strncmp("constructor", subToken.lx, 11))
    subKind = CONSTRUCTOR;
  else if (0 == strncmp("function", subToken.lx, 8))
    subKind = FUNCTION;
  else
    subKind = METHOD;

  // type() | 'void'
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (isType()) {
    type();
  } else {
    // 'void'
    token = eatTerminal(reswordSet, (char *[]){"void", "\0"}, illegalType,
                        "expected type or 'void' token");
  }

  // identifier
  Token id = eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  // symbol table
  if (!isCodeGenning()) {
    subroutineHashTable = NULL;
    subroutineHashTable = createHashTable(SUBROUTINE_SCOPE, id.lx);
    if (!insertHashTable(id, classHashTable, subKind, token.lx,
                         subroutineHashTable))
      error(id, redefineMsg, redecIdentifier);
    // <<<<<< insert class ref
    // TODO: FIND ONE LINE SETTING
    Token implicitArg;
    implicitArg.tp = ID;
    strncpy(implicitArg.lx, "this", 128);
    implicitArg.ec = NoLexErr;
    implicitArg.ln = id.ln;
    strncpy(implicitArg.fl, id.fl, 32);

    insertHashTable(implicitArg, subroutineHashTable, ARGS,
                    classHashTable->name, NULL);
  } else {
    HashRow *row = findHashRow(id.lx, classHashTable);
    assert(NULL != row);

    subroutineHashTable = row->deeperTable;

    // def function and allocate local vars
    fprintf(vmFptr, "%s %s.%s %d\n", vmComm[FUNC], classHashTable->name,
            subroutineHashTable->name,
            subroutineHashTable->kindCounters[LOCAL_VAR]);

    switch (subKind) {
    case FUNCTION:
      break;
    case CONSTRUCTOR: // allocate field vars and set pointer 0 to this class
      fprintf(vmFptr, "%s %s %d\n", vmComm[PUSH], vmMem[CONST_MEM],
              (classHashTable->kindCounters[FIELD]));
      fprintf(vmFptr, "%s Memory.alloc 1\n", vmComm[CALL]);
      fprintf(vmFptr, "%s %s\n", vmComm[POP], vmMem[PTR_THIS]);
      break;
    case METHOD: // make sure to make this class the pointer 0 class
      fprintf(vmFptr, "%s %s 0\n", vmComm[PUSH],
              vmMem[ARG_MEM]); // implicit 'this' arg
      fprintf(vmFptr, "%s %s\n", vmComm[POP], vmMem[PTR_THIS]);
      break;
    default:
      printf("\n\n\n\n\n\n\n\nERROR SHOULD NOT BE HERE\n\n\n\n\n\n\n\n");
      break;
    }
  }

  // '('
  eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
              "'(' symbol");

  // paramList()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  // if paramList() is NOT empty
  if (!strcmpList(token.lx, (char *[]){")", "\0"})) {
    if (isType()) {
      paramList();
    } else {
      error(token, "paramiter list", closeParenExpected);
    }
  }

  // ')'
  eatTerminal(symbolSet, (char *[]){")", "\0"}, closeParenExpected, ")");

  // subroutineBody()
  token = PeekNextToken();
  if (!eatNonTerminal(
          &subroutineBody,
          (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"{", "\0"}))))
    error(token, "'{' to start a subroutine body", openBraceExpected);

  // reset subrHashtable pointer
  subroutineHashTable = NULL;
}

void paramList() {
  Token typeToken;

  // type()
  typeToken = PeekNextToken(); // for error function
  if (!eatNonTerminal(&type, isType()))
    error(typeToken, "valid type token", illegalType);

  // identifier
  Token idToken =
      eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");
  if (!isCodeGenning() &&
      !insertHashTable(idToken, subroutineHashTable, ARGS, typeToken.lx, NULL))
    error(idToken, redefineMsg, redecIdentifier);

  // {',' type() identifier}
  while (true) {
    typeToken = PeekNextToken();
    if (ERR == typeToken.tp)
      error(typeToken, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(typeToken.lx, (char *[]){",", "\0"})) {
      break;
    }

    // ','
    eatTerminal(symbolSet, (char *[]){",", "\0"}, syntaxError, "',' symbol");

    // type()
    typeToken = PeekNextToken(); // for error function
    if (!eatNonTerminal(&type, isType()))
      error(typeToken, "valid type token", illegalType);

    // identifier
    idToken = eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");
    if (!isCodeGenning() && !insertHashTable(idToken, subroutineHashTable, ARGS,
                                             typeToken.lx, NULL))
      error(idToken, redefineMsg, redecIdentifier);
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

    if (!strcmpList(token.lx, stmtStart))
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
  Token token, varType;

  // 'var'
  eatTerminal(reswordSet, (char *[]){"var", "\0"}, syntaxError,
              "'var' resword expected");

  // type()
  varType = PeekNextToken(); // for error function
  if (!eatNonTerminal(&type, isType()))
    error(varType, "valid type token", illegalType);

  // identifier
  token = PeekNextToken();
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");
  // ACCORDING TO /DECLARED_VAR/A.jack subr var can have the same name as a
  if (!isCodeGenning() && 0 == insertHashTable(token, subroutineHashTable,
                                               LOCAL_VAR, varType.lx, NULL))
    error(token, redefineMsg, redecIdentifier);

  // {, identifier}
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(token.lx, (char *[]){",", "\0"})) {
      break;
    }

    // ','
    eatTerminal(symbolSet, (char *[]){",", "\0"}, syntaxError, "',' symbol");

    // identifier
    token = PeekNextToken();
    eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");
    if (!isCodeGenning() && NULL == findHashRow(token.lx, classHashTable) &&
        0 == insertHashTable(token, subroutineHashTable, LOCAL_VAR, varType.lx,
                             NULL))
      error(token, redefineMsg, redecIdentifier);

    // (to stretch whitespace in formatter)
  }

  // ';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected, ";");
}

// TODO: code gen
void letStmt() {
  Token token;

  // 'let'
  eatTerminal(reswordSet, (char *[]){"let", "\0"}, syntaxError,
              "'let' resword expected");

  // identifier
  token = PeekNextToken();
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");
  if (!isCodeGenning() && NULL == findHashRow(token.lx, classHashTable) &&
      NULL == findHashRow(token.lx, subroutineHashTable))
    addUndeclar(token, classHashTable->name);

  // [ '[' expr() ']' ]
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  // if not equals THEN THERE is a'[' expr() ']'
  if (strcmpList(token.lx, (char *[]){"[", "\0"})) {

    // '['
    eatTerminal(symbolSet, (char *[]){"[", "\0"}, syntaxError, "'[' symbol");

    // expr()
    token = PeekNextToken(); // for error function
    if (!eatNonTerminal(&expr, isExpr()))
      error(token, "a expression", syntaxError);

    // ']'
    eatTerminal(symbolSet, (char *[]){"]", "\0"}, closeBracketExpected,
                "']' symbol");
  }

  // '='
  eatTerminal(symbolSet, (char *[]){"=", "\0"}, equalExpected, "= symbol");

  // expr()
  token = PeekNextToken(); // for error function
  if (!eatNonTerminal(&expr, isExpr()))
    error(token, "a expression", syntaxError);

  // ';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected, ";");
}

// TODO: code gen
void ifStmt() {
  // 'if'
  eatTerminal(reswordSet, (char *[]){"if", "\0"}, syntaxError,
              "'if' resword expected");

  // '('
  eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
              "'(' symbol");

  Token token = PeekNextToken(); // to give it a token to return
  // printf("\n\n\n%s\n\n\n", token.lx);
  // expr()
  if (!eatNonTerminal(&expr, isExpr()))
    error(token, "a expression", syntaxError);

  // printf("\n\n\n%s\n\n\n", token.lx);
  // ')'
  eatTerminal(symbolSet, (char *[]){")", "\0"}, closeParenExpected,
              "')' symbol");
  // printf("\n\n\n%s\n\n\n", token.lx);

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
    if (!strcmpList(token.lx, stmtStart)) {
      break;
    }
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
    // 'else'
    eatTerminal(reswordSet, (char *[]){"else", "\0"}, syntaxError,
                "'else' resword expected");
    // '{'
    eatTerminal(symbolSet, (char *[]){"{", "\0"}, openBraceExpected,
                "'{' symbol");

    // { stmt() }
    while (true) {
      token = PeekNextToken();
      if (ERR == token.tp)
        error(token, "valid lexical token", lexerErr);
      if (!strcmpList(token.lx, stmtStart))
        break;

      stmt();
    }

    // '}'
    eatTerminal(symbolSet, (char *[]){"}", "\0"}, closeBraceExpected,
                "'}' symbol");
  }
}

// TODO: code gen
void whileStmt() {
  Token token;

  // while
  eatTerminal(reswordSet, (char *[]){"while", "\0"}, syntaxError,
              "'while' resword expected");

  // '('
  eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
              "'(' symbol");

  // expr()
  token = PeekNextToken(); // for error function
  if (!eatNonTerminal(&expr, isExpr()))
    error(token, "a expression", syntaxError);

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
    if (!strcmpList(token.lx, stmtStart))
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
  if (!eatNonTerminal(&subroutineCall, ID == token.tp))
    error(token, "valid subroutineCall", idExpected);

  if (isCodeGenning())
    fprintf(vmFptr, "%s %s 0\n", vmComm[POP], vmMem[TEMP_MEM]);

  // ';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected, ";");
}

// TODO: code gen
void subroutineCall() {
  Token token, callID;

  // identifier
  callID = PeekNextToken();
  eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

  // [ '.'identifier ]
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  if (strcmpList(token.lx, (char *[]){".", "\0"})) {
    // '.'
    eatTerminal(symbolSet, (char *[]){".", "\0"}, syntaxError, "'.' symbol");

    // identifier
    token = PeekNextToken();
    eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

    HashRow *row;
    HashRow *class = findHashRow(callID.lx, rootHT());
    // find class of object variable
    if (!isCodeGenning()) {
      // object in class var
      if (NULL != (row = findHashRow(callID.lx, classHashTable)))
        addUndeclar(token, row->type);
      // object in subroutine var
      else if (NULL != subroutineHashTable &&
               NULL != (row = findHashRow(callID.lx, subroutineHashTable)))
        addUndeclar(token, row->type);
      // find class and subroutine in future
      else if (NULL == class) {
        addUndeclar(token, callID.lx);
        addUndeclar(callID, "");
      }
      // find subroutine in future in known class
      else {
        addUndeclar(token, callID.lx);
      }
      // else if (NULL == findHashRow(token.lx, class->deeperTable))
      //   addUndeclar(token, callID.lx);
    }
  } else {
    // find subroutine in THIS class
    if (!isCodeGenning() && NULL == findHashRow(callID.lx, classHashTable))
      addUndeclar(callID, classHashTable->name);
  }

  // '('
  eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
              "'(' symbol");

  // expressionList()
  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  // don't like that that's how I test for an empty exprList() but alas
  if (!strcmpList(token.lx, (char *[]){")", "\0"})) {
    exprList();
  }

  // ')'
  eatTerminal(symbolSet, (char *[]){")", "\0"}, closeParenExpected,
              "')' symbol");
}

void exprList() {
  Token token;

  // expr() or empty
  if (!eatNonTerminal(&expr, isExpr()))
    return;

  // { ',' expr() }
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(token.lx, (char *[]){",", "\0"})) {
      break;
    }

    // ','
    eatTerminal(symbolSet, (char *[]){",", "\0"}, syntaxError, "',' symbol");

    // expr()
    token = PeekNextToken(); // for error function
    if (!eatNonTerminal(&expr, isExpr()))
      error(token, "a expression", syntaxError);

    // (to stretch whitespace in formatter)
  }
}

void returnStmt() {
  // 'return'
  eatTerminal(reswordSet, (char *[]){"return", "\0"}, syntaxError,
              "'return' resword expected");

  // [ expr() ]
  Token token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  if (isExpr())
    expr();
  // garbage value returned on 'void' subroutine
  else if (isCodeGenning())
    fprintf(vmFptr, "%s %s 0\n", vmComm[PUSH], vmMem[CONST_MEM]);

  token = PeekNextToken();

  if (isCodeGenning())
    fprintf(vmFptr, "%s\n", vmComm[RETURN]);

  //';'
  eatTerminal(symbolSet, (char *[]){";", "\0"}, semicolonExpected, ";");
}

void expr() {
  Token token;

  // relationalExpr()
  token = PeekNextToken();
  if (!eatNonTerminal(&relationalExpr, isExpr()))
    error(token, "a relational expression token", syntaxError);

  // { ( '&' | '|' ) relationalExpr() }
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(token.lx, (char *[]){"&", "|", "\0"})) {
      break;
    }

    // get '&' or '|'
    Token conjunc = eatTerminal(symbolSet, (char *[]){"&", "|", "\0"},
                                syntaxError, "'&' or '|' symbol");

    // relationalExpr()
    token = PeekNextToken();
    if (!eatNonTerminal(&relationalExpr, isExpr()))
      error(token, "a relational expression token", syntaxError);

    if (isCodeGenning()) {
      if (!strncmp(conjunc.lx, "&", 2))
        fprintf(vmFptr, "%s\n", vmComm[AND]);
      else
        fprintf(vmFptr, "%s\n", vmComm[OR]);
    }
    // (to stretch whitespace in formatter)
  }
}

void relationalExpr() {
  Token token;

  // arithmeticExpr()
  token = PeekNextToken();
  if (!eatNonTerminal(&arithmeticExpr, isExpr()))
    error(token, "a arithmetic expression token", syntaxError);

  // { ( '=' | '>' | '<' ) arithmeticExpr() }
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(token.lx, (char *[]){"=", ">", "<", "\0"})) {
      break;
    }

    // get '=' or '>' or '<'
    Token conjunc = eatTerminal(symbolSet, (char *[]){"=", ">", "<", "\0"},
                                equalExpected, "'=' or '>' or '<' symbol");

    // arithmeticExpr()
    token = PeekNextToken();
    if (!eatNonTerminal(&arithmeticExpr, isExpr()))
      error(token, "a arithmetic expression token", syntaxError);

    if (isCodeGenning()) {
      if (!strncmp(conjunc.lx, "=", 2))
        fprintf(vmFptr, "%s\n", vmComm[EQ]);
      else if (!strncmp(conjunc.lx, ">", 2))
        fprintf(vmFptr, "%s\n", vmComm[GT]);
      else
        fprintf(vmFptr, "%s\n", vmComm[LT]);
    }
  }
}

void arithmeticExpr() {
  Token token;

  // term()
  token = PeekNextToken();
  if (!eatNonTerminal(&term, isExpr()))
    error(token, "a term expression token", syntaxError);

  // { ( '+' | '-' ) term() }
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(token.lx, (char *[]){"+", "-", "\0"})) {
      break;
    }

    // get '+' or '-'
    Token conjunc = eatTerminal(symbolSet, (char *[]){"+", "-", "\0"},
                                syntaxError, "'+' or '-' symbol");

    // term()
    token = PeekNextToken();
    if (!eatNonTerminal(&term, isExpr()))
      error(token, "a term expression token", syntaxError);

    if (isCodeGenning()) {
      if (!strncmp(conjunc.lx, "+", 2))
        fprintf(vmFptr, "%s\n", vmComm[ADD]);
      else
        fprintf(vmFptr, "%s\n", vmComm[SUB]);
    }
    // (to stretch whitespace in formatter)
  }
}

void term() {
  Token token;

  // factor()
  token = PeekNextToken();
  if (!eatNonTerminal(&factor, isExpr()))
    error(token, "a factor expression token", syntaxError);

  // { ( '*' | '/' ) factor() }
  while (true) {
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);

    // break case
    if (!strcmpList(token.lx, (char *[]){"*", "/", "\0"})) {
      break;
    }

    // get '*' or '/'
    Token conjunc = eatTerminal(symbolSet, (char *[]){"*", "/", "\0"},
                                syntaxError, "'*' or '/' symbol");

    // factor()
    token = PeekNextToken();
    if (!eatNonTerminal(&factor, isExpr()))
      error(token, "a factor expression token", syntaxError);

    if (isCodeGenning()) {
      // when your vm don't got multiply and divide commands ;,,,,(
      if (!strncmp(conjunc.lx, "*", 2))
        fprintf(vmFptr, "call Math.multiply 2\n");
      else
        fprintf(vmFptr, "call Math.divide 2\n");
    }

    // (to stretch whitespace in formatter)
  }
}

void factor() {
  Token token;

  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);
  // check if '-' or '~'
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"-", "~", "\0"})) {

    // consume '-' or '~'
    Token prefix = eatTerminal(symbolSet, (char *[]){"-", "~", "\0"},
                               syntaxError, "'-' or '~' symbol");

    token = PeekNextToken(); // get operand()
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);
    if ((INT == token.tp || ID == token.tp || STRING == token.tp) ||
        (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"(", "\0"})) ||
        (RESWORD == token.tp && strcmpList(token.lx, operandStart))) {
      operand();
    }

    // todo: remove double negative or double not
    if (isCodeGenning()) {
      if (!strncmp(prefix.lx, "-", 2))
        fprintf(vmFptr, "%s\n", vmComm[NEG]);
      else
        fprintf(vmFptr, "%s\n", vmComm[NOT]);
    }

  }
  // starts with empty string i.e. starts with operand()
  else if ((INT == token.tp || ID == token.tp || STRING == token.tp) ||
           (SYMBOL == token.tp &&
            strcmpList(token.lx, (char *[]){"(", "\0"})) ||
           (RESWORD == token.tp && strcmpList(token.lx, operandStart))) {
    operand();
  }

  // todo empty controlflow could lead to issue unless errored
}

void operand() {
  Token token;

  token = PeekNextToken();
  if (ERR == token.tp)
    error(token, "valid lexical token", lexerErr);

  if (INT == token.tp) {
    token = eatTerminal((TokenTypeSet){1, (TokenType[]){INT}}, (char *[]){"\0"},
                        syntaxError, "int literal");

    if (isCodeGenning())
      fprintf(vmFptr, "%s %s %s\n", vmComm[PUSH], vmMem[CONST_MEM], token.lx);

    return;
  }

  if (STRING == token.tp) {
    token = eatTerminal((TokenTypeSet){1, (TokenType[]){STRING}},
                        (char *[]){"\0"}, syntaxError, "string literal");
    if (isCodeGenning()) {
      fprintf(vmFptr, "%s %s %d\n", vmComm[PUSH], vmMem[CONST_MEM],
              (int)strlen(token.lx));
      fprintf(vmFptr, "call String.new 1\n");
      for (int i = 0; i < strlen(token.lx); i++) {
        fprintf(vmFptr, "%s %s %d\n", vmComm[PUSH], vmMem[CONST_MEM],
                token.lx[i]);
        fprintf(vmFptr, "call String.appendChar 2\n");
      }
    }

    return;
  }

  // todo: code gen this baheamoth
  // identifier [ '.'identifier ] [ '['expr()']' | '('exprList')']
  if (ID == token.tp) {
    // identifier
    Token callID = PeekNextToken();
    eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

    // [ '.'identifier ]
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);
    if (strcmpList(token.lx, (char *[]){".", "\0"})) {
      // '.'
      eatTerminal(symbolSet, (char *[]){".", "\0"}, syntaxError, "'.' symbol");

      // identifier
      token = PeekNextToken();
      eatTerminal(idSet, (char *[]){"\0"}, idExpected, "identifier");

      // HASH TABLE INSERTION/ CHECKING
      HashRow *row;
      HashRow *class = findHashRow(callID.lx, rootHT());
      // find class of object variable
      if (!isCodeGenning()) {
        // object in class var
        if (NULL != (row = findHashRow(callID.lx, classHashTable)))
          addUndeclar(token, row->type);
        // object in subroutine var
        else if (NULL != subroutineHashTable &&
                 NULL != (row = findHashRow(callID.lx, subroutineHashTable)))
          addUndeclar(token, row->type);
        // find class and subroutine in future
        else if (NULL == class) {
          addUndeclar(token, callID.lx);
          addUndeclar(callID, "");
        }
        // find subroutine in future in known class
        else {
          addUndeclar(token, callID.lx);
        }
        // else if (NULL == findHashRow(token.lx, class->deeperTable))
        //   addUndeclar(token, callID.lx);
      }
    } else {
      // HASH TABLE INSERTION/ CHECKING
      // find VAR in THIS class or VAR in subroutinHashTable
      HashTable *t;
      if (!isCodeGenning() &&
          NULL == findHashRow(callID.lx, (t = classHashTable)) &&
          (NULL == subroutineHashTable ||
           NULL == findHashRow(callID.lx, (t = subroutineHashTable))))
        addUndeclar(callID, t->name);
    }

    // [ '['expr()']' | '('exprList')' ]
    token = PeekNextToken();

    // '['expr()']'
    if (strcmpList(token.lx, (char *[]){"[", "\0"})) {
      // '['
      eatTerminal(symbolSet, (char *[]){"[", "\0"}, syntaxError,
                  "'[' symbol at end of expression");

      // expr()
      token = PeekNextToken();
      // printf("\n\n%s\n\n", token.lx);
      if (ERR == token.tp)
        error(token, "valid lexical token", lexerErr);
      if (isExpr()) {
        expr();
      } else {
        error(token, "a expression", syntaxError);
      }
      token = PeekNextToken();
      // ']'
      eatTerminal(symbolSet, (char *[]){"]", "\0"}, closeBracketExpected, "]");

      return;
    }

    // '(' exprList() ')'
    if (strcmpList(token.lx, (char *[]){"(", "\0"})) {
      // '('
      eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
                  "'(' symbol at end of expression");

      // exprList()
      token = PeekNextToken();
      if (ERR == token.tp)
        error(token, "valid lexical token", lexerErr);

      // don't like that that's how I test for an empty exprList() but alas
      if (!strcmpList(token.lx, (char *[]){")", "\0"})) {
        exprList();
      }

      // ')'
      eatTerminal(symbolSet, (char *[]){")", "\0"}, closeBracketExpected,
                  "')' symbol at end of expression ");

      return;
    }

    return;
  }

  // '('expr()')'
  if (SYMBOL == token.tp && strcmpList(token.lx, (char *[]){"(", "\0"})) {
    // '('
    eatTerminal(symbolSet, (char *[]){"(", "\0"}, openParenExpected,
                "'(' symbol");

    // expr()
    token = PeekNextToken();
    if (ERR == token.tp)
      error(token, "valid lexical token", lexerErr);
    if (isExpr()) {
      expr();
    } else {
      error(token, "a expression", syntaxError);
    }

    // ')'
    eatTerminal(symbolSet, (char *[]){")", "\0"}, closeParenExpected,
                "')' symbol at end of expression");

    return;
  }

  //  'true' | 'false' | 'null' | 'this'
  char *operandConst[] = {"true", "false", "null", "this", "\0"};
  Token consts =
      eatTerminal(reswordSet, operandConst, syntaxError, "operand value");

  if (isCodeGenning()) {
    if (!strncmp(consts.lx, "true", 5)) {
      fprintf(vmFptr, "%s %s 1\n", vmComm[PUSH], vmMem[CONST_MEM]);
      fprintf(vmFptr, "%s\n", vmComm[NEG]);
    } else if (!strncmp(consts.lx, "this", 5)) {
      // todo: code gen
    }
    // false of null
    else {
      fprintf(vmFptr, "%s %s 0\n", vmComm[PUSH], vmMem[CONST_MEM]);
    }
  }
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

  if (isCodeGenning())
    if (NULL == (vmFptr = fopen(getCodeGenFile(), "w")))
      return 0;

  return InitLexer(file_name);
}

ParserInfo Parse() {
  ParserInfo pi;

  // pseudo-async
  if (setjmp(buf)) {
    return errInfo;
  }
  // run first then
  classDeclar();

  pi.er = none;
  return pi;
}

int StopParser() {
  // reset ptr but keep hashtable
  classHashTable = NULL;
  subroutineHashTable = NULL;

  if (NULL != vmFptr) {
    fclose(vmFptr);
    vmFptr = NULL;
  }

  return StopLexer();
}

#ifndef TEST_PARSER
// int main(int argc, char **argv) {
//   if (argc != 2) {
//     printf("usage: ./parser filename.jack");
//     return 1;
//   }
//
//   InitParser(argv[1]);
//   Parse();
//   StopParser();
//
//   return 1;
// }
//
#endif
