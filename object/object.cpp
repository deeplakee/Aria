#include "object.h"

#include "memory/gc.h"
#include "util/util.h"

namespace aria {
const char *objTypeStr[] = {
    "BASE",
    "STRING",
    "FUNCTION",
    "NATIVE_FN",
    "UPVALUE",
    "CLASS",
    "INSTANCE",
    "BOUND_METHOD",
    "LIST",
    "MAP",
    "MODULE",
    "ITERATOR",
};

void Obj::mark(GC *gc)
{
    if (isMarked)
        return;
    if (type==objType::BASE) {
        cerr << "error type"<<endl;
    }
#ifdef DEBUG_LOG_GC
    print("{:p} mark {}\n", toVoidPtr(this), this->toString());
#endif
    isMarked = true;
    gc->addToGrey(this);
}
} // namespace aria