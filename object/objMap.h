#ifndef OBJMAP_H
#define OBJMAP_H

#include "object.h"
#include "value/valueHashTable.h"

namespace aria {
class ValueStack;

class ObjMap:public Obj
{
    public:
    ObjMap()=delete;
    explicit ObjMap(GC *gc);
    ObjMap(ValueHashTable *_map,GC *gc);
    ObjMap(int count, ValueStack *stack, GC *gc);
    ~ObjMap() override;

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

    ValueHashTable *map;

    static ValueHashTable* builtinMethod;
    // Initialize the built-in methods of Map
    static void init(GC *gc);
};

inline bool is_objMap(Value value)
{
    return isObjType(value, objType::MAP);
}

inline ObjMap *as_objMap(Value value)
{
    return dynamic_cast<ObjMap *>(as_obj(value));
}

ObjMap *newObjMap(GC *gc);

ObjMap *newObjMap(int count, ValueStack *stack,GC *gc);

}

#endif //OBJMAP_H
