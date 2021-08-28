#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// initialize an empty chunk
void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  initValueArray(&chunk->constants);
}

// append byte to end of chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
  // grow chunk if needed
  if (chunk->capacity < (chunk->count + 1)) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
  }

  // write byte to end of chunk
  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

// add constant to constants array
int addConstant(Chunk* chunk, Value value) {
  writeValueArray(&chunk->constants, value);
  return chunk->constants.count - 1;
}

// delete chunk and free memory
void freeChunk(Chunk* chunk) {
  // free chunk
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);

  // free lines array
  FREE_ARRAY(uint8_t, chunk->lines, chunk->capacity);

  // free value array
  freeValueArray(&chunk->constants);

  // make chunk empty
  initChunk(chunk);
}