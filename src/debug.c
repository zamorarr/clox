#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  // constant value is located 1 after chunk offset
  uint8_t constant_idx = chunk->code[offset + 1];

  // print info
  printf("%-16s %d '", name, constant_idx);
  printValue(chunk->constants.values[constant_idx]);
  printf("'\n");

  // return new offset
  return offset + 2;
}

static int constantLongInstruction(const char* name, Chunk* chunk, int offset) {
  // constant value is located 1 after chunk offset
  uint8_t byte1 = chunk->code[offset + 1];
  uint8_t byte2 = chunk->code[offset + 2] << 8;
  uint8_t byte3 = chunk->code[offset + 3] << 16;
  uint32_t index = byte1 | byte2 | byte3;

  // print info
  printf("%-16s %4d '", name, index);
  printValue(chunk->constants.values[index]);
  printf("'\n");

  // return new offset
  return offset + 4;
}

static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset) {
  // print op stack position
  printf("%04d ", offset);

  // print line number
  int line = getLine(chunk, offset);
  if (offset > 0 && line == getLine(chunk, offset - 1)) {
    printf("   | ");
  } else {
    printf("%4d ", line);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_CONSTANT_LONG:
      return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    default:
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
  }
}

