#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_RETURN,
} OpCode;

// dynamic array to hold instructions
typedef struct {
  int count; // size of array
  int capacity; // current max size of array
  uint8_t* code; // pointer to current position in array
  int* lines; // array of line values
  ValueArray constants; // array of constant values
} Chunk;

// initialize an empty chunk
void initChunk(Chunk* chunk);

// append byte to end of chunk
void writeChunk(Chunk* chunk, uint8_t byte, int line);

// add constant to constants array
// returns index in ValueArray where constant is
int addConstant(Chunk* chunk, Value value);

// delete chunk and free memory
void freeChunk(Chunk* chunk);

#endif