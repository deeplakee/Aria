#ifndef GC_H
#define GC_H
#include <cstring>
#include <type_traits>

#include "common.h"
#include "conStringPool.h"
#include "object/iterator.h"
#include "util/lock.h"
#include "util/util.h"
#include "value/value.h"
#include "value/valueStack.h"

namespace aria {
struct FunctionContext;
class ValueHashTable;
class VM;
class Obj;
class Iterator;
class ConStringPool;
class ObjUpvalue;
class CallFrame;
class ValueStack;

template<typename T>
concept DerivedFromObj = std::is_base_of_v<Obj, T>;

template<typename T>
concept DerivedFromIterator = std::is_base_of_v<Iterator, T>;

template<typename T>
concept Trivial = std::is_trivial_v<T>;

constexpr int GC_HEAP_GROW_FACTOR = 2;

class GC
{
public:
    GC();

    ~GC();

    size_t bytesAllocated;
    size_t nextGC;

    Obj *objList;
    Stack<Obj *> greyStack;
    ConStringPool *conStrPool;

    ValueStack *stack;
    ValueStack *tempVars;
    CallFrame *callFrame;
    int frameCount;
    ObjUpvalue *objUpvalues;
    ValueHashTable *nativeVars;
    ValueHashTable *cachedModules;
    ValueHashTable *globalVarsForRepl;
    FunctionContext *func;

    Lock gcLock;
    bool inGC;

    static constexpr const char *errMsg = "Memory allocation failed\n";
    static constexpr size_t errMsgLen = constexprStrlen(errMsg);

    void bindVM(VM *vm);

    void bindGlobalVarsForRepl(ValueHashTable *table);

    void bindCompiler(FunctionContext *_func);

    bool insertStr(ObjString *obj);

    ObjStringPtr getStr(const char *chars, size_t length, uint32_t hash);

    void addToGrey(Obj *obj);

    bool enableGC() { return gcLock.unlock(); }

    void disableGC() { gcLock.lock(); }

    void cache(Value v) const { tempVars->push(v); }

    void releaseCache(int n) const { tempVars->pop_n(n); }

    void markRoots();

    void traceReferences();

    void sweep();

    void collectGarbage();

    // allocatedObject will be freed with method freeObject
    template<DerivedFromObj T, typename... Args>
    T *allocate_object(Args &&...args)
    {
        bytesAllocated += sizeof(T);
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
        if (bytesAllocated > nextGC) {
            collectGarbage();
        }
        T *obj = nullptr;
        try {
            obj = new T(std::forward<Args>(args)...);
        } catch ([[maybe_unused]] std::bad_alloc &e) {
            fwrite(errMsg, sizeof(char), errMsgLen, stderr);
            exit(1);
        }
        obj->next = objList;
        objList = obj;
        return obj;
    }

    // allocatedObject will be freed with method free_class
    template<DerivedFromIterator T, typename... Args>
    T *allocate_iterator(Args &&...args)
    {
        bytesAllocated += sizeof(T);
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
        if (bytesAllocated > nextGC) {
            collectGarbage();
        }

        T *obj = nullptr;
        try {
            obj = new T(std::forward<Args>(args)...);
        } catch ([[maybe_unused]] std::bad_alloc &e) {
            fwrite(errMsg, sizeof(char), errMsgLen, stderr);
            exit(1);
        }
        return obj;
    }

    template<Trivial T>
    T *reallocate(T *pointer, size_t oldCount, size_t newCount)
    {
        bytesAllocated += (newCount - oldCount) * sizeof(T);
        if (newCount > oldCount) {
#ifdef DEBUG_STRESS_GC
            collectGarbage();
#endif
        }
        if (newCount > oldCount && bytesAllocated > nextGC) {
            collectGarbage();
        }

        if (newCount == 0) {
            delete[] pointer;
            return nullptr;
        }

        T *result = nullptr;
        try {
            result = new T[newCount];
        } catch ([[maybe_unused]] std::bad_alloc &e) {
            fwrite(errMsg, sizeof(char), errMsgLen, stderr);
            exit(1);
        }

        if (pointer) {
            size_t copySize = newCount < oldCount ? newCount : oldCount;
            memcpy(result, pointer, sizeof(T) * copySize);
            delete[] pointer;
        }

        return result;
    }

    template<Trivial T>
    T *allocate_array(size_t count)
    {
        return reallocate<T>(nullptr, 0, count);
    }

    template<Trivial T>
    T *grow_array(T *pointer, size_t oldCount, size_t newCount)
    {
        return reallocate<T>(pointer, oldCount, newCount);
    }

    static int grow_capacity(int capacity) { return capacity < 8 ? 8 : capacity * 2; }

    template<Trivial T>
    void free_array(T *pointer, size_t oldCount)
    {
        reallocate<T>(pointer, oldCount, 0);
    }

private:
    void free_iterator(Iterator *pointer);


    void free_object(Obj *obj);

    void free_objects();
};
} // namespace aria

#endif // GC_H
