#ifndef VALUEHASHTABLE_H
#define VALUEHASHTABLE_H

#include "value.h"

namespace aria {
class ObjString;
class ObjList;
class ValueArray;
class GC;
#define TABLE_MAX_LOAD 0.75

struct KVPair
{
    bool isEmpty;
    Value key;
    Value value;
};

inline void initKVPair(KVPair *pair, bool _isEmpty, Value _key, Value _value)
{
    pair->isEmpty = _isEmpty;
    pair->key = _key;
    pair->value = _value;
}

inline void initKVPair(KVPair *pair)
{
    pair->isEmpty = true;
    pair->key = nil_val;
    pair->value = nil_val;
}

class ValueHashTable
{
public:
    explicit ValueHashTable(GC *_gc);

    ~ValueHashTable();

    bool insert(Value k, Value v);

    bool get(Value k, Value &v);

    bool has(Value k);

    int size() const;

    bool remove(Value k);

    void copy(ValueHashTable *other);

    bool equals(ValueHashTable *other);

    void clear();

    String toString(ValueStack *outer) const;

    String toRawString(ValueStack *outer) const;

    void mark();

    ValueArray *genKeysArray(GC *gc);

    ValueArray *genValuesArray(GC *gc);

    friend Value builtin_pairs(int argCount, Value *args, GC *gc);

    ObjList *createPairList(int index,GC *gc);

    int getNextIndex(int pre);

    Value getByIndex(int index, GC *gc);

private:
    int true_count;
    int count;
    int capacity;
    KVPair *entry;
    GC *gc;

    static KVPair *findDest(KVPair *f_entry, Value key, int f_capacity);

    void adjustCapacity(int newCapacity);
};
} // namespace aria

#endif // VALUEHASHTABLE_H