#include <stdlib.h>

#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
  // delete pointer if newSize is zero
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }

  // reallocate array size to newSize
  void* result = realloc(pointer, newSize);

  // if realloc() can't allocate memory it will return NULL
  if (result == NULL) exit(1);

  // return result
  return result;
}