#include "objModule.h"

#include "chunk/chunk.h"
#include "memory/gc.h"
#include "objFunction.h"
#include "objString.h"
#include "value/valueHashTable.h"

namespace aria {

ObjModule::ObjModule(ObjFunction *_module, GC *_gc)
    : name{_module->name}
    , module{_module->chunk->globalVarTable}
{
    type = objType::MODULE;
    gc = _gc;
    _module->chunk->asModule = true;
}

ObjModule::~ObjModule() = default;

bool ObjModule::getAttribute(ObjString *name, Value &value)
{
    return module->get(obj_val(name), value);
}

String ObjModule::toString()
{
    return format("<module {}>", name->toString());
}

String ObjModule::toRawString()
{
    return format("<module {}>", name->toRawString());
}

void ObjModule::blacken(GC *gc)
{
    module->mark();
    name->mark(gc);
}

ObjModule *newObjModule(ObjFunction *module, GC *gc)
{
    ObjModule *obj = gc->allocate_object<ObjModule>(module, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjModule\n", toVoidPtr(obj), sizeof(ObjModule));
#endif
    return obj;
}
} // namespace aria