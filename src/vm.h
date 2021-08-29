#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

// VM definition
typedef struct {
  // pointer to current code chunk
  Chunk* chunk;
  
  // current instruction pointer
  uint8_t* ip;

  // stack (array of values)
  Value stack[STACK_MAX];
  Value* stackTop;
} VM;

// interpret enums
typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

// create and destroy VM
void initVM(VM* vm);
void freeVM(VM* vm);

// interpret code chunk
InterpretResult interpret(VM* vm, Chunk* chunk);

// push/pop values onto stack
void push(VM* vm, Value value);
Value pop(VM* vm);

#endif