#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_RETURN,
} OpCode;

// data structure for run-length line encoding
typedef struct {
  int offset; // offset in Chunk
  int line; // line in user code
} LineStart;

// dynamic array to hold instructions
typedef struct {
  int count; // size of array
  int capacity; // current max size of array
  uint8_t* code; // pointer to current position in array
  ValueArray constants; // array of constant values
  int lineCount;
  int lineCapacity;
  LineStart* lines; // array of lineStarts
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

// get line given chunk and offset
int getLine(Chunk* chunk, int instruction);

#endif