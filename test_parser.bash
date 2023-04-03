#!/bin/bash

make parser
cp lexer.c parser.c parser_autograder/.
cd parser_autograder/
./run_autograder
