#include "symbols.h"

#include "assert.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define HASHTABLE_SIZE 10000

typedef struct HashRow_ HashRow;
typedef struct HashTable_ HashTable;

typedef struct HashRow_ {
  char lexem[128];
  SymbolTypes t;
  SymbolKind k;
  // TODO: info

  HashTable *deeperTable;
  struct HashRow_ *next;
  struct HashRow_ *previous;
} HashRow;

typedef struct HashTable_ {
  HashRow **allRows; // 1D array of pointers
  ScopeLevels tableScope;
} HashTable;

HashTable *rootHashTable = NULL;

HashTable *createHashTable(ScopeLevels scope) {
  HashTable *hashTable;
  hashTable = (HashTable *)malloc(sizeof(HashTable *));
  hashTable->tableScope = scope;

  hashTable->allRows = (HashRow **)malloc(sizeof(HashRow *) * HASHTABLE_SIZE);
  for (unsigned int i = 0; i < HASHTABLE_SIZE; i++)
    hashTable->allRows[i] = NULL;

  return hashTable;
}

void InitSymbol() {
  // already init exit
  assert(rootHashTable == NULL);
  rootHashTable = createHashTable(PROGRAM_SCOPE);
}

unsigned int hash(char *lexem) { return 0; }

void insertHashTable(char *lexem, HashTable *table, SymbolKind kind,
                     SymbolTypes type, HashTable *deeper) {
  assert(table != NULL);

  unsigned int index = hash(lexem);
  HashRow *row;
  row = table->allRows[index];

  // insert new
  if (row == NULL) {
    row = (HashRow *)malloc(sizeof(HashRow));
    table->allRows[index] = row; // set original to new malloced ptr

    strncpy(row->lexem, lexem, 128);
    row->k = kind;
    row->t = type;
    row->deeperTable = deeper;
    row->next = NULL;
    row->previous = NULL;

    return;
  }
  // insert hash miss
  while (row->next != NULL && 0 != strncmp(row->lexem, lexem, 128))
    row = row->next;

  // lexem found in middle of as last element of hash miss list
  if (row->next != NULL || 0 == strncmp(row->lexem, lexem, 128))
    return;

  // link up new item on end of list
  HashRow *newItem = (HashRow *)malloc(sizeof(HashRow));
  row->next = newItem;
  newItem->previous = row;

  // fill in new item details
  row = row->next;
  strncpy(row->lexem, lexem, 128);
  row->k = kind;
  row->t = type;
  row->deeperTable = deeper;
  row->next = NULL;
}

void freeHashTable(HashTable *hashTable) {
  assert(hashTable != NULL);

  for (int i = 0; i < HASHTABLE_SIZE; i++) {
    HashRow *row = hashTable->allRows[i];
    if (row == NULL)
      continue;
    // free hash miss linked list
    //    frees last row in list in filnal free row at bottom
    while (row->next != NULL) {
      row = row->next;
      free(row->previous);
    }

    // free table further down in scope tree
    if (row->deeperTable != NULL)
      freeHashTable(row->deeperTable);

    // free row
    free(row);
  }
  free(hashTable->allRows);

  free(hashTable);
}

void CloseSymbol() {
  assert(rootHashTable != NULL);
  freeHashTable(rootHashTable);
  rootHashTable = NULL; // DANGLING POINTER
}

#ifndef TEST_SYMBOL
void printTable(HashTable *table) {
  printf("HASHTABLE <%p> BEGINNING\n", table);

  for (int i = 0; i < HASHTABLE_SIZE; i++) {
    HashRow *row = table->allRows[i];
    while (row != NULL) {
      printf("%d::<%s>::scope<%d>::kind<%d>::type<%d>::deeper<%p>\n", i,
             row->lexem, table->tableScope, row->k, row->t, row->deeperTable);
      row = row->next;
    }
  }

  printf("HASHTABLE END\n");
}
int main(int argc, char **argv) {

  // reset test
  InitSymbol();

  insertHashTable("hi", rootHashTable, CLASS, CLASS_TYPE, NULL);
  HashTable *class;
  class = createHashTable(CLASS_SCOPE);
  insertHashTable("main", rootHashTable, CLASS, CLASS_TYPE, class);
  printTable(rootHashTable);

  CloseSymbol();
  InitSymbol();

  insertHashTable("hi", rootHashTable, CLASS, CLASS_TYPE, NULL);
  printf("%s done\n", rootHashTable->allRows[hash("hi")]->lexem);

  CloseSymbol();

  return 0;
}
#endif
