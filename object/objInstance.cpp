#include "objInstance.h"

#include "memory/gc.h"
#include "objBoundMethod.h"
#include "objFunction.h"
#include "objString.h"
#include "util/util.h"

namespace aria {

ObjInstance::ObjInstance(ObjClass *_klass, GC *_gc)
    : klass{_klass}
    , fields{_gc}
{
    type = objType::INSTANCE;
}

ObjInstance::~ObjInstance() = default;

bool ObjInstance::getAttribute(ObjString *name, Value &value)
{
    if (fields.get(obj_val(name), value)) {
        return true;
    }
    if (klass->methods.get(obj_val(name), value)) {
        return true;
    }
    return false;
}

Value ObjInstance::copy(GC *gc)
{
    ObjInstance *newObj = new ObjInstance(klass, gc);
    gc->cache(obj_val(newObj));
    newObj->fields.copy(&fields);
    gc->releaseCache(1);
    return obj_val(newObj);
}

String ObjInstance::toString()
{
    return format("<{} instance>", klass->name->chars);
}

String ObjInstance::toRawString()
{
    return format("<{} instance>", klass->name->chars);
}

void ObjInstance::blacken(GC *gc)
{
    klass->mark(gc);
    fields.mark();
}

ObjInstance *newObjInstance(ObjClass *klass, GC *gc)
{
    ObjInstance *obj = gc->allocate_object<ObjInstance>(klass, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjInstance\n", toVoidPtr(obj), sizeof(ObjInstance));
#endif
    return obj;
}
} // namespace aria