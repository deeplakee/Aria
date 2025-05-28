#include "objBoundMethod.h"

#include "memory/gc.h"
#include "objFunction.h"
#include "objNativeFn.h"

namespace aria {

ObjBoundMethod::ObjBoundMethod(Value _receiver, ObjFunction *_method, GC *_gc)
    : receiver{_receiver}
    , methodType{BoundMethodType::FUNCTION}
    , method{_method}
    , native_method{nullptr}
{
    type = objType::BOUND_METHOD;
}
ObjBoundMethod::ObjBoundMethod(Value _receiver, ObjNativeFn *_method, GC *_gc)
    : receiver{_receiver}
    , methodType{BoundMethodType::NATIVE_FN}
    , method{nullptr}
    , native_method{_method}
{
    type = objType::BOUND_METHOD;
}

ObjBoundMethod::~ObjBoundMethod() = default;

String ObjBoundMethod::toString()
{
    String objStr = valueTypeString(receiver);
    String methodStr;
    if (methodType == BoundMethodType::FUNCTION) {
        methodStr = method->toString();
    } else if (methodType == BoundMethodType::NATIVE_FN) {
        methodStr = native_method->toString();
    } else {
        methodStr = "unknownMethod";
    }
    return format("<{} {}>", objStr, methodStr);
}

String ObjBoundMethod::toRawString()
{
    String objStr = valueTypeString(receiver);
    String methodStr;
    if (methodType == BoundMethodType::FUNCTION) {
        methodStr = method->toString();
    } else if (methodType == BoundMethodType::NATIVE_FN) {
        methodStr = native_method->toString();
    } else {
        methodStr = "unknownMethod";
    }
    return format("<{} {}>", objStr, methodStr);
}

void ObjBoundMethod::blacken(GC *gc)
{
    markValue(receiver, gc);
    if (method != nullptr) {
        method->mark(gc);
    }
    if (native_method != nullptr) {
        native_method->mark(gc);
    }
}

ObjBoundMethod *newObjBoundMethod(Value receiver, ObjFunction *method, GC *gc)
{
    ObjBoundMethod *obj = gc->allocate_object<ObjBoundMethod>(receiver, method, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjBoundMethod\n", toVoidPtr(obj), sizeof(ObjBoundMethod));
#endif
    return obj;
}

ObjBoundMethod *newObjBoundMethod(Value receiver, ObjNativeFn *method, GC *gc)
{
    ObjBoundMethod *obj = gc->allocate_object<ObjBoundMethod>(receiver, method, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjBoundMethod\n", toVoidPtr(obj), sizeof(ObjBoundMethod));
#endif
    return obj;
}
} // namespace aria