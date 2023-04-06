# jack-compiler

> This monorepo is my incremental implementation of a compiler for the JACK programming language.
> There are **4 parts** to my compiler but I worked on it in **3 overarching modules**. The lexer-module
> corresponds to working on the lexer part, and similarly with the parser-module. The compiler-module
> contains the overarching compiler program but also a symbol table utility.

## How to run

You run each module independantly and each module has it's own makefile. If you want to run the full
compiler run the compiler-module.

### lexer-module

- **MAKE** `make lexer`
- **COMMAND** `./lexer filename`
- **OUTPUT** a file where there every lexeme extracted on a new line with additional metadata

### parser-module

- **MAKE** `make parser`
- **COMMAND** `./parser filename`
- **OUTPUT** returns nothing on encountering no syntactic or semantic errors. On error it returns an
  appropriate error msg

### compiler-module

- TODO

## Key technical features

### lexer-module

### parser-module

### compiler-module
