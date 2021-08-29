#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
  // create VM
  VM vm;
  initVM(&vm);

  // create chunk
  Chunk chunk;
  initChunk(&chunk);

  // add a test opcodes
  writeConstant(&chunk, 1.2, 123);
  writeConstant(&chunk, 3.4, 123);
  writeChunk(&chunk, OP_ADD, 123);
  writeConstant(&chunk, 5.6, 123);
  writeChunk(&chunk, OP_DIVIDE, 123);
  writeChunk(&chunk, OP_NEGATE, 123);
  writeChunk(&chunk, OP_RETURN, 124);

  // show chunk
  //disassembleChunk(&chunk, "test chunk");

  // interpret chunk
  interpret(&vm, &chunk);

  // destroy chunk
  freeChunk(&chunk);

  // destroy VM
  freeVM(&vm);

  return 0;
}