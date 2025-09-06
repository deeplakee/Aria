#ifndef OBJSTRING_H
#define OBJSTRING_H

#include "object.h"

namespace aria {
class ValueHashTable;

class ObjString : public Obj
{
public:
    ObjString(char *_chars, size_t _length, uint32_t _hash, bool _ownChars, GC *_gc);
    ObjString(const char *_chars, size_t _length, uint32_t _hash, GC *_gc);
    ~ObjString() override;

    bool getAttribute(ObjString *name, Value &value) override;
    bool getElement(Value k, Value &v, GC *gc) override;
    Value createIterator(GC *gc) override;
    String toString() override;
    String toRawString() override;
    bool addable(Value right) override;
    void blacken(GC *gc) override {}

    char *C_str() { return isLong ? longChars : shortChars; }
    [[nodiscard]] const char *C_str_ref() const { return isLong ? longChars : shortChars; }

    static constexpr auto SHORT_CAPACITY = 15;
    static ValueHashTable *builtinMethod;
    static void init(GC *gc);

    bool isLong;
    union {
        char shortChars[SHORT_CAPACITY + 1];
        char *longChars;
    };

    size_t length;
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
    return as_objString(value)->C_str();
}

ObjString *newObjString(const String &str, GC *gc);

ObjString *newObjString(const char *str, GC *gc);

ObjString *newObjString(char ch, GC *gc);

ObjString *newObjString(char *str, size_t length, GC *gc);

ObjString *newObjStringFromRawCStr(char *str, size_t length, GC *gc);

ObjString *concatenateString(const ObjString *a, const ObjString *b, GC *gc);

} // namespace aria

#endif //OBJSTRING_H
