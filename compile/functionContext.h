#ifndef SCOPE_H
#define SCOPE_H

#include "chunk/code.h"
#include "common.h"
#include "object/objFunction.h"
#include "util/rleList.h"

namespace aria {
struct Local
{
    String name;
    int depth;
    bool isCaptured;

    Local()
        : name{}
        , depth{0}
        , isCaptured{false}
    {}

    Local(String _name, int _depth, bool _isCaptured)
        : name{std::move(_name)}
        , depth{_depth}
        , isCaptured{_isCaptured}
    {}
};

struct Upvalue
{
    uint16_t index{0};
    bool isLocal{false};

    Upvalue(uint16_t _index, bool _isLocal)
        : index{_index}
        , isLocal{_isLocal}
    {}
};

struct ClassContext
{
    ClassContext()
        : enclosing{nullptr}
        , hasSuperClass{false}
    {}

    ClassContext(ClassContext *_enclosing, bool _hasSuperClass)
        : enclosing{_enclosing}
        , hasSuperClass{_hasSuperClass}
    {}

    ClassContext *enclosing;
    bool hasSuperClass;
};

struct FunctionContext
{
    FunctionContext *enclosing;

    ClassContext *currentClass;

    GC *gc;
    ObjFunction *function;
    FunctionType type;

    List<Local> locals;
    List<Upvalue> upvalues;
    int scopeDepth;

    Stack<int> loopDepths;
    Stack<List<int>> loopBreaks;
    Stack<List<int>> loopContinues;

    FunctionContext(FunctionType _type, GC *_gc);

    FunctionContext(FunctionType _type, GC *_gc, ValueHashTable *globalVarTable);

    FunctionContext(
        FunctionContext *other,
        FunctionType _type,
        const String &name,
        int arity,
        bool acceptsVarargs);

    ~FunctionContext();

    void beginScope();

    /**
     * @return opcodes which are needed to clear the stack
     * (remember to add these opcodes into chunk!)
     */
    RLEList<opCode> endScope();

    int popLocalsOnControlFlow();

    bool addLocal(String name, int line);

    void markInitialized();

    bool findVariableInSameDepth(StringView name) const;

    /**
     * @return -2: not local variable
     * @return -1 :read local variable in its own initializer.
     * @return >=0 : index of local variable in stack
     */
    int findLocalVariable(StringView name);

    int findUpvalueVariable(StringView name);

    int addUpvalue(uint16_t index, bool isLocal);

    Chunk *getChunk() const;

    void mark();
};
} // namespace aria

#endif //SCOPE_H
