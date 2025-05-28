#ifndef OBJNATIVEFN_H
#define OBJNATIVEFN_H

#include "functionType.h"
#include "object.h"

namespace aria {
class ValueHashTable;
class GC;
class ObjString;

class ObjNativeFn : public Obj
{
public:
    ObjNativeFn() = delete;
    ObjNativeFn(
        FunctionType _funType,
        NativeFn _function,
        ObjString *_name,
        int _arity,
        bool acceptsVarargs,
        GC *_gc);
    ~ObjNativeFn() override;

    String toString() override;
    String toRawString() override;
    void blacken(GC *gc) override;

    FunctionType funType;
    NativeFn function;
    ObjString *name;
    int arity;
    bool acceptsVarargs;
};

inline bool is_objNativeFn(Value value)
{
    return isObjType(value, objType::NATIVE_FN);
}

inline ObjNativeFn *as_objNativeFn(Value value)
{
    return dynamic_cast<ObjNativeFn *>(as_obj(value));
}

inline NativeFn as_NativeFn(Value value)
{
    return dynamic_cast<ObjNativeFn *>(as_obj(value))->function;
}

ObjNativeFn *newObjNativeFn(
    FunctionType funType, NativeFn fn, ObjString *name, int arity, bool acceptsVarargs, GC *gc);

void bindBuiltinMethod(
    ValueHashTable *methodTable,
    const char *name,
    NativeFn fn,
    int arity,
    GC *gc,
    bool acceptsVarargs = false);

} // namespace aria

#endif //OBJNATIVEFN_H