#ifndef OBJUPVALUE_H
#define OBJUPVALUE_H

#include "object.h"

namespace aria {
class ObjUpvalue : public Obj
{
public:
    ObjUpvalue() = delete;
    ObjUpvalue(Value *_location, GC *_gc);
    ~ObjUpvalue() override;

    String toString() override;
    String toRawString() override;
    void blacken(GC *gc) override;

    Value *location;
    Value closed;
    ObjUpvalue *nextUpvalue;
};

inline bool is_objUpvalue(Value value)
{
    return isObjType(value, objType::UPVALUE);
}

inline ObjUpvalue *as_objUpvalue(Value value)
{
    return dynamic_cast<ObjUpvalue *>(as_obj(value));
}

ObjUpvalue *newObjUpvalue(Value *location, GC *gc);
} // namespace aria

#endif //OBJUPVALUE_H