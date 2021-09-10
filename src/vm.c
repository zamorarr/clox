#include <stdarg.h>
#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

// reset stack
static void resetStack(VM* vm) {
  vm->stackTop = vm->stack;
}

// runtime error
static void runtimeError(VM* vm, const char* format, ...) {
  // what the heck is this. a variadic function?
  va_list args;
  // sets args to the ... argument in the function
  va_start(args, format);

  // print format to stderr, substituting values from args in it
  vfprintf(stderr, format, args);
  va_end(args);

  // print new line
  fputs("\n", stderr);

  // print line number of error
  size_t instruction = vm->ip - vm->chunk->code - 1;
  int line = vm->chunk->lines[instruction].line;
  fprintf(stderr, "[line %d] in script\n", line);

  // reset stack to empty
  resetStack(vm);
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

// peek at value ontop of stack
static Value peek(VM* vm, int distance) {
  return vm->stackTop[-1 - distance];
}

// check if value is "falsey" - nil or false
static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

// run code chunk
static InterpretResult run(VM* vm) {
  // READ_BYTE: gets address of byte pointed at by ip, dereferences, 
  // and then advances the instruction pointer
  #define READ_BYTE() (*(vm->ip)++)
  
  // READ_CONSTANT 
  #define READ_CONSTANT() (vm->chunk->constants.values[READ_BYTE()])

  // BINARY_OP: performs binary op on stack
  #define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) { \
        runtimeError(vm, "Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop(vm)); \
      double a = AS_NUMBER(pop(vm)); \
      push(vm, valueType(a op b)); \
    } while (false)

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

      // literals
      case OP_NIL: push(vm, NIL_VAL); break;
      case OP_TRUE: push(vm, BOOL_VAL(true)); break;
      case OP_FALSE: push(vm, BOOL_VAL(false)); break;

      // equality and comparisons
      case OP_EQUAL: {
        Value b = pop(vm);
        Value a = pop(vm);
        push(vm, BOOL_VAL(valuesEqual(a, b)));
        break;
      }
      case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
      case OP_LESS: BINARY_OP(BOOL_VAL, <); break;

      // unary operations
      case OP_NEGATE: {
        // peek at top of stack
        if (!IS_NUMBER(peek(vm, 0))) {
          // throw error
          runtimeError(vm, "Operand must be number");
          return INTERPRET_RUNTIME_ERROR;
        }

        // negate the current value and put it back on top
        // push(vm, NUMBER_VAL(-AS_NUMBER(pop(vm)))); 
        // break;
        
        // instead of a push/pop combo, we will just modify the value in-place
        *(vm->stackTop - 1) = NUMBER_VAL(-AS_NUMBER(*(vm->stackTop - 1))); 
        break;
      }

      // binary operations
      case OP_ADD: BINARY_OP(NUMBER_VAL, +); break;
      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /); break;

      case OP_NOT:
        push(vm, BOOL_VAL(isFalsey(pop(vm))));
        break;

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
InterpretResult interpret(VM* vm, const char* source) {
  // initialize chunk
  Chunk chunk;
  initChunk(&chunk);

  // compile source to bytecodes in chunk
  if (!compile(vm, source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  // initialize chunk in VM
  vm->chunk = &chunk;
  vm->ip = vm->chunk->code;

  // run vm
  InterpretResult result = run(vm);

  // free chunk
  freeChunk(&chunk);
  return result;
}