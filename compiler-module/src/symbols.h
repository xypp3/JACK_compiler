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

typedef struct HashRow_ HashRow;
typedef struct HashTable_ HashTable;

typedef struct HashRow_ {
  char lexem[128];
  SymbolTypes t;
  SymbolKind k;
  // TODO: info (e.g. value of item)
  HashTable *deeperTable;
  struct HashRow_ *next;
  struct HashRow_ *previous;
} HashRow;

typedef struct HashTable_ {
  // TODO: find out how to dynamic *allRows[]
  HashRow **allRows; // 1D array of pointers
  ScopeLevels tableScope;
} HashTable;

void InitSymbol();
HashTable *createHashTable(ScopeLevels scope);
void insertHashTable(char *lexem, HashTable *table, SymbolKind kind,
                     SymbolTypes type, HashTable *deeper);
HashRow *findHashRow(char *lexem, HashTable *table);
void StopSymbol();
