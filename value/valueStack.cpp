#include "valueStack.h"

#include "common.h"
#include "memory/gc.h"

namespace aria {
ValueStack::ValueStack()
{
    top = 0;
    stack = new Value[stackSize];
}

ValueStack::~ValueStack()
{
    delete[] stack;
}

void ValueStack::mark(GC *gc)
{
    for (int i = 0; i < top; i++) {
        markValue(stack[i],gc);
    }
}

} // namespace aria
