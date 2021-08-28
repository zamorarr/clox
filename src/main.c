#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
  // create chunk
  Chunk chunk;
  initChunk(&chunk);

  // add a test opcode
  int constant = addConstant(&chunk, 1.2);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);
  writeChunk(&chunk, OP_RETURN, 123);

  // show opcode
  disassembleChunk(&chunk, "test chunk");

  // destroy chunk
  freeChunk(&chunk);

  return 0;
}