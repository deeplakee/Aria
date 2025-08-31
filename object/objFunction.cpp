#include "objFunction.h"
#include "chunk/chunk.h"
#include "memory/gc.h"
#include "objString.h"
#include "objUpvalue.h"
#include "value/valueHashTable.h"

namespace aria {

ObjFunction::ObjFunction(FunctionType _funType, GC *_gc)
    : funType{_funType}
    , name{nullptr}
    , arity{0}
    , chunk{new Chunk{_gc}}
    , acceptsVarargs{false}
    , upvalues{nullptr}
    , upvalueCount{0}
{
    type = objType::FUNCTION;
    gc = _gc;
}

ObjFunction::ObjFunction(FunctionType _funType, ValueHashTable *_globalVarTable, GC *_gc)
    : funType{_funType}
    , name{nullptr}
    , arity{0}
    , chunk{new Chunk{_gc, _globalVarTable}}
    , acceptsVarargs{false}
    , upvalues{nullptr}
    , upvalueCount{0}
{
    type = objType::FUNCTION;
    gc = _gc;
}

ObjFunction::ObjFunction(
    FunctionType _funType,
    const String &_name,
    int _arity,
    bool acceptsVarargs,
    GC *_gc,
    ValueHashTable *_globalVarTable)
    : funType{_funType}
    , name{newObjString(_name, _gc)}
    , arity{_arity}
    , chunk{new Chunk{_gc, _globalVarTable}}
    , acceptsVarargs{acceptsVarargs}
    , upvalues{nullptr}
    , upvalueCount{0}
{
    type = objType::FUNCTION;
    gc = _gc;
}

ObjFunction::~ObjFunction() = default;

String ObjFunction::toString()
{
    String f_name = String{name == nullptr ? "anonymous" : name->chars};
    if (funType == FunctionType::SCRIPT) {
        return format("<module {}>", f_name);
    }
    return format("<fn {}>", f_name);
}

String ObjFunction::toRawString()
{
    String f_name = String{name == nullptr ? "anonymous" : name->chars};
    if (funType == FunctionType::SCRIPT) {
        return format("<module {}>", f_name);
    }
    return format("<fn {}>", f_name);
}
void ObjFunction::blacken(GC *gc)
{
    if (name != nullptr) {
        name->mark(gc);
    }
    chunk->constantPool.mark();
    chunk->globalVarTable->mark();
    if (upvalues != nullptr) {
        for (int i = 0; i < upvalueCount; i++) {
            if (upvalues[i] == nullptr) {
                continue;
            }
            upvalues[i]->mark(gc);
        }
    }
}

void ObjFunction::initUpvalues(GC *gc)
{
    if (upvalueCount == 0) {
        return;
    }
    upvalues = gc->allocate_array<ObjUpvalue *>(upvalueCount);
    for (int i = 0; i < upvalueCount; i++) {
        upvalues[i] = nullptr;
    }
}

ObjFunction *newObjFunction(FunctionType funType, GC *gc)
{
    ObjFunction *obj = gc->allocate_object<ObjFunction>(funType, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjFunction\n", toVoidPtr(obj), sizeof(ObjFunction));
#endif
    return obj;
}

ObjFunction *newObjFunction(FunctionType funType, ValueHashTable *globalVarTable, GC *gc)
{
    ObjFunction *obj = gc->allocate_object<ObjFunction>(funType, globalVarTable, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjFunction\n", toVoidPtr(obj), sizeof(ObjFunction));
#endif
    return obj;
}

ObjFunction *newObjFunction(
    FunctionType funType,
    const String &name,
    int arity,
    bool acceptsVarargs,
    ValueHashTable *globalVarTable,
    GC *gc)
{
    ObjFunction *obj
        = gc->allocate_object<ObjFunction>(funType, name, arity, acceptsVarargs, gc, globalVarTable);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjFunction\n", toVoidPtr(obj), sizeof(ObjFunction));
#endif
    return obj;
}

} // namespace aria
