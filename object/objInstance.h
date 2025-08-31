#ifndef OBJINSTANCE_H
#define OBJINSTANCE_H

#include "objClass.h"
#include "object.h"
#include "value/valueHashTable.h"

namespace aria {
class ObjClass;

class ObjInstance : public Obj
{
public:
    ObjInstance() = delete;
    ObjInstance(ObjClass *_klass, GC *_gc);
    ~ObjInstance() override;

    bool getAttribute(ObjString *name, Value &value) override;
    Value copy(GC *gc) override;
    String toString() override;
    String toRawString() override;
    bool add(Value right) override;
    void blacken(GC *gc) override;

    ObjClass *klass;
    ValueHashTable fields;
};

inline bool is_objInstance(Value value)
{
    return isObjType(value, objType::INSTANCE);
}

inline ObjInstance *as_objInstance(Value value)
{
    return dynamic_cast<ObjInstance *>(as_obj(value));
}

ObjInstance *newObjInstance(ObjClass *klass, GC *gc);
}

#endif //OBJINSTANCE_H