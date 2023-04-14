#include "symbols.h"

#include "assert.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define HASHTABLE_SIZE 10000

typedef enum { false, true } Boolean;

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

unsigned int hash(char *lexem) {
  unsigned int length = strlen(lexem);
  unsigned int hash = 0;
  for (int i = 0; i < length; i++)
    hash += lexem[i] * 37;
  return hash % HASHTABLE_SIZE;
}

void insertHashTable(char *lexem, HashTable *table, SymbolKind kind,
                     SymbolTypes type, HashTable *deeper) {

  assert(table != NULL);

  unsigned int index = hash(lexem);
  HashRow *row = table->allRows[index];

  // traverse Linked List
  HashRow *previous = NULL;
  while (row != NULL) {
    if (0 == strncmp(row->lexem, lexem, 128))
      return;

    previous = row;
    row = row->next;
  }

  row = (HashRow *)malloc(sizeof(HashRow));

  // link up to new Linked List or to end of previous Linked List
  if (previous != NULL) {
    // hash miss case
    previous->next = row;
    row->previous = previous;
  } else {
    // new hash case
    table->allRows[index] = row; // set original to new malloced ptr
  }

  strncpy(row->lexem, lexem, 128);
  row->k = kind;
  row->t = type;
  row->deeperTable = deeper;
  row->next = NULL;
}

HashRow *findHashRow(char *lexem, HashTable *table) {
  HashRow *row = table->allRows[hash(lexem)];
  while (row != NULL) {
    if (0 == strncmp(row->lexem, lexem, 128))
      return row;

    row = row->next;
  }

  return NULL;
}

// recursive free table to free entire HashTable graph
void freeHashTable(HashTable *hashTable) {
  // base case
  if (hashTable == NULL)
    return;

  for (int i = 0; i < HASHTABLE_SIZE; i++) {
    HashRow *row = hashTable->allRows[i];
    if (row == NULL)
      continue;
    // free hash miss linked list
    //    frees last row in list in filnal free row at bottom
    while (row->next != NULL) {
      if (row->deeperTable != NULL)
        freeHashTable(row->deeperTable);

      row = row->next;
      // free table further down in scope tree
      free(row->previous);
    }

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
