#ifndef OBJCLASS_H
#define OBJCLASS_H

#include "object.h"
#include "value/valueHashTable.h"

namespace aria {
class ObjFunction;
}
namespace aria {
class ObjString;

class ObjClass : public Obj
{
public:
    ObjClass() = delete;
    ObjClass(ObjString *_name, GC *_gc);
    ~ObjClass() override;

    String toString() override;
    String toRawString() override;
    void blacken(GC *gc) override;

    ObjString *name;
    ValueHashTable methods;
    ObjClass *superKlass;
    ObjFunction * initMethod;
};

inline bool is_objClass(Value value)
{
    return isObjType(value, objType::CLASS);
}

inline ObjClass *as_objClass(Value value)
{
    return dynamic_cast<ObjClass *>(as_obj(value));
}

ObjClass *newObjClass(ObjString *name, GC *gc);
}

#endif //OBJCLASS_H