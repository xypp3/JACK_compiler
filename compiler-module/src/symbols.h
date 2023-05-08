#ifndef SYMBOLS_H
#define SYMBOLS_H
#endif

#define TEST_COMPILER // uncomment to run the compiler autograder

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

  /* DO NOT SET !!!!
      - hacky way of finding number of enumerated vals
      - ALWAYS KEEP AT BOTTOM OF ENUM
  */
  ENUM_SIZE
} SymbolKind;

typedef struct HashRow_ HashRow;
typedef struct HashTable_ HashTable;

typedef struct HashRow_ {
  Token token;
  // SymbolTypes t;
  char type[128];
  SymbolKind k;
  // TODO: info (e.g. value of item)
  unsigned int vmStackNum;
  HashTable *deeperTable;
  struct HashRow_ *next;
  struct HashRow_ *previous;
} HashRow;

typedef struct HashTable_ {
  // TODO: find out how to dynamic *allRows[]
  HashRow **allRows; // 1D array of pointers

  ScopeLevels tableScope;
  char name[128];

  // array of SymbolKind counters
  // accessed kindCounters[SymbolKind k] e.g. kindCounters[ARGS]
  unsigned int kindCounters[ENUM_SIZE];
} HashTable;

HashTable *rootHT();
void InitSymbol();
HashTable *createHashTable(ScopeLevels scope, char *name);
int insertHashTable(Token token, HashTable *table, SymbolKind kind, char *type,
                    HashTable *deeper);
HashRow *findHashRow(char *lexem, HashTable *table);
void addUndeclar(Token token, char *className);
ParserInfo findLostKids();
void freeHashTable(HashTable *hashTable);
void StopSymbol();
