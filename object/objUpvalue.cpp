#include "objUpvalue.h"

#include "memory/gc.h"

namespace aria {

ObjUpvalue::ObjUpvalue(Value *_location, GC *_gc)
    : location{_location}
    , closed{nil_val}
    , nextUpvalue{nullptr}
{
    type = objType::UPVALUE;
    gc = _gc;
}

ObjUpvalue::~ObjUpvalue() = default;

String ObjUpvalue::toString()
{
    return String{"upvalue:" + valueString(*location)};
}

String ObjUpvalue::toRawString()
{
    return String{"upvalue:" + rawValueString(*location)};
}

void ObjUpvalue::blacken(GC *gc)
{
    markValue(closed, gc);
}

ObjUpvalue *newObjUpvalue(Value *location, GC *gc)
{
    ObjUpvalue *newObjUpvalue = gc->allocate_object<ObjUpvalue>(location, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjUpvalue\n", toVoidPtr(newObjUpvalue), sizeof(ObjUpvalue));
#endif

    return newObjUpvalue;
}

} // namespace aria
