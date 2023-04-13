#include "symbols.h"

#include "assert.h"
#include "stdio.h"
#include "stdlib.h"

#define HASHTABLE_SIZE 10000

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
  // TODO: info

  HashTable *deeperTable;
  struct HashRow *next;
} HashRow;

typedef struct HashTable_ {
  HashRow **row;
  ScopeLevels tableScope;
} HashTable;

HashTable *rootHashTable = NULL;

HashTable *createHashTable(ScopeLevels scope) {
  HashTable *hashTable;
  hashTable = (HashTable *)malloc(sizeof(HashTable *));
  hashTable->tableScope = scope;

  hashTable->row = (HashRow **)malloc(sizeof(HashRow *) * HASHTABLE_SIZE);
  for (unsigned int i = 0; i < HASHTABLE_SIZE; i++)
    hashTable->row[i] = NULL;

  return hashTable;
}

void InitSymbol() {
  // already init exit
  assert(rootHashTable == NULL);

  rootHashTable = createHashTable(PROGRAM_SCOPE);
  printf("%d", rootHashTable->tableScope);
}

void freeHashTable(HashTable *hashTable) {
  if (hashTable == NULL)
    return;

  for (int i = 0; i < HASHTABLE_SIZE; i++) {
    if (hashTable->row[i] == NULL)
      continue;
    // free hash miss linked list
    if (hashTable->row[i]->next != NULL)
      free(hashTable->row[i]->next);
    // free table further down in scope tree
    if (hashTable->row[i]->deeperTable != NULL)
      freeHashTable(hashTable->row[i]->deeperTable);
  }
  free(hashTable->row);
  free(hashTable);
}

void CloseSymbol() {
  if (rootHashTable == NULL)
    return;

  freeHashTable(rootHashTable);
  rootHashTable = NULL; // DANGLING POINTER
}
#ifndef TEST_SYMBOL
int main(int argc, char **argv) {

  InitSymbol();
  CloseSymbol();

  return 0;
}
#endif
