#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

void compile(VM* vm, const char* source) {
  Scanner scanner;
  initScanner(&scanner, source);

  // demo code
  int line = -1;
  for (;;) {
    Token token = scanToken(&scanner);
    
    if (token.line != line) {
      // print line number
      printf("%4d ", token.line);
      line = token.line;
    } else {
      // print continuation
      printf("   | ");
    }

    // show token type and token start
    printf("%2d '%.*s'\n", token.type, token.length, token.start);

    // if token is EOF exit this loop
    if (token.type == TOKEN_EOF) break;
  }
}