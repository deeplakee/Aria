#include "objMap.h"

#include "memory/gc.h"
#include "objIterator.h"
#include "objList.h"
#include "objNativeFn.h"
#include "objString.h"
#include "runtime/runtimeException.h"
#include "value/valueStack.h"

namespace aria {

/////////////////////////////////////////////////////////////////////
///       The functions following are the built-in method of list

static Value builtin_insert(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    objmap->map->insert(args[0], args[1]);
    return nil_val;
}

static Value builtin_get(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    Value value = nil_val;
    objmap->map->get(args[0], value);
    return value;
}

static Value builtin_remove(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    objmap->map->remove(args[0]);
    return nil_val;
}

static Value builtin_has(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    bool result = objmap->map->has(args[0]);
    return bool_val(result);
}

static Value builtin_size(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    return number_val(objmap->map->size());
}

static Value builtin_empty(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    return bool_val(objmap->map->size() == 0);
}

static Value builtin_clear(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    objmap->map->clear();
    return nil_val;
}

static Value builtin_keys(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    ObjList *newList = newObjList(objmap->map->genKeysArray(gc), gc);
    return obj_val(newList);
}

static Value builtin_values(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    ObjList *newList = newObjList(objmap->map->genValuesArray(gc), gc);
    return obj_val(newList);
}

Value builtin_pairs(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    ObjList *newList = newObjList(gc);
    gc->cache(obj_val(newList));
    int capacity = objmap->map->capacity;
    KVPair *entry = objmap->map->entry;
    for (int i = 0; i < capacity; i++) {
        if (entry[i].isEmpty)
            continue;
        ObjList *list = objmap->map->createPairList(i, gc);
        gc->cache(obj_val(list));
        newList->list->write(obj_val(list));
        gc->releaseCache(1);
    }
    gc->releaseCache(1);
    return obj_val(newList);
}

static Value builtin_equals(int argCount, Value *args, GC *gc)
{
    ObjMap *objmap = as_objMap(args[-1]);
    if (!is_objMap(args[0])) {
        throw RuntimeException("Argument must be a map");
    }
    bool result = objmap->map->equals(as_objMap(args[0])->map);
    return bool_val(result);
}

/////////////////////////////////////////////////////////////////////

ValueHashTable *ObjMap::builtinMethod = nullptr;

ObjMap::ObjMap(GC *_gc)
    : map{new ValueHashTable{_gc}}
{
    type = objType::MAP;
    gc = _gc;
}

ObjMap::ObjMap(ValueHashTable *_map, GC *_gc)
    : map{_map}
{
    type = objType::MAP;
    gc = _gc;
}

ObjMap::ObjMap(int count, ValueStack *stack, GC *_gc)
    : map{new ValueHashTable{_gc}}
{
    type = objType::MAP;
    gc = _gc;
    int i = count * 2 - 1;
    while (i >= 0) {
        map->insert(stack->peek(i), stack->peek(i - 1));
        i -= 2;
    }
    stack->pop_n(count * 2);
}
ObjMap::~ObjMap() = default;

bool ObjMap::getAttribute(ObjString *name, Value &value)
{
    return builtinMethod->get(obj_val(name), value);
}

bool ObjMap::getElement(Value k, Value &v, GC *gc)
{
    return map->get(k, v);
}
bool ObjMap::storeElement(Value k, Value v)
{
    map->insert(k, v);
    return true;
}

Value ObjMap::createIterator(GC *gc)
{
    return obj_val(newObjIterator(this, gc));
}

Value ObjMap::copy(GC *gc)
{
    ObjMap *newObj = newObjMap(gc);
    gc->cache(obj_val(newObj));
    newObj->map->copy(map);
    gc->releaseCache(1);
    return obj_val(newObj);
}

String ObjMap::toString()
{
    ValueStack stack;
    stack.push(obj_val(this));
    return map->toString(&stack);
}

String ObjMap::toRawString()
{
    ValueStack stack;
    stack.push(obj_val(this));
    return map->toRawString(&stack);
}

String ObjMap::toString(ValueStack *outer)
{
    if (outer->isExist(obj_val(this))) {
        return "{...}";
    }
    outer->push(obj_val(this));
    return map->toString(outer);
}

String ObjMap::toRawString(ValueStack *outer)
{
    if (outer->isExist(obj_val(this))) {
        return "{...}";
    }
    outer->push(obj_val(this));
    return map->toRawString(outer);
}

void ObjMap::blacken(GC *gc)
{
    map->mark();
}
void ObjMap::init(GC *gc)
{
    builtinMethod = new ValueHashTable{gc};
    bindBuiltinMethod(builtinMethod, "insert", builtin_insert, 2, gc);
    bindBuiltinMethod(builtinMethod, "get", builtin_get, 1, gc);
    bindBuiltinMethod(builtinMethod, "remove", builtin_remove, 1, gc);
    bindBuiltinMethod(builtinMethod, "has", builtin_has, 1, gc);
    bindBuiltinMethod(builtinMethod, "size", builtin_size, 0, gc);
    bindBuiltinMethod(builtinMethod, "empty", builtin_empty, 0, gc);
    bindBuiltinMethod(builtinMethod, "clear", builtin_clear, 0, gc);
    bindBuiltinMethod(builtinMethod, "keys", builtin_keys, 0, gc);
    bindBuiltinMethod(builtinMethod, "values", builtin_values, 0, gc);
    bindBuiltinMethod(builtinMethod, "pairs", builtin_pairs, 0, gc);
    bindBuiltinMethod(builtinMethod, "equals", builtin_equals, 1, gc);
}

ObjMap *newObjMap(GC *gc)
{
    ObjMap *obj = gc->allocate_object<ObjMap>(gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjMap\n", toVoidPtr(obj), sizeof(ObjMap));
#endif
    return obj;
}
ObjMap *newObjMap(int count, ValueStack *stack, GC *gc)
{
    if (count == 0) {
        return newObjMap(gc);
    }
    ObjMap *obj = gc->allocate_object<ObjMap>(count, stack, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjMap\n", toVoidPtr(obj), sizeof(ObjMap));
#endif
    return obj;
}
} // namespace aria
