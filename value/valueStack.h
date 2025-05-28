#ifndef VALUESTACK_H
#define VALUESTACK_H

#include "value.h"

namespace aria {
class GC;
static constexpr int stackSize = 65535;

class ValueStack
{
public:
    ValueStack();

    ~ValueStack();

    void push(Value value)
    {
        stack[top] = value;
        top++;
    }

    Value pop()
    {
        top--;
        return stack[top];
    }

    void reset() { top = 0; }

    void pop_n(int count) { top -= count; }

    Value peek(int depth = 0) { return stack[top - 1 - depth]; }

    int size() const { return top; }

    void resize(int newTop) { top = newTop; }

    Value *getTop() { return &stack[top]; }

    void replaceTop(Value v) { stack[top - 1] = v; }

    Value *getBase() { return stack; }

    bool isExist(Value v)
    {
        if (!is_obj(v)) {
            return false;
        }
        Obj *obj = as_obj(v);
        for (int slot = 0; slot < top; slot++) {
            if (is_obj(stack[slot]) && as_obj(stack[slot]) == obj) {
                return true;
            }
        }
        return false;
    }

    void print() const
    {
        for (int slot = 0; slot < top; slot++) {
            cout << "[ " << valueString(stack[slot]) << " ]";
        }
    }

    Value &operator[](int index) { return stack[index]; }

    const Value &operator[](int index) const { return stack[index]; }

    void mark(GC *gc);

private:
    Value *stack;
    int top;
};
} // namespace aria

#endif // VALUESTACK_H