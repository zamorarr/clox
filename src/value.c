#include <stdio.h>

#include "memory.h"
#include "value.h"

// initialize an empty array of Values
void initValueArray(ValueArray* array) {
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

// append value to array of Values
void writeValueArray(ValueArray* array, Value value) {
  // grow array if needed
  if (array->capacity < (array->count + 1)) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
  }

  // append value to end of array
  array->values[array->count] = value;
  array->count++;
}

// destroy an array of Values
void freeValueArray(ValueArray* array) {
  FREE_ARRAY(Value, array->values, array->capacity);
  initValueArray(array);
}

// print value
void printValue(Value value) {
  printf("%g", value);
}