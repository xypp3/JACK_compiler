#ifndef COMPILER_H
#define COMPILER_H

#define TEST_COMPILER // uncomment to run the compiler autograder

#include "parser.h"

int InitCompiler();
ParserInfo compile(char *dir_name);
int StopCompiler();

int isCodeGenning();
char *getCodeGenFile();

#endif
