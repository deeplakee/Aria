#ifndef OBJSTRING_H
#define OBJSTRING_H

#include "object.h"

namespace aria {
class ValueHashTable;

class ObjString : public Obj
{
public:
    ObjString(char *_chars, size_t _length, uint32_t _hash, GC *_gc);
    ~ObjString() override;

    bool getAttribute(ObjString *name, Value &value) override;
    bool getElement(Value k, Value &v, GC *gc) override;
    Value createIterator(GC *gc) override;
    String toString() override;
    String toRawString() override;
    void blacken(GC *gc) override {}

    char *chars;
    size_t length;

    static ValueHashTable *builtinMethod;
    static void init(GC *gc);
};

inline bool is_objString(Value value)
{
    return isObjType(value, objType::STRING);
}

inline ObjString *as_objString(Value value)
{
    return dynamic_cast<ObjString *>(as_obj(value));
}

inline char *as_c_string(Value value)
{
    return as_objString(value)->chars;
}

ObjString *newObjString(const String &str, GC *gc);

ObjString *newObjString(const char* str, GC *gc);

ObjString *newObjString(char ch, GC *gc);

ObjString *newObjString(char *c_str, size_t length, GC *gc);

ObjString *newObjStringFromRawCStr(char *c_str, size_t length, GC *gc);

ObjString *concatenateString(const ObjString *a, const ObjString *b, GC *gc);

ObjString *concatenateString(const char *a, const char *b, GC *gc);

} // namespace aria

#endif //OBJSTRING_H
