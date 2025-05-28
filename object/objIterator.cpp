#include "objIterator.h"

#include "memory/gc.h"
#include "objNativeFn.h"
#include "objString.h"
#include "value/valueHashTable.h"

namespace aria {

/////////////////////////////////////////////////////////////////////
///       The functions following are the built-in method of iterator

static Value builtin_hasNext(int argCount, Value *args, GC *gc)
{
    ObjIterator *obj = as_objIterator(args[-1]);
    return bool_val(obj->iter->hasNext());
}

static Value builtin_next(int argCount, Value *args, GC *gc)
{
    ObjIterator *obj = as_objIterator(args[-1]);
    return obj->iter->next(gc);
}

/////////////////////////////////////////////////////////////////////

ValueHashTable *ObjIterator::builtinMethod = nullptr;

ObjIterator::ObjIterator(Iterator *iter)
    : iter{iter}
{
    type = objType::ITERATOR;
}

ObjIterator::~ObjIterator() = default;

String ObjIterator::toString()
{
    return format("<iter {}>",iter->typeString());
}

String ObjIterator::toRawString()
{
    return format("<iter {}>",iter->typeString());
}

void ObjIterator::blacken(GC *gc)
{
    iter->blacken(gc);
}
bool ObjIterator::getAttribute(ObjString *name, Value &value)
{
    return builtinMethod->get(obj_val(name), value);
}

void ObjIterator::init(GC *gc)
{
    builtinMethod = new ValueHashTable{gc};
    bindBuiltinMethod(builtinMethod, "hasNext", builtin_hasNext, 0, gc);
    bindBuiltinMethod(builtinMethod, "next", builtin_next, 0, gc);
}

ObjIterator *newObjIterator(Iterator *iter, GC *gc)
{
    ObjIterator *obj = gc->allocate_object<ObjIterator>(iter);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjIterator\n", toVoidPtr(obj), sizeof(ObjIterator));
#endif
    return obj;
}

ObjIterator *newObjIterator(ObjList *list, GC *gc)
{
    auto iter = gc->allocate_iterator<ListIterator>(list);
    ObjIterator *obj = gc->allocate_object<ObjIterator>(iter);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjIterator\n", toVoidPtr(obj), sizeof(ObjIterator));
#endif
    return obj;
}

ObjIterator *newObjIterator(ObjMap *map, GC *gc)
{
    auto iter = gc->allocate_iterator<MapIterator>(map);
    ObjIterator *obj = gc->allocate_object<ObjIterator>(iter);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjIterator\n", toVoidPtr(obj), sizeof(ObjIterator));
#endif
    return obj;
}

ObjIterator *newObjIterator(ObjString *str, GC *gc)
{
    auto iter = gc->allocate_iterator<StringIterator>(str);
    ObjIterator *obj = gc->allocate_object<ObjIterator>(iter);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjIterator\n", toVoidPtr(obj), sizeof(ObjIterator));
#endif
    return obj;
}

} // namespace aria