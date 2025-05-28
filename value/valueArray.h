#ifndef VALUEARRAY_H
#define VALUEARRAY_H

#include "value.h"

namespace aria {
class GC;

class ValueArray
{
public:
    explicit ValueArray(GC *_gc);

    ValueArray(int begin, int end, ValueArray *other, GC *_gc);

    ~ValueArray();

    // No cross-border handling has been done!!!
    Value &operator[](int index);

    // No cross-border handling has been done!!!
    const Value &operator[](int index) const;

    void write(Value value);

    void pop();

    bool remove(int index);

    bool insert(int index, Value v);

    void extend(ValueArray *other);

    void clear();

    void reverse();

    int size() const;

    String toString(ValueStack *outer) const;

    String toRawString(ValueStack *outer) const;

    void copy(ValueArray *other);

    bool equals(ValueArray *other) const;

    void mark();

    void growCapacity(int newCapacity);

private:
    int capacity;
    int count;
    Value *values;
    GC *gc;
};
int cal_slice_size(int size);

} // namespace aria

#endif // VALUEARRAY_H
