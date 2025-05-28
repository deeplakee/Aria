#ifndef OBJBOUNDMETHOD_H
#define OBJBOUNDMETHOD_H

#include "object.h"
#include "functionType.h"

namespace aria {
class ObjNativeFn;
class ObjFunction;

class ObjBoundMethod : public Obj
{
public:
    ObjBoundMethod() = delete;
    ObjBoundMethod(Value _receiver, ObjFunction *_method, GC *_gc);
    ObjBoundMethod(Value _receiver, ObjNativeFn *_method, GC *_gc);
    ~ObjBoundMethod() override;

    String toString() override;
    String toRawString() override;
    void blacken(GC *gc) override;

    Value receiver;
    BoundMethodType methodType;
    ObjFunction *method;
    ObjNativeFn *native_method;
};

inline bool is_objBoundMethod(Value value)
{
    return isObjType(value, objType::BOUND_METHOD);
}

inline ObjBoundMethod *as_objBoundMethod(Value value)
{
    return dynamic_cast<ObjBoundMethod *>(as_obj(value));
}

ObjBoundMethod *newObjBoundMethod(Value receiver, ObjFunction *method, GC *gc);

ObjBoundMethod *newObjBoundMethod(Value receiver, ObjNativeFn *method, GC *gc);

} // namespace aria

#endif //OBJBOUNDMETHOD_H
