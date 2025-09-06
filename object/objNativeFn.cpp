#include "objNativeFn.h"

#include "memory/gc.h"
#include "objString.h"
#include "value/valueHashTable.h"

namespace aria {

ObjNativeFn::ObjNativeFn(
    FunctionType _funType,
    NativeFn _function,
    ObjString *_name,
    int _arity,
    bool acceptsVarargs,
    GC *_gc)
    : funType{_funType}
    , function{_function}
    , name{_name}
    , arity{_arity}
    , acceptsVarargs{acceptsVarargs}
{
    type = objType::NATIVE_FN;
    gc = _gc;
}

ObjNativeFn::~ObjNativeFn() = default;

String ObjNativeFn::toString()
{
    if (name == nullptr)
        return String{"<nativeFn anonymous>"};
    return format("<nativeFn {}>", name->C_str_ref());
}
String ObjNativeFn::toRawString()
{
    if (name == nullptr)
        return String{"<nativeFn anonymous>"};
    return format("<nativeFn {}>", name->C_str_ref());
}

void ObjNativeFn::blacken(GC *gc)
{
    if (name != nullptr) {
        name->mark(gc);
    }
}

ObjNativeFn *newObjNativeFn(
    FunctionType funType, NativeFn fn, ObjString *name, int arity, bool acceptsVarargs, GC *gc)
{
    ObjNativeFn *obj
        = gc->allocate_object<ObjNativeFn>(funType, fn, name, arity, acceptsVarargs, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjNativeFn\n", toVoidPtr(obj), sizeof(ObjNativeFn));
#endif
    return obj;
}

void bindBuiltinMethod(
    ValueHashTable *methodTable,
    const char *name,
    const NativeFn fn,
    const int arity,
    GC *gc,
    bool acceptsVarargs)
{
    const Value name_val = obj_val(newObjString(name, gc));
    gc->cache(name_val);
    const Value method_val = obj_val(
        newObjNativeFn(FunctionType::METHOD, fn, as_objString(name_val), arity, acceptsVarargs, gc));
    gc->cache(method_val);
    methodTable->insert(name_val, method_val);
    gc->releaseCache(2);
}

} // namespace aria
