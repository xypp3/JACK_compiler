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
#include <dirent.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "compiler.h"
#include "symbols.h"

#define FILE_PATH_LEN 512

int findLastIndexOf(char *str, char c) {
  int foundIndex = -1;
  int iterIndex = 0;

  while (str != NULL && str[iterIndex] != '\0') {
    if (str[iterIndex] == c)
      foundIndex = iterIndex;

    iterIndex++;
  }

  return foundIndex;
}

// CODE GEN CODE
int codeGenning = 0;
char codeGenFile[FILE_PATH_LEN] = {0};
int isCodeGenning() { return codeGenning; }
void setCodeGenFile(char *name) {
  memset(codeGenFile, 0, FILE_PATH_LEN);
  strncpy(codeGenFile, name, findLastIndexOf(name, '.') + 1);
  strncat(codeGenFile, "vm", 2);
}
char *getCodeGenFile() { return codeGenFile; }

char parseFile[FILE_PATH_LEN];

int InitCompiler() {
  InitSymbol();
  return 1;
}

ParserInfo compile(char *dir_name) {
  ParserInfo p;
  p.er = none;

  //
  // char std_dir_name[400] = {0};
  // strncpy(std_dir_name, dir_name, findLastIndexOf(dir_name, '/'));
  //
  // // open std dir
  // if (NULL == (dir = opendir(std_dir_name))) {
  //   printf("directory %s, does not exist", std_dir_name);
  //   p.er = syntaxError;
  //   return p;
  // }
  //
  // // reading standard libraries
  // while (NULL != (file = readdir(dir))) {
  //   if (NULL == strstr(file->d_name, ".jack"))
  //     continue;
  //
  //   snprintf(parseFile, FILE_PATH_LEN, "%s/%s", std_dir_name, file->d_name);
  //
  //   if (0 == InitParser(parseFile)) {
  //     p.er = lexerErr;
  //     return p;
  //   }
  //
  //   p = Parse();
  //   StopParser();
  //
  //   if (p.er != none)
  //     return p;
  // }
  // closedir(dir);
  glob_t globbuf;

  if (glob("*.jack", 0, NULL, &globbuf) != 0) {
    perror("glob");
    return p;
  }

  // parse core lib
  for (int i = 0; i < globbuf.gl_pathc; i++) {
    snprintf(parseFile, FILE_PATH_LEN, "%s", globbuf.gl_pathv[i]);

    if (0 == InitParser(parseFile)) {
      p.er = lexerErr;
      return p;
    }

    p = Parse();
    StopParser();

    if (none != p.er)
      return p;
  }

  struct dirent *file;
  DIR *dir;
  // open project dir
  if (NULL == (dir = opendir(dir_name))) {
    printf("directory %s, does not exist", dir_name);
    p.er = syntaxError;
    return p;
  }
  // initial parsing
  while (NULL != (file = readdir(dir))) {
    if (NULL == strstr(file->d_name, ".jack"))
      continue;

    snprintf(parseFile, FILE_PATH_LEN, "%s/%s", dir_name, file->d_name);

    if (0 == InitParser(parseFile)) {
      p.er = lexerErr;
      return p;
    }

    p = Parse();
    StopParser();

    if (none != p.er)
      return p;
  }

  // final findLostKids
  if (none != (p = findLostKids()).er) {
    return p;
  }

  // code generation parse
  codeGenning = 1;
  rewinddir(dir);
  while (NULL != (file = readdir(dir))) {
    if (NULL == strstr(file->d_name, ".jack"))
      continue;

    snprintf(parseFile, FILE_PATH_LEN, "%s/%s", dir_name, file->d_name);
    setCodeGenFile(parseFile);

    if (0 == InitParser(parseFile)) {
      p.er = lexerErr;
      return p;
    }

    p = Parse();
    StopParser();

    if (none != p.er)
      return p;
  }
  codeGenning = 0;
  closedir(dir);

  return p;
}

int StopCompiler() {
  StopSymbol();
  return 1;
}

#ifndef TEST_COMPILER
// int main() {
//   InitCompiler();
//   compile("/home/xypp3/jack_compiler/compiler-module/data/Pong");
//   StopCompiler();
//   return 1;
// }
#endif
