#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// initialize an empty chunk
void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lineCount = 0;
  chunk->lineCapacity = 0;
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
  }

  // write byte to end of chunk
  chunk->code[chunk->count] = byte;
  chunk->count++;

  // do we need to add a new lineStart?
  if (chunk->lineCount > 0 && chunk->lines[chunk->lineCount - 1].line == line) {
    return;
  }

  // grow lines array if needed
  if (chunk->lineCapacity < (chunk->lineCount + 1)) {
    int oldCapacity = chunk->lineCapacity;
    chunk->lineCapacity = GROW_CAPACITY(chunk->lineCapacity);
    chunk->lines = GROW_ARRAY(LineStart, chunk->lines, oldCapacity, chunk->lineCapacity);
  }

  // update lines array
  LineStart* lineStart = &chunk->lines[chunk->lineCount];
  lineStart->offset = chunk->count - 1;
  lineStart->line = line;
  chunk->lineCount++;
  
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
  FREE_ARRAY(LineStart, chunk->lines, chunk->lineCapacity);

  // free value array
  freeValueArray(&chunk->constants);

  // make chunk empty
  initChunk(chunk);
}

// get line given chunk and offset
int getLine(Chunk* chunk, int instruction) {
  int start = 0;
  int end = chunk->lineCount - 1;

  // binary search to find line number
  for (;;) {
    int mid = (start + end) / 2;
    LineStart* line = &chunk->lines[mid];
    if (instruction < line->offset) {
      // search lower
      end = mid - 1;
    } else if (mid == chunk->lineCount - 1 || instruction < chunk->lines[mid + 1].offset) {
      // we found it
      // instruction between lines[mid].offset and lines[mid + 1].offset
      return line->line;
    } else {
      // search higher
      start = mid + 1;
    }
  }
}