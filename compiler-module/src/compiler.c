/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Compiler Module

I confirm that the following code has been developed and written by me and it is
entirely the result of my own work. I also confirm that I have not copied any
parts of this program from another person or any other source or facilitated
someone to copy this program from me. I confirm that I will not publish the
program online or share it with anyone without permission of the module leader.

Student Name:
Student ID:
Email:
Date Work Commenced:
*************************************************************************/

#include "compiler.h"
#include "dirent.h"
#include "stdio.h"
#include "string.h"

int InitCompiler() {
  InitSymbol();
  return 1;
}

ParserInfo compile(char *dir_name) {
  ParserInfo p;
  p.er = none;

  struct dirent *file;
  DIR *dir;

  if (NULL == (dir = opendir(dir_name))) {
    printf("directory %s, does not exist", dir_name);
    p.er = syntaxError;
    return p;
  }

  // initial parsing
  while (NULL != (file = readdir(dir))) {
    if (p.er != none)
      break;
    if (NULL == strstr(file->d_name, ".jack"))
      continue;

    char parseFile[512];
    sprintf(parseFile, "%s/%s", dir_name, file->d_name);

    if (0 == InitParser(parseFile)) {
      p.er = lexerErr;
      return p;
    }

    p = Parse();
    StopParser();
  }

  // STAGE
  if (none != (p.tk = findLostKids()).ec) {
    p.er = undecIdentifier;
    return p;
  }

  // code generation parse
  // TODO

  return p;
}

int StopCompiler() {
  StopSymbol();
  return 1;
}

#ifndef TEST_COMPILER
int main() {
  InitCompiler();
  compile("/home/xypp3/jack_compiler/compiler-module/data/Pong");
  StopCompiler();
  return 1;
}
#endif
