#include "gc.h"
#include "chunk/chunk.h"
#include "compile/functionContext.h"
#include "conStringPool.h"
#include "object/objBoundMethod.h"
#include "object/objClass.h"
#include "object/objFunction.h"
#include "object/objInstance.h"
#include "object/objIterator.h"
#include "object/objList.h"
#include "object/objMap.h"
#include "object/objModule.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "object/objUpvalue.h"
#include "object/object.h"
#include "runtime/vm.h"

namespace aria {
extern const char *objTypeStr[];

GC::GC()
    : bytesAllocated{0}
    , nextGC{1024 * 1024}
    , inGC{false}
    , objList{nullptr}
    , conStrPool{nullptr}
    , stack{nullptr}
    , tempVars{new ValueStack}
    , callFrame{nullptr}
    , frameCount{0}
    , objUpvalues{nullptr}
    , nativeVars{nullptr}
    , cachedModules{nullptr}
    , globalVarsForRepl{nullptr}
    , func{nullptr}
{
    conStrPool = new ConStringPool{this};
}

GC::~GC()
{
    delete tempVars;
    delete conStrPool;
    free_objects();
}

void GC::bindVM(VM *vm)
{
    stack = &vm->stack;
    callFrame = vm->frames;
    frameCount = vm->frameCount;
    objUpvalues = vm->openUpvalues;
    nativeVars = vm->nativeVarTable;
    cachedModules = vm->modules;
    globalVarsForRepl = vm->globalVarTableForRepl;
}

void GC::bindGlobalVarsForRepl(ValueHashTable *table)
{
    globalVarsForRepl = table;
}

void GC::bindCompiler(FunctionContext *_func)
{
    func = _func;
}

bool GC::insertStr(ObjString *obj)
{
    cache(obj_val(obj));
    bool res = conStrPool->insert(obj);
    releaseCache(1);
    return res;
}

ObjStringPtr GC::getStr(const char *chars, size_t length, uint32_t hash)
{
    return conStrPool->get(chars, length, hash);
}

void GC::addToGrey(Obj *obj)
{
    greyStack.push(obj);
}

void GC::markRoots()
{
    stack->mark(this);

    tempVars->mark(this);

    for (int i = 0; i < frameCount; i++) {
        callFrame[i].function->mark(this);
    }

    for (ObjUpvalue *p = objUpvalues; p != nullptr; p = p->nextUpvalue) {
        p->mark(this);
    }

    nativeVars->mark();

    cachedModules->mark();

    if (globalVarsForRepl != nullptr) {
        globalVarsForRepl->mark();
    }

    if (ObjString::builtinMethod != nullptr) {
        ObjString::builtinMethod->mark();
    }

    if (ObjList::builtinMethod != nullptr) {
        ObjList::builtinMethod->mark();
    }

    if (ObjMap::builtinMethod != nullptr) {
        ObjMap::builtinMethod->mark();
    }

    if (ObjIterator::builtinMethod != nullptr) {
        ObjIterator::builtinMethod->mark();
    }
    func->mark();
}

void GC::traceReferences()
{
    while (!greyStack.empty()) {
        Obj *obj = greyStack.top();
        greyStack.pop();
#ifdef DEBUG_LOG_GC
        print("{:p} blacken {}\n", toVoidPtr(obj), obj->toString());
#endif
        obj->blacken(this);
    }
}

void GC::sweep()
{
    Obj *previous = nullptr;
    Obj *object = objList;
    while (object != nullptr) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj *unreached = object;
            object = object->next;
            if (previous != nullptr) {
                previous->next = object;
            } else {
                objList = object;
            }

            free_object(unreached);
        }
    }
}

void GC::collectGarbage()
{
    if (!gcLock.available() || inGC) {
        return;
    }
    inGC = true;
#ifdef DEBUG_LOG_GC
    cout << "-- gc begin" << endl;
    size_t before = bytesAllocated;
#endif

    markRoots();

    traceReferences();

    conStrPool->removeWhite();

    sweep();

    nextGC = bytesAllocated * GC_HEAP_GROW_FACTOR;
#ifdef DEBUG_LOG_GC
    cout << "-- gc end" << endl;
    print(
        "   collected {} bytes (from {} to {}) next at {}\n",
        before - bytesAllocated,
        before,
        bytesAllocated,
        nextGC);
#endif
    inGC = false;
}

void GC::free_iterator(Iterator *pointer)
{
    bytesAllocated -= pointer->getSize();
    delete pointer;
}

void GC::free_object(Obj *obj)
{
#ifdef DEBUG_LOG_GC
    print("{:p} free type {}\n", toVoidPtr(obj), objTypeStr[static_cast<int>(obj->type)]);
#endif

    switch (obj->type) {
    case objType::CLASS: {
        delete dynamic_cast<ObjClass *>(obj);
        bytesAllocated -= sizeof(ObjClass);
        break;
    }
    case objType::INSTANCE: {
        delete dynamic_cast<ObjInstance *>(obj);
        bytesAllocated -= sizeof(ObjInstance);
        break;
    }
    case objType::BOUND_METHOD: {
        delete dynamic_cast<ObjBoundMethod *>(obj);
        bytesAllocated -= sizeof(ObjBoundMethod);
        break;
    }
    case objType::FUNCTION: {
        ObjFunction *objFn = dynamic_cast<ObjFunction *>(obj);
        delete objFn->chunk;
        if (objFn->upvalues != nullptr) {
            free_array<ObjUpvalue *>(objFn->upvalues, objFn->upvalueCount);
        }
        delete objFn;
        bytesAllocated -= sizeof(ObjFunction);
        break;
    }
    case objType::NATIVE_FN: {
        delete dynamic_cast<ObjNativeFn *>(obj);
        bytesAllocated -= sizeof(ObjNativeFn);
        break;
    }
    case objType::UPVALUE: {
        delete dynamic_cast<ObjUpvalue *>(obj);
        bytesAllocated -= sizeof(ObjUpvalue);
        break;
    }
    case objType::STRING: {
        ObjString *objStr = dynamic_cast<ObjString *>(obj);
        if (objStr->chars != nullptr) {
            free_array<char>(objStr->chars, objStr->length + 1);
        }
        bytesAllocated -= sizeof(ObjString);
        break;
    }
    case objType::LIST: {
        ObjList *objList = dynamic_cast<ObjList *>(obj);
        delete objList->list;
        delete objList;
        bytesAllocated -= sizeof(ObjList);
        break;
    }
    case objType::MAP: {
        delete dynamic_cast<ObjMap *>(obj);
        bytesAllocated -= sizeof(ObjMap);
        break;
    }
    case objType::MODULE: {
        ObjModule *objModule = dynamic_cast<ObjModule *>(obj);
        delete objModule->module;
        delete objModule;
        bytesAllocated -= sizeof(ObjModule);
        break;
    }
    case objType::ITERATOR: {
        ObjIterator *objIterator = dynamic_cast<ObjIterator *>(obj);
        free_iterator(objIterator->iter);
        delete objIterator;
        bytesAllocated -= sizeof(ObjIterator);
        break;
    }
    default:;
    }
}

void GC::free_objects()
{
    while (objList != nullptr) {
        Obj *next = objList->next;
        free_object(objList);
        objList = next;
    }
}
} // namespace aria