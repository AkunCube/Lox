#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include <stdint.h>

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
  ObjClosure *closure; // callee.
  uint8_t *ip;         // callee's own ip.
  Value *slots;        // the first slot that this function can use
} CallFrame;

typedef struct {
  CallFrame frames[FRAMES_MAX]; // call frames
  int frameCount;
  Value stack[STACK_MAX];
  Value *stackTop;
  Table strings;
  Table globals;
  Obj *objects;
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();

InterpretResult interpret(const char *source);
void push(Value value);
Value pop();
Value *top();

extern VM vm;

#endif
