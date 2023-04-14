#include "symbols.h"

#include "assert.h"
#include "stdlib.h"
#include "string.h"

#define HASHTABLE_SIZE 10000

HashTable *rootHashTable = NULL;
// TODO: have a way of checking for valid type and if it's not found search
// enire program tree at end to find

HashTable *createHashTable(ScopeLevels scope, char name[128]) {
  HashTable *hashTable;
  hashTable = (HashTable *)malloc(sizeof(HashTable *));
  hashTable->tableScope = scope;
  strncpy(hashTable->name, name, 128);

  for (int i = 0; i < ENUM_SIZE; i++)
    hashTable->kindCounters[i] = 0;

  hashTable->allRows = (HashRow **)malloc(sizeof(HashRow *) * HASHTABLE_SIZE);
  for (int i = 0; i < HASHTABLE_SIZE; i++)
    hashTable->allRows[i] = NULL;

  return hashTable;
}

void InitSymbol() {
  // already init exit
  assert(rootHashTable == NULL);
  rootHashTable = createHashTable(PROGRAM_SCOPE, "program");
}

unsigned int hash(char *lexem) {
  unsigned int length = strlen(lexem);
  unsigned int hash = 0;
  for (int i = 0; i < length; i++)
    hash += lexem[i] * 37;
  return hash % HASHTABLE_SIZE;
}

int insertHashTable(Token token, HashTable *table, SymbolKind kind, char *type,
                    HashTable *deeper) {

  if (table == NULL)
    table = rootHashTable;

  unsigned int index = hash(token.lx);
  HashRow *row = table->allRows[index];

  // traverse Linked List
  HashRow *previous = NULL;
  while (row != NULL) {
    if (0 == strncmp(row->token.lx, token.lx, 128))
      return 0; // false

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

  row->token = token;
  row->k = kind;
  row->type = type;
  row->deeperTable = deeper;
  row->next = NULL;
  row->vmStackNum = table->kindCounters[kind];
  table->kindCounters[kind]++;

  return 1; // true
}

HashRow *findHashRow(char *lexem, HashTable *table) {
  HashRow *row = table->allRows[hash(lexem)];
  while (row != NULL) {
    if (0 == strncmp(row->token.lx, lexem, 128))
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

void StopSymbol() {
  assert(rootHashTable != NULL);
  freeHashTable(rootHashTable);
  rootHashTable = NULL; // PREVENT DANGLING POINTER
}

#ifndef TEST_SYMBOL
#include "stdio.h"

void printTable(HashTable *table) {
  printf("HASHTABLE <%p> BEGINNING\n", table);

  for (int i = 0; i < HASHTABLE_SIZE; i++) {
    HashRow *row = table->allRows[i];
    while (row != NULL) {
      printf("%d::<%s>::scope<%d>::kind<%d>::type<%s>::deeper<%p>\n", i,
             row->token.lx, table->tableScope, row->k, row->type,
             row->deeperTable);
      row = row->next;
    }
  }

  printf("HASHTABLE END\n");
}

int main(int argc, char **argv) {

  // reset test
  InitSymbol();

  insertHashTable("hi", rootHashTable, CLASS, "class", NULL);
  HashTable *class;
  class = createHashTable(CLASS_SCOPE, "main");
  insertHashTable("main", rootHashTable, CLASS, "class", class);
  printTable(rootHashTable);

  StopSymbol();
  InitSymbol();

  insertHashTable("hi", rootHashTable, CLASS, "class", NULL);
  printf("%s done\n", rootHashTable->allRows[hash("hi")]->token.lx);

  StopSymbol();

  return 0;
}

#endif
