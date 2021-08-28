#ifndef clox_value_h
#define clox_value_h

#include "common.h"

// object to hold double-precision values
typedef double Value;

// array to  hold Values
typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;

// initialize an empty array of Values
void initValueArray(ValueArray* array);

// append value to array of Values
void writeValueArray(ValueArray* array, Value value);

// destroy an array of Values
void freeValueArray(ValueArray* array);

// print value
void printValue(Value value);

#endif