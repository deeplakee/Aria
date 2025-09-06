#include <cstring>

#include "conStringPool.h"
#include "gc.h"
#include "object/objString.h"

namespace aria {

ConStringPool::ConStringPool(GC *_gc)
    : count{0}
    , capacity{0}
    , table{nullptr}
    , gc{_gc}
{}
ConStringPool::~ConStringPool()
{
    gc->free_array<ObjStringPtr>(table, capacity);
}
bool ConStringPool::insert(ObjString *obj)
{
    if (count + 1 > capacity * TABLE_MAX_LOAD) {
        adjustCapacity(GC::grow_capacity(capacity));
    }
    ObjStringPtr *dest = findDest(table, obj, capacity);

    if (*dest == nullptr || *dest == TOMBSTONE) {
        *dest = obj;
        count++;
        return true;
    }

    if (*dest == obj) {
        return false;
    }

    cerr << __FILE__ << ":" << __LINE__ << ":" << __func__ << ": findDest error" << endl;
    return false;
}

ObjStringPtr ConStringPool::get(const char *chars, size_t length, uint32_t hash)
{
    if (count == 0 || capacity == 0)
        return nullptr;

    uint32_t index = hash & (capacity - 1);
    for (;;) {
        ObjStringPtr *eachEntry = &table[index];
        if (*eachEntry == nullptr) {
            return nullptr;
        }
        if (*eachEntry != TOMBSTONE) {
            ObjStringPtr objStr = *eachEntry;
            if (objStr->length == length && objStr->hash == hash
                && memcmp(objStr->C_str_ref(), chars, length) == 0) {
                return *eachEntry;
            }
        }
        index = (index + 1) & (capacity - 1);
    }
}

bool ConStringPool::remove(ObjString *obj)
{
    if (count == 0)
        return false;
    uint32_t index = obj->hash & (capacity - 1);
    for (;;) {
        ObjStringPtr *entry = &table[index];
        if (*entry == nullptr) {
            return false; // not found
        }
        if (*entry != TOMBSTONE) {
            ObjStringPtr candidate = *entry;
            if (candidate->length == obj->length && candidate->hash == obj->hash
                && memcmp(candidate->C_str_ref(), obj->C_str_ref(), obj->length) == 0) {
                *entry = TOMBSTONE;
                count--;
                return true;
            }
        }
        index = (index + 1) & (capacity - 1);
    }
}

void ConStringPool::removeWhite()
{
    for (int i = 0; i < capacity; i++) {
        ObjStringPtr entry = table[i];
        if (entry != nullptr && !entry->isMarked) {
            entry->isMarked = true;
            //tableDelete(table, entry->key);
        }
    }
}

ObjStringPtr *ConStringPool::findDest(ObjStringPtr *f_table, ObjStringPtr key, int f_capacity)
{
    uint32_t index = key->hash & (f_capacity - 1);
    ObjStringPtr *tombstone = nullptr;

    for (;;) {
        ObjStringPtr *eachPtr = &f_table[index];
        if (*eachPtr == nullptr) {
            return tombstone != nullptr ? tombstone : eachPtr;
        }
        if (*eachPtr == TOMBSTONE) {
            if (tombstone == nullptr)
                tombstone = eachPtr;
        } else {
            ObjStringPtr objStr = *eachPtr;
            if (objStr->length == key->length && objStr->hash == key->hash
                && memcmp(objStr->C_str_ref(), key->C_str_ref(), key->length) == 0) {
                return eachPtr;
            }
        }
        index = (index + 1) & (f_capacity - 1);
    }
}

void ConStringPool::adjustCapacity(int newCapacity)
{
    ObjStringPtr *newTable = gc->allocate_array<ObjStringPtr>(newCapacity);
    for (int i = 0; i < newCapacity; i++) {
        newTable[i] = nullptr;
    }

    count = 0;
    for (int i = 0; i < capacity; i++) {
        ObjStringPtr src = table[i];
        if (src == nullptr || src == TOMBSTONE)
            continue;

        ObjStringPtr *dest = findDest(newTable, src, newCapacity);
        *dest = src;
        count++;
    }

    gc->free_array<ObjStringPtr>(table, capacity);
    table = newTable;
    capacity = newCapacity;
}

} // namespace aria
