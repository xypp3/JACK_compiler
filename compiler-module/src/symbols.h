#ifndef SYMBOLS_H
#define SYMBOLS_H
#endif

#include "lexer.h"
#include "parser.h"

typedef enum { PROGRAM_SCOPE, CLASS_SCOPE, SUBROUTINE_SCOPE } ScopeLevels;

typedef enum {
  CLASS,
  STATIC,
  FIELD,
  CONSTRUCTOR,
  FUNCTION,
  METHOD,
  ARGS,
  LOCAL_VAR,
  // CONSTANT, // e.g. int (4) or string ("hello world")
  // do i include constants like int or string const?
} SymbolKind;

typedef enum {
  INT_TYPE,
  CHAR_TYPE,
  BOOLEAN_TYPE,
  STRING_TYPE,
  VOID_TYPE,
  CLASS_TYPE, // TODO: find a way to validate if class init
} SymbolTypes;
