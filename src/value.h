#ifndef clox_value_h
#define clox_value_h

#include "common.h"

// types of values
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
} ValueType;

// object (tagged union) to hold values
typedef struct {
  ValueType type;

  // size of union is size of largest member
  union {
    bool boolean;
    double number;
  } as;

} Value;

// macros to check if Value is type
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

// macros to create C-type from Lox Value
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

// macros to create Lox Value from C-type
#define BOOL_VAL(value) ((Value) {VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value) {VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value) {VAL_NUMBER, {.number = value}})

// array to  hold Values
typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);

// initialize an empty array of Values
void initValueArray(ValueArray* array);

// append value to array of Values
void writeValueArray(ValueArray* array, Value value);

// destroy an array of Values
void freeValueArray(ValueArray* array);

// print value
void printValue(Value value);

#endif