#ifndef OBJITERATOR_H
#define OBJITERATOR_H

#include "iterator.h"
#include "object.h"

namespace aria {
class ValueHashTable;
class ObjList;
class ObjMap;

class ObjIterator : public Obj
{
public:
    ObjIterator() = delete;
    explicit ObjIterator(Iterator *iter, GC *_gc);
    ~ObjIterator() override;

    String toString() override;
    String toRawString() override;
    void blacken(GC *gc) override;
    bool getAttribute(ObjString *name, Value &value) override;

    Iterator *iter;

    static ValueHashTable *builtinMethod;
    // Initialize the built-in methods of iterator
    static void init(GC *gc);
};

inline bool is_objIterator(Value value)
{
    return isObjType(value, objType::ITERATOR);
}

inline ObjIterator *as_objIterator(Value value)
{
    return dynamic_cast<ObjIterator *>(as_obj(value));
}

ObjIterator *newObjIterator(Iterator *iter, GC *gc);

ObjIterator *newObjIterator(ObjList *list, GC *gc);

ObjIterator *newObjIterator(ObjMap *map, GC *gc);

ObjIterator *newObjIterator(ObjString *str, GC *gc);

} // namespace aria

#endif //OBJITERATOR_H
