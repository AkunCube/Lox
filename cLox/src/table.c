#include "table.h"
#include "memory.h"
#include "value.h"
#include <assert.h>
#include <string.h>

#define TABLE_MAX_LOAD 0.75
static Entry *findEntry(Entry *entries, int capacity, ObjString *key);
static void adjustCapacity(Table *table, int capacity);

void initTable(Table *table) {
  table->count = 0;
  table->capacity = 0;
  table->entries = NULL;
}

void freeTable(Table *table) {
  FREE_ARRAY(Entry, table->entries, table->capacity);
  initTable(table);
}

bool tableSet(Table *table, ObjString *key, Value value) {
  if (table->count + 1 > (table->capacity * TABLE_MAX_LOAD)) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
  }

  Entry *entry = findEntry(table->entries, table->capacity, key);
  bool isNewKey = (entry->key == NULL);

  if (isNewKey && IS_NIL(entry->value))
    ++table->count;

  entry->key = key;
  entry->value = value;

  return isNewKey;
}

static Entry *findEntry(Entry *entries, int capacity, ObjString *key) {
  uint32_t index = key->hash & (capacity - 1);
  Entry *tombstone = NULL;

  while (true) {
    Entry *entry = &entries[index];
    assert(entry != NULL);

    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
        // empty entry.
        return (tombstone != NULL) ? tombstone : entry;
      } else {
        // We found a tombstone.
        if (tombstone == NULL)
          tombstone = entry;
      }
    } else if (entry->key == key) {
      // find key.
      return entry;
    }

    index = (index + 1) & (capacity - 1);
  }
}

static void adjustCapacity(Table *table, int capacity) {
  Entry *entries = ALLOCATE(Entry, capacity);

  // 1. init new entries.
  for (int i = 0; i < capacity; ++i) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  table->count = 0;

  // 2. insert every entry.
  for (int i = 0; i < table->capacity; ++i) {
    Entry *entry = &table->entries[i];
    assert(entry != NULL);
    if (NULL == entry->key)
      continue;
    Entry *dest = findEntry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    ++table->count;
  }

  // 3. delete old entries.
  FREE_ARRAY(Entry, table->entries, table->capacity);
  table->entries = entries;
  table->capacity = capacity;
}

void tableAddAll(Table *from, Table *to) {
  for (int i = 0; i < from->capacity; ++i) {
    Entry *entry = &from->entries[i];
    assert(entry != NULL);
    if (entry->key != NULL) {
      tableSet(to, entry->key, entry->value);
    }
  }
}

bool tableGet(Table *table, ObjString *key, Value *value) {
  if (0 == table->count)
    return false;

  Entry *entry = findEntry(table->entries, table->capacity, key);
  assert(entry != NULL);

  if (entry->key == NULL)
    return false;

  *value = entry->value;
  return true;
}

bool tableDelete(Table *table, ObjString *key) {
  if (0 == table->count)
    return false;

  // find entry
  Entry *entry = findEntry(table->entries, table->capacity, key);
  assert(entry != NULL);

  if (entry->key == NULL)
    return false;

  //* Place a tombstone in the entry.
  entry->key = NULL;
  //* distinguish between `nil` and `true`.
  entry->value = BOOL_VAL(true);

  return true;
}

ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash) {
  if (table->count == 0)
    return NULL;

  uint32_t index = hash & (table->capacity - 1);
  while (true) {
    Entry *entry = &table->entries[index];
    if (entry->key == NULL) {
      // Stop if we find an empty non-tombstone entry.
      if (IS_NIL(entry->value))
        return NULL;
    } else if (entry->key->length == length && entry->key->hash == hash &&
               memcmp(entry->key->chars, chars, length) == 0) {
      return entry->key;
    }

    index = (index + 1) & (table->capacity - 1);
  }
}

void markTable(Table *table) {
  for (int i = 0; i < table->capacity; ++i) {
    Entry *entry = &table->entries[i];
    markObject((Obj *)entry->key);
    markValue(entry->value);
  }
}

void tableRemoveWhite(Table *table) {
  for (int i = 0; i < table->capacity; ++i) {
    Entry *entry = &table->entries[i];
    if (entry->key && !entry->key->obj.isMarked) {
      tableDelete(table, entry->key);
    }
  }
}
