#include "objClass.h"

#include "memory/gc.h"
#include "objFunction.h"
#include "objString.h"

namespace aria {

ObjClass::ObjClass(ObjString *_name, GC *_gc)
    : name{_name}
    , methods{_gc}
    , superKlass{nullptr}
    , initMethod{nullptr}
{
    type = objType::CLASS;
}

ObjClass::~ObjClass() = default;

String ObjClass::toString()
{
    return format("<class {}>", name->chars);
}

String ObjClass::toRawString()
{
    return format("<class {}>", name->chars);
}

void ObjClass::blacken(GC *gc)
{
    name->mark(gc);
    methods.mark();
    if (superKlass != nullptr) {
        superKlass->mark(gc);
    }
    if (initMethod != nullptr) {
        initMethod->mark(gc);
    }
}

ObjClass *newObjClass(ObjString *name, GC *gc)
{
    ObjClass *obj = gc->allocate_object<ObjClass>(name, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjClass\n", toVoidPtr(obj), sizeof(ObjClass));
#endif
    return obj;
}
} // namespace aria