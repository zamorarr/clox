#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
  // create chunk
  Chunk chunk;
  initChunk(&chunk);

  // add a test opcodes
  writeConstant(&chunk, 1.2, 123);
  writeConstant(&chunk, 200, 124);
  writeChunk(&chunk, OP_RETURN, 124);

  // show opcode
  disassembleChunk(&chunk, "test chunk");

  // destroy chunk
  freeChunk(&chunk);

  return 0;
}