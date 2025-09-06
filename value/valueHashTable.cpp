#include "valueHashTable.h"

#include <cstring>

#include "common.h"
#include "memory/gc.h"
#include "object/objList.h"
#include "object/objString.h"
#include "valueArray.h"

namespace aria {
ValueHashTable::ValueHashTable(GC *_gc)
    : true_count{0}
    , count{0}
    , capacity{0}
    , entry{nullptr}
    , gc{_gc}
{}

ValueHashTable::~ValueHashTable()
{
    gc->free_array<KVPair>(entry, capacity);
}

bool ValueHashTable::insert(Value k, Value v)
{
    if (count + 1 > capacity * TABLE_MAX_LOAD) {
        adjustCapacity(GC::grow_capacity(capacity));
    }
    KVPair *dest = findDest(entry, k, capacity);
    bool isNewKey = dest->isEmpty;
    if (isNewKey) {
        true_count++;
        if (is_nil(dest->value))
            count++;
    }

    dest->isEmpty = false;
    dest->key = k;
    dest->value = v;
    return isNewKey;
}

bool ValueHashTable::get(Value k, Value &v)
{
    if (count == 0)
        return false;
    const KVPair *dest = findDest(entry, k, capacity);
    if (dest->isEmpty)
        return false;
    v = dest->value;
    return true;
}

bool ValueHashTable::has(Value k)
{
    if (count == 0)
        return false;
    const KVPair *dest = findDest(entry, k, capacity);
    if (dest->isEmpty)
        return false;
    return true;
}

int ValueHashTable::size() const
{
    return true_count;
}

bool ValueHashTable::remove(Value k)
{
    if (count == 0)
        return false;
    KVPair *dest = findDest(entry, k, capacity);
    if (dest->isEmpty)
        return false;

    dest->isEmpty = true;
    dest->key = nil_val;
    dest->value = true_val;
    true_count--;
    return true;
}

void ValueHashTable::copy(ValueHashTable *other)
{
    for (int i = 0; i < other->capacity; i++) {
        KVPair *src = &other->entry[i];
        if (src->isEmpty)
            continue;
        insert(src->key, src->value);
    }
}

bool ValueHashTable::equals(ValueHashTable *other)
{
    if (true_count != other->true_count) {
        return false;
    }
    for (int i = 0; i < capacity; i++) {
        KVPair *each = &entry[i];
        if (each->isEmpty)
            continue;
        Value v = nil_val;
        if (!other->get(each->key, v)) {
            return false;
        }
        if (!valuesEqual(each->value, v)) {
            return false;
        }
    }
    return true;
}

void ValueHashTable::clear()
{
    gc->free_array<KVPair>(entry, capacity);
    true_count = 0;
    count = 0;
    capacity = 0;
    entry = nullptr;
}

String ValueHashTable::toString(ValueStack *outer) const
{
    String str = "{";
    bool empty = true;
    for (int i = 0; i < capacity; i++) {
        if (entry[i].isEmpty)
            continue;
        str += valueString(entry[i].key, outer);
        str += ":";
        str += valueString(entry[i].value, outer);
        str += ",";
        empty = false;
    }
    if (empty) {
        return "{}";
    }
    str.pop_back();
    str += "}";
    return str;
}
String ValueHashTable::toRawString(ValueStack *outer) const
{
    String str = "{";
    bool empty = true;
    for (int i = 0; i < capacity; i++) {
        if (entry[i].isEmpty)
            continue;
        str += rawValueString(entry[i].key, outer);
        str += ":";
        str += rawValueString(entry[i].value, outer);
        str += ",";
        empty = false;
    }
    if (empty) {
        return "{}";
    }
    str.pop_back();
    str += "}";
    return str;
}

void ValueHashTable::mark()
{
    for (int i = 0; i < capacity; i++) {
        KVPair *each = &entry[i];
        if (each->isEmpty)
            continue;
        markValue(each->key, gc);
        markValue(each->value, gc);
    }
}
ValueArray *ValueHashTable::genKeysArray(GC *gc)
{
    ValueArray *array = new ValueArray{gc};
    for (int i = 0; i < capacity; i++) {
        if (entry[i].isEmpty)
            continue;
        array->write(entry[i].key);
    }
    return array;
}
ValueArray *ValueHashTable::genValuesArray(GC *gc)
{
    ValueArray *array = new ValueArray{gc};
    for (int i = 0; i < capacity; i++) {
        if (entry[i].isEmpty)
            continue;
        array->write(entry[i].value);
    }
    return array;
}

ObjList *ValueHashTable::createPairList(int index, GC *gc)
{
    if (index < 0 || index >= capacity) {
        return nullptr;
    }
    ObjList *list = newObjList(gc);
    gc->cache(obj_val(list));
    list->list->write(entry[index].key);
    list->list->write(entry[index].value);
    gc->releaseCache(1);
    return list;
}

int ValueHashTable::getNextIndex(int pre)
{
    // -2 means reach the end
    if (pre == -2) {
        return -2;
    }
    // -1 means begin
    if (pre == -1) {
        for (int i = 0; i < capacity; i++) {
            if (entry[i].isEmpty)
                continue;
            return i;
        }
        return -2;
    }
    // Check if pre is within valid range
    if (pre < 0 || pre >= capacity) {
        return -2;
    }
    for (int i = pre + 1; i < capacity; i++) {
        if (entry[i].isEmpty)
            continue;
        return i;
    }
    return -2;
}

Value ValueHashTable::getByIndex(int index, GC *gc)
{
    if (index < 0 || index >= capacity) {
        return nil_val;
    }
    ObjList *obj = createPairList(index, gc);
    if (obj == nullptr) {
        return nil_val;
    }
    return obj_val(obj);
}

// isEmpty = true; value = true_val;  means a tombstone
// isEmpty = true; value = nil_val;   means empty
// isEmpty = false;                   means the KVPair has been used
// else undefined
KVPair *ValueHashTable::findDest(KVPair *f_entry, Value key, int f_capacity)
{
    uint32_t index = valueHash(key) & (f_capacity - 1);
    KVPair *tombstone = nullptr;
    for (;;) {
        KVPair *eachEntry = &f_entry[index];
        if (eachEntry->isEmpty) {
            if (is_nil(eachEntry->value)) {
                return tombstone != nullptr ? tombstone : eachEntry;
            }
            if (tombstone == nullptr)
                tombstone = eachEntry;
        } else if (valuesEqual(eachEntry->key, key)) {
            return eachEntry;
        }

        index = (index + 1) & (f_capacity - 1);
    }
}

void ValueHashTable::adjustCapacity(int newCapacity)
{
    KVPair *newEntry = gc->allocate_array<KVPair>(newCapacity);
    for (int i = 0; i < newCapacity; i++) {
        initKVPair(&newEntry[i]);
    }
    count = 0;
    for (int i = 0; i < capacity; i++) {
        KVPair *src = &entry[i];
        if (src->isEmpty) {
            continue;
        }
        KVPair *dest = findDest(newEntry, src->key, newCapacity);
        initKVPair(dest, false, src->key, src->value);
        count++;
    }

    gc->free_array<KVPair>(entry, capacity);
    entry = newEntry;
    capacity = newCapacity;
    true_count = count;
}
} // namespace aria