#ifndef OBJFUNCTION_H
#define OBJFUNCTION_H

#include "functionType.h"
#include "object.h"

namespace aria {
class ValueHashTable;
class Chunk;
class ObjString;
class ObjUpvalue;

class ObjFunction : public Obj
{
public:
    ObjFunction() = delete;
    ObjFunction(FunctionType _funType, GC *_gc);
    ObjFunction(FunctionType _funType, ValueHashTable *_globalVarTable, GC *_gc);
    ObjFunction(
        FunctionType _funType,
        const String &_name,
        int _arity,
        bool acceptsVarargs,
        GC *_gc,
        ValueHashTable *_globalVarTable);
    ~ObjFunction() override;

    String toString() override;
    String toRawString() override;
    void blacken(GC *gc) override;
    void initUpvalues(GC *gc);

    FunctionType funType;
    ObjString *name;
    int arity;
    Chunk *chunk;
    bool acceptsVarargs;
    ObjUpvalue **upvalues;
    int upvalueCount;
};

inline bool is_objFunction(Value value)
{
    return isObjType(value, objType::FUNCTION);
}

inline ObjFunction *as_objFunction(Value value)
{
    return dynamic_cast<ObjFunction *>(as_obj(value));
}

// for chunk in runfile mode
ObjFunction *newObjFunction(FunctionType funType, GC *gc);

// for chunk in repl mode
ObjFunction *newObjFunction(FunctionType funType, ValueHashTable *globalVarTable, GC *gc);

// for chunk in common fun
ObjFunction *newObjFunction(
    FunctionType funType,
    const String &name,
    int arity,
    bool acceptsVarargs,
    ValueHashTable *globalVarTable,
    GC *gc);
} // namespace aria

#endif //OBJFUNCTION_H