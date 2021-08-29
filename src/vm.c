#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

// reset stack
static void resetStack(VM* vm) {
  vm->stackTop = vm->stack;
}

// create vm
void initVM(VM* vm) {
  resetStack(vm);
}

// destroy vm
void freeVM(VM* vm) {

}

// push value onto stack
void push(VM* vm, Value value) {
  *vm->stackTop = value;
  vm->stackTop++;
}

// pop value from stack
Value pop(VM* vm) {
  vm->stackTop--;
  return *vm->stackTop;
}

// run code chunk
static InterpretResult run(VM* vm) {
  // READ_BYTE: gets address of byte pointed at by ip, dereferences, 
  // and then advances the instruction pointer
  #define READ_BYTE() (*(vm->ip)++)
  
  // READ_CONSTANT 
  #define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])

  // BINARY_OP: performs binary op on stack
  #define BINARY_OP(op) \
    do { \
      Value b = pop(vm); \
      Value a = pop(vm); \
      push(vm, a op b); \
    } while (false);

  // main loop to read all instructions in chunk
  for (;;) {
    #ifdef DEBUG_TRACE_EXECUTION

    // show stack contents
    printf("          ");
    for (Value* slot = vm->stack; slot < vm->stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");

    // show instruction
    int offset = (int)(vm->ip - vm->chunk->code);
    disassembleInstruction(vm->chunk, offset);
    #endif

    // read next instruction
    uint8_t instruction = READ_BYTE();

    // choose action
    switch(instruction) {

      // load constant
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(vm, constant);
        //printf("\n");
        break;
      }
      case OP_CONSTANT_LONG: {
        uint8_t byte1 = READ_BYTE();
        uint8_t byte2 = READ_BYTE() << 8;
        uint8_t byte3 = READ_BYTE() << 16;
        uint32_t index = byte1 | byte2 | byte3;
        Value constant = vm->chunk->constants.values[index];
        push(vm, constant);
        //printf("\n");
        break;
      }

      // unary operations
      case OP_NEGATE: {
        // instead of a push/pop combo, we will just modify the value in-place
        // push(vm, -pop(vm)); break;
        *(vm->stackTop - 1) = -*(vm->stackTop - 1); 
        break;
      }

      // binary operations
      case OP_ADD: BINARY_OP(+); break;
      case OP_SUBTRACT: BINARY_OP(-); break;
      case OP_MULTIPLY: BINARY_OP(*); break;
      case OP_DIVIDE: BINARY_OP(/); break;

      // return value
      case OP_RETURN:
        printValue(pop(vm));
        printf("\n");
        return INTERPRET_OK;
    }
  }

  #undef READ_BYTE
  #undef READ_CONSTANT
  #undef BINARY_OP
}

// interpret code chunk
InterpretResult interpret(VM* vm, Chunk* chunk) {
  // initialize vm code chunk
  vm->chunk = chunk;

  // initialize instruction pointer to first byte in code chunk
  vm->ip = vm->chunk->code;

  // execut result
  return run(vm);
}