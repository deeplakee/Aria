#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "value/value.h"

namespace aria {
class GC;
class ObjString;
enum class objType : uint8_t {
    BASE,
    STRING,
    FUNCTION,
    NATIVE_FN,
    UPVALUE,
    CLASS,
    INSTANCE,
    BOUND_METHOD,
    LIST,
    MAP,
    MODULE,
    ITERATOR,
};

inline constexpr const char *overloadingAdd_FunName = "__add__";
inline constexpr const char *overloadingSub_FunName = "__sub__";
inline constexpr const char *overloadingMul_FunName = "__mul__";
inline constexpr const char *overloadingDiv_FunName = "__div__";
inline constexpr const char *overloadingMod_FunName = "__mod__";
inline constexpr const char *overloadingEqual_FunName = "__equal__";

static uint32_t hashObj(Obj *obj)
{
    uint32_t hash = 2166136261U;

    uintptr_t ptr_val = reinterpret_cast<uintptr_t>(obj);
    hash ^= static_cast<uint32_t>(ptr_val & 0xFF);
    hash *= 16777619U;

    ptr_val >>= 8;
    while (ptr_val > 0) {
        hash ^= static_cast<uint32_t>(ptr_val & 0xFF);
        hash *= 16777619U;
        ptr_val >>= 8;
    }
    return hash;
}

class Obj
{
public:
    objType type = objType::BASE;
    bool isMarked = false;
    uint32_t hash = hashObj(this);
    Obj *next = nullptr;
    GC *gc = nullptr;

    virtual ~Obj() = default;

    virtual String toString() { return valueTypeString(obj_val(this)); }

    virtual String toRawString() { return valueTypeString(obj_val(this)); }

    virtual String toString(ValueStack *outer) { return this->toString(); }

    virtual String toRawString(ValueStack *outer) { return this->toRawString(); }

    virtual bool getAttribute(ObjString *name, Value &value) { return false; }

    virtual bool getElement(Value k, Value &v, GC *gc) { return false; };

    virtual bool storeElement(Value k, Value v) { return false; }

    virtual Value createIterator(GC *gc) { return nil_val; }

    virtual Value copy(GC *gc) { return nil_val; }

    virtual bool add(Value right) { return false; }

    virtual void blacken(GC *gc) = 0;

    void mark(GC *gc);
};

inline objType obj_type(const Value value)
{
    return as_obj(value)->type;
}

inline bool isObjType(const Value value, const objType type)
{
    return is_obj(value) && obj_type(value) == type;
}
} // namespace aria

#endif // OBJECT_H