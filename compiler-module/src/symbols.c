#include "symbols.h"
#include "stdio.h"
#include "stdlib.h"
#include <sys/types.h>

#define MAX_SCOPE_TREE_DEPTH 256
#define HASHTABLE_SIZE 100000

typedef struct HashRow {
  char lexem[128];
  // TODO: info
  struct HashRow *next;
  unsigned short scopeID;
  unsigned short scopeParentID;
} HashRow;

unsigned short scopeTreeSize =
    MAX_SCOPE_TREE_DEPTH + 1; // arbitraryly limit scope num

HashRow **hashTable = NULL;

Boolean initHashTable() {
  if (NULL != hashTable)
    return false;

  hashTable = (HashRow **)malloc(sizeof(HashRow *) * HASHTABLE_SIZE);

  for (unsigned int i = 0; i < HASHTABLE_SIZE; i++)
    hashTable[i] = NULL;

  return true;
}

Boolean InitSymbol() {
  // already init exit
  if (MAX_SCOPE_TREE_DEPTH + 1 == scopeTreeSize)
    return false;

  scopeTreeSize = 0;
  initHashTable();

  return true;
}

int main(int argc, char **argv) {

  InitSymbol();

  return 0;
}
