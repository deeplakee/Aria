#include "valueArray.h"

#include "memory/gc.h"
#include "object/objList.h"
#include "object/objMap.h"
namespace aria {
ValueArray::ValueArray(GC *_gc)
    : values{nullptr}
    , capacity{0}
    , count{0}
    , gc{_gc}
{}
ValueArray::ValueArray(int begin, int end, ValueArray *other, GC *_gc)
    : values{nullptr}
    , capacity{0}
    , count{0}
    , gc{_gc}
{
    int size = end - begin;
    growCapacity(cal_slice_size(size));
    Value *from = other->values + begin;
    memcpy(values, from, size * sizeof(Value));
    count = size;
}

ValueArray::~ValueArray()
{
    gc->free_array<Value>(values, capacity);
}

Value &ValueArray::operator[](int index)
{
    return values[index];
}

const Value &ValueArray::operator[](int index) const
{
    return values[index];
}

void ValueArray::write(Value value)
{
    if (capacity < count + 1) {
        int oldCapacity = capacity;
        capacity = GC::grow_capacity(oldCapacity);
        values = gc->grow_array<Value>(values, oldCapacity, capacity);
    }

    values[count] = value;
    count++;
}
void ValueArray::pop()
{
    count--;
}
bool ValueArray::remove(int index)
{
    if (index > count || index < 0)
        return false;
    count--;
    for (int i = index; i < count; i++) {
        values[i] = values[i + 1];
    }
    return true;
}

bool ValueArray::insert(int index, Value v)
{
    if (index > count || index < 0)
        return false;
    if (capacity < count + 1) {
        int oldCapacity = capacity;
        capacity = GC::grow_capacity(oldCapacity);
        values = gc->grow_array<Value>(values, oldCapacity, capacity);
    }

    for (int i = count; i > index; i--) {
        values[i] = values[i - 1];
    }
    values[index] = v;
    count++;
    return true;
}
void ValueArray::extend(ValueArray *other)
{
    int newsize = count + other->count;
    growCapacity(newsize);
    memcpy(values + count, other->values, other->count * sizeof(Value));
    count = newsize;
}
void ValueArray::clear()
{
    count = 0;
    growCapacity(0);
}
void ValueArray::reverse()
{
    for (int i = 0; i < count / 2; i++) {
        swap(values[i], values[count - 1 - i]);
    }
}

int ValueArray::size() const
{
    return count;
}

String ValueArray::toString(ValueStack *outer) const
{
    String listStr = "[";
    if (count == 0) {
        listStr += "]";
    }
    for (int i = 0; i < count; i++) {
        Value elem = values[i];
        listStr += valueString(elem, outer);
        if (i != count - 1) {
            listStr += ",";
        } else {
            listStr += "]";
        }
    }
    return listStr;
}

String ValueArray::toRawString(ValueStack *outer) const
{
    String listStr = "[";
    if (count == 0) {
        listStr += "]";
    }
    for (int i = 0; i < count; i++) {
        listStr += rawValueString(values[i], outer);
        if (i != count - 1) {
            listStr += ",";
        } else {
            listStr += "]";
        }
    }
    return listStr;
}
void ValueArray::copy(ValueArray *other)
{
    growCapacity(other->capacity);
    memcpy(values, other->values, other->count * sizeof(Value));
    count = other->count;
}

bool ValueArray::equals(ValueArray *other) const
{
    if (count != other->count)
        return false;
    for (int i = 0; i < count; i++) {
        if (!valuesEqual(values[i], other->values[i]))
            return false;
    }
    return true;
}

void ValueArray::mark()
{
    for (int i = 0; i < count; i++) {
        markValue(values[i], gc);
    }
}
void ValueArray::growCapacity(int newCapacity)
{
    int oldCapacity = capacity;
    values = gc->grow_array<Value>(values, oldCapacity, newCapacity);
    capacity = newCapacity;
}

int cal_slice_size(int size)
{
    if (size < 0) {
        return 0;
    }
    int power = 1;
    while (power < size) {
        power *= 2;
    }
    return power < 8 ? 8 : power;
}
} // namespace aria
