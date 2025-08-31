#ifndef OBJLIST_H
#define OBJLIST_H

#include "object.h"
#include "value/valueArray.h"

namespace aria {
class ValueHashTable;

class ValueStack;

class ObjList : public Obj
{
public:
    ObjList() = delete;
    explicit ObjList(GC *_gc);
    ObjList(int count, ValueStack *stack, GC *_gc);
    ObjList(int begin, int end, ValueArray *other, GC *_gc);
    ObjList(ValueArray *_list, GC *_gc);
    ~ObjList() override;

    bool getAttribute(ObjString *name, Value &value) override;
    bool getElement(Value k, Value &v, GC *gc) override;
    bool storeElement(Value k, Value v) override;
    Value createIterator(GC *gc) override;
    Value copy(GC *gc) override;
    String toString() override;
    String toRawString() override;
    String toString(ValueStack *outer) override;
    String toRawString(ValueStack *outer) override;
    void blacken(GC *gc) override;

    ValueArray *list;

    static ValueHashTable *builtinMethod;
    // Initialize the built-in methods of List
    static void init(GC *gc);
};

inline bool is_objList(Value value)
{
    return isObjType(value, objType::LIST);
}

inline ObjList *as_objList(Value value)
{
    return dynamic_cast<ObjList *>(as_obj(value));
}

ObjList *newObjList(GC *gc);

ObjList *newObjList(int count, ValueStack *stack, GC *gc);

ObjList *newObjList(ValueArray *list, GC *gc);

ObjList *newObjList(int begin, int end, ValueArray *other, GC *gc);
} // namespace aria

#endif //OBJLIST_H
