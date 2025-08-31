#include "objList.h"

#include "util/util.h"
#include "memory/gc.h"
#include "objIterator.h"
#include "objNativeFn.h"
#include "objString.h"
#include "runtime/runtimeException.h"
#include "value/valueHashTable.h"
#include "value/valueStack.h"

namespace aria {

/////////////////////////////////////////////////////////////////////
///       The functions following are the built-in method of list

static Value builtin_append(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    objlist->list->write(args[0]);
    return nil_val;
}

static Value builtin_extend(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    if (!is_objList(args[0])) {
        throw RuntimeException("Argument must be a list");
    }
    objlist->list->extend(as_objList(args[0])->list);
    return nil_val;
}

static Value builtin_size(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    return number_val(objlist->list->size());
}

static Value builtin_empty(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    return bool_val(objlist->list->size() == 0);
}

static Value builtin_pop(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    objlist->list->pop();
    return nil_val;
}

static Value builtin_insert(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    if (!is_number(args[0])) {
        throw RuntimeException("argument must be a number");
    }
    int index = static_cast<int>(as_number(args[0]));
    if (index != as_number(args[0])) {
        throw RuntimeException("argument must be a integer");
    }
    Value value = args[1];
    bool result = objlist->list->insert(index, value);
    return bool_val(result);
}

static Value builtin_remove(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    if (!is_number(args[0])) {
        throw RuntimeException("argument must be a number");
    }
    int index = static_cast<int>(as_number(args[0]));
    if (index != as_number(args[0])) {
        throw RuntimeException("argument must be a integer");
    }
    if (index < 0 || index >= objlist->list->size()) {
        throw RuntimeException("Index out of range");
    }
    bool result = objlist->list->remove(index);
    return bool_val(result);
}

static Value builtin_at(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    if (unlikely(!is_number(args[0]))) {
        throw RuntimeException("argument must be a number");
    }
    int index = static_cast<int>(as_number(args[0]));
    if (index != as_number(args[0])) {
        throw RuntimeException("argument must be a integer");
    }
    if (unlikely(index < 0 || index >= objlist->list->size())) {
        throw RuntimeException("index out of range");
    }
    return (*objlist->list)[index];
}

static Value builtin_clear(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    objlist->list->clear();
    return nil_val;
}

static Value builtin_slice(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);

    if (!is_number(args[0])) {
        throw RuntimeException("argument must be a number");
    }
    int startIndex = static_cast<int>(as_number(args[0]));
    if (startIndex != as_number(args[0])) {
        throw RuntimeException("argument must be a integer");
    }
    if (startIndex < 0 || startIndex >= objlist->list->size()) {
        throw RuntimeException("start index out of range");
    }

    if (!is_number(args[1])) {
        throw RuntimeException("argument must be a number");
    }
    int endIndex = static_cast<int>(as_number(args[1]));
    if (endIndex != as_number(args[1])) {
        throw RuntimeException("argument must be a integer");
    }
    if (endIndex < 0 || endIndex >= objlist->list->size()) {
        throw RuntimeException("end index out of range");
    }

    if (endIndex < startIndex) {
        throw RuntimeException("start index should be smaller than end index");
    }
    ObjList *newList = newObjList(startIndex, endIndex, objlist->list, gc);
    return obj_val(newList);
}

static Value builtin_reverse(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    objlist->list->reverse();
    return nil_val;
}

static Value builtin_equals(int argCount, Value *args, GC *gc)
{
    ObjList *objlist = as_objList(args[-1]);
    if (!is_objList(args[0])) {
        throw RuntimeException("Argument must be a list");
    }
    bool result = objlist->list->equals(as_objList(args[0])->list);
    return bool_val(result);
}

////////////////////////////////////////////////////////////////////

ValueHashTable *ObjList::builtinMethod = nullptr;

ObjList::ObjList(GC *_gc)
    : list{new ValueArray{_gc}}
{
    type = objType::LIST;
    gc = _gc;
}

ObjList::ObjList(int count, ValueStack *stack, GC *_gc)
    : list{new ValueArray{_gc}}
{
    type = objType::LIST;
    gc = _gc;
    int i = count - 1;
    while (i >= 0) {
        list->write(stack->peek(i));
        --i;
    }
    stack->pop_n(count);
}
ObjList::ObjList(int begin, int end, ValueArray *other, GC *_gc)
    : list{new ValueArray{begin, end, other, _gc}}
{
    type = objType::LIST;
    gc = _gc;
}
ObjList::ObjList(ValueArray *_list, GC *_gc)
    : list{_list}
{
    type = objType::LIST;
    gc = _gc;
}

ObjList::~ObjList() = default;

bool ObjList::getAttribute(ObjString *name, Value &value)
{
    return builtinMethod->get(obj_val(name), value);
}

bool ObjList::getElement(Value k, Value &v, GC *gc)
{
    if (unlikely(!is_number(k))) {
        throw RuntimeException("index of list must be a number");
    }
    int index = static_cast<int>(as_number(k));
    if (index != as_number(k)) {
        throw RuntimeException("index of list must be a integer");
    }
    if (unlikely(index < 0 || index >= list->size())) {
        throw RuntimeException("index out of range");
    }
    v = (*list)[index];
    return true;
}

bool ObjList::storeElement(Value k, Value v)
{
    if (unlikely(!is_number(k))) {
        throw RuntimeException("index of list must be a number");
    }
    int index = static_cast<int>(as_number(k));
    if (index != as_number(k)) {
        throw RuntimeException("index of list must be a integer");
    }
    if (unlikely(index < 0 || index >= list->size())) {
        throw RuntimeException("index out of range");
    }
    (*list)[index] = v;
    return true;
}

Value ObjList::createIterator(GC *gc)
{
    return obj_val(newObjIterator(this, gc));
}

Value ObjList::copy(GC *gc)
{
    ObjList *newObj = newObjList(gc);
    gc->cache(obj_val(newObj));
    newObj->list->copy(list);
    gc->releaseCache(1);
    return obj_val(newObj);
}

String ObjList::toString()
{
    ValueStack stack;
    stack.push(obj_val(this));
    return list->toString(&stack);
}

String ObjList::toRawString()
{
    ValueStack stack;
    stack.push(obj_val(this));
    return list->toRawString(&stack);
}

String ObjList::toString(ValueStack *outer)
{
    if (outer->isExist(obj_val(this))) {
        return "[...]";
    }
    outer->push(obj_val(this));
    return list->toString(outer);
}

String ObjList::toRawString(ValueStack *outer)
{
    if (outer->isExist(obj_val(this))) {
        return "[...]";
    }
    outer->push(obj_val(this));
    return list->toRawString(outer);
}

void ObjList::blacken(GC *gc)
{
    list->mark();
}

void ObjList::init(GC *gc)
{
    builtinMethod = new ValueHashTable{gc};
    bindBuiltinMethod(builtinMethod, "append", builtin_append, 1, gc);
    bindBuiltinMethod(builtinMethod, "extend", builtin_extend, 1, gc);
    bindBuiltinMethod(builtinMethod, "size", builtin_size, 0, gc);
    bindBuiltinMethod(builtinMethod, "empty", builtin_empty, 0, gc);
    bindBuiltinMethod(builtinMethod, "pop", builtin_pop, 0, gc);
    bindBuiltinMethod(builtinMethod, "insert", builtin_insert, 2, gc);
    bindBuiltinMethod(builtinMethod, "remove", builtin_remove, 1, gc);
    bindBuiltinMethod(builtinMethod, "at", builtin_at, 1, gc);
    bindBuiltinMethod(builtinMethod, "clear", builtin_clear, 0, gc);
    bindBuiltinMethod(builtinMethod, "slice", builtin_slice, 2, gc);
    bindBuiltinMethod(builtinMethod, "reverse", builtin_reverse, 0, gc);
    bindBuiltinMethod(builtinMethod, "equals", builtin_equals, 1, gc);
}

ObjList *newObjList(GC *gc)
{
    ObjList *obj = gc->allocate_object<ObjList>(gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjList\n", toVoidPtr(obj), sizeof(ObjList));
#endif
    return obj;
}

ObjList *newObjList(int count, ValueStack *stack, GC *gc)
{
    if (count == 0) {
        return newObjList(gc);
    }
    ObjList *obj = gc->allocate_object<ObjList>(count, stack, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjList\n", toVoidPtr(obj), sizeof(ObjList));
#endif
    return obj;
}

ObjList *newObjList(ValueArray *list, GC *gc)
{
    ObjList *obj = gc->allocate_object<ObjList>(list, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjList\n", toVoidPtr(obj), sizeof(ObjList));
#endif
    return obj;
}

ObjList *newObjList(int begin, int end, ValueArray *other, GC *gc)
{
    if (end == begin) {
        return newObjList(gc);
    }
    ObjList *obj = gc->allocate_object<ObjList>(begin, end, other, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjList\n", toVoidPtr(obj), sizeof(ObjList));
#endif
    return obj;
}

} // namespace aria
