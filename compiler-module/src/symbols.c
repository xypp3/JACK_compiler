#include "symbols.h"

#include "assert.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define HASHTABLE_SIZE 1000

HashTable *rootHashTable = NULL;
// TODO: have a way of checking for valid type and if it's not found search
// enire program tree at end to find
typedef struct {
  Token token;
  char className[128];
} LostKids;

LostKids *undeclarList = NULL;
size_t undeclarListIter;
size_t undeclarListSize;

HashTable *createHashTable(ScopeLevels scope, char *name) {
  HashTable *hashTable;
  hashTable = (HashTable *)malloc(sizeof(HashTable));
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

  undeclarListSize = 64;
  undeclarList = (LostKids *)malloc(sizeof(LostKids) * undeclarListSize);
  for (int i = 0; i < 128; i++) {
    strncpy(undeclarList[i].className, "", 128);
  }

  undeclarListIter = 0;
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

// used to be (Token token) instead of (char *lexem)
//    is there a reason for it to be otherwise?
HashRow *findHashRow(char *lexem, HashTable *table) {
  HashRow *row = table->allRows[hash(lexem)];
  while (row != NULL) {
    if (0 == strncmp(row->token.lx, lexem, 128))
      return row;

    row = row->next;
  }

  return NULL;
}

HashRow *findRowOrAddUndeclar(Token token, HashTable *table, char *className) {
  HashRow *row = findHashRow(token.lx, table);
  if (NULL != row)
    return row;

  addUndeclar(token, className);

  return NULL;
}

void addUndeclar(Token token, char *className) {
  // check if already in undeclarList
  for (int i = 0; i < undeclarListIter; i++) {
    if ((0 == strncmp(undeclarList[i].token.lx, token.lx, 128)) &&
        ((0 != strncmp(className, "", 128)) ||
         (0 == strncmp(undeclarList[i].className, className, 128))))
      return;
  }

  // check list size
  if (undeclarListSize <= undeclarListIter) {
    undeclarListSize *= 4;
    void *err = realloc(undeclarList, undeclarListSize);
    if (NULL == err)
      printf("realloc err");
    exit(404);
  }

  // add to undeclarList
  undeclarList[undeclarListIter].token = token;
  strncpy(undeclarList[undeclarListIter].className, className, 128);
  undeclarListIter++;
}

Token findLostKids() {
  HashRow *class;
  for (; 0 <= undeclarListIter; undeclarListIter--) {
    LostKids curr = undeclarList[undeclarListIter];
    if (0 == strncmp(curr.className, "", 128)) {
      // class
      if (NULL == findHashRow(curr.token.lx, rootHashTable))
        return curr.token;

    } else {
      // class subroutine
      if (NULL == (class = findHashRow(curr.className, rootHashTable)) ||
          NULL == findHashRow(curr.token.lx, class->deeperTable))
        /* TODO: might lead to subr err showing up before class err
         *    cause this can exit early in the undeclarList array
         */
        return curr.token;
    }
  }

  Token t;
  t.ec = none;
  return t;
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
  free(undeclarList);

  rootHashTable = NULL;
  undeclarList = NULL;
}

#ifndef TEST_SYMBOL

void printTable(HashTable *table) {
  printf("HASHTABLE <%p> BEGINNING\n", table);

  for (int i = 0; i < HASHTABLE_SIZE; i++) {
    HashRow *row = table->allRows[i];
    while (NULL != row) {
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
  Token t;
  strncpy(t.lx, "hi", 128);

  insertHashTable(t, rootHashTable, CLASS, "class", NULL);
  HashTable *class;
  class = createHashTable(CLASS_SCOPE, "main");
  strncpy(t.lx, "main", 128);
  insertHashTable(t, rootHashTable, CLASS, "class", class);
  printTable(rootHashTable);

  StopSymbol();
  InitSymbol();

  insertHashTable(t, rootHashTable, CLASS, "class", NULL);
  printf("%s done\n", rootHashTable->allRows[hash("hi")]->token.lx);

  StopSymbol();

  return 0;
}

#endif
