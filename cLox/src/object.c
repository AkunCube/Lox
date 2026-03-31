#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static Obj *allocateObject(size_t objSize, ObjType type);
static ObjString *allocateString(char *chars, int length, uint32_t hash);
static uint32_t hashString(const char *key, int length);
static void printFunction(ObjFunction *func);

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType)

ObjString *copyString(const char *chars, int length) {
  uint32_t hash = hashString(chars, length);

  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL)
    return interned;

  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';

  return allocateString(heapChars, length, hash);
}

void printObject(Value value) {
  assert(AS_OBJ(value) != NULL);
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
    case OBJ_FUNCTION:
      printFunction(AS_FUNCTION(value));
      break;
    case OBJ_CLOSURE:
      printFunction(AS_CLOSURE(value)->function);
      break;
    case OBJ_NATIVE:
      printf("<native fn>");
      break;
    default:
      break;
  }
}

ObjFunction *newFunction() {
  ObjFunction *func = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  func->arity = 0;
  func->upValueCount = 0;
  func->funcName = NULL;
  initChunk(&func->chunk);
  return func;
}

ObjNative *newNative(NativeFn *function) {
  ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function = function;

  return native;
}

ObjClosure *newClosure(ObjFunction *function) {
  ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
  closure->function = function;
  return closure;
}

ObjString *takeString(char *chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);

  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocateString(chars, length, hash);
}

/***********************************************************************/

static ObjString *allocateString(char *chars, int length, uint32_t hash) {
  ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->chars = chars;
  string->length = length;
  string->hash = hash;

  tableSet(&vm.strings, string, NIL_VAL);
  return string;
}

static Obj *allocateObject(size_t objSize, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, 0, objSize);

  object->type = type;

  object->next = vm.objects;
  vm.objects = object;
  return object;
}

static uint32_t hashString(const char *key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; ++i) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

static void printFunction(ObjFunction *func) {
  if (func->funcName == NULL) {
    printf("<script>");
    return;
  }
  printf("<fn %s>", func->funcName->chars);
}
