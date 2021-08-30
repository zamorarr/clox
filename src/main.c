#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl(VM* vm) {
  char line[1024];
  for (;;) {
    printf("> ");

    // read line from stdin
    if (!fgets(line, sizeof(line), stdin)) {
      // nothing in stdin, break
      printf("\n");
      break;
    }

    // otherwise interpret line
    interpret(vm, line);
  }
}

static char* readFile(const char* path) {
  // open file
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }

  // seek to end of file to get file size
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);

  // rewind back to beginning of file
  rewind(file);

  // allocate buffer to hold file (+1 for NULL byte)
  char* buffer = (char*) malloc(fileSize + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    exit(74);
  }

  // read file into buffer
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  // add NULL byte
  buffer[bytesRead] = '\0';

  // close file
  fclose(file);

  // return buffer (don't forget free it outside this function!)
  return buffer;
}

static void runFile(VM* vm, const char* path) {
  // read source from file
  char* source = readFile(path);

  // interpret result
  InterpretResult result = interpret(vm, source);

  // free source from memory
  free(source);

  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
  // create VM
  VM vm;
  initVM(&vm);

  // repl
  if (argc == 1) {
    repl(&vm);
  } else if (argc == 2) {
    runFile(&vm, argv[1]);
  } else {
    fprintf(stderr, "Usage: clox [path]\n");
    exit(64);
  }

  // destroy VM
  freeVM(&vm);

  return 0;
}