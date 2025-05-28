#include "functionContext.h"

#include "chunk/chunk.h"
#include "compile/compilingException.h"
#include "memory/gc.h"

namespace aria {
FunctionContext::FunctionContext(FunctionType _type, GC *_gc)
    : gc{_gc}
    , enclosing{nullptr}
    , currentClass{nullptr}
    , type{_type}
    , scopeDepth{0}
{
    function = nullptr;

    // the function object is hosted in the garbage collector
    function = newObjFunction(_type, gc);
    gc->bindCompiler(this);
    // the first slot is for the VM’s own internal use
    locals.emplace_back();

    if (_type == FunctionType::METHOD || _type == FunctionType::INIT_METHOD) {
        locals[0].name = "this";
    }
}

FunctionContext::FunctionContext(FunctionType _type, GC *_gc, ValueHashTable *globalVarTable)
    : gc{_gc}
    , enclosing{nullptr}
    , currentClass{nullptr}
    , type{_type}
    , scopeDepth{0}
{
    function = nullptr;

    // the function object is hosted in the garbage collector
    function = newObjFunction(_type, globalVarTable, _gc);
    gc->bindCompiler(this);
    // the first slot is for the VM’s own internal use
    locals.emplace_back();

    if (_type == FunctionType::METHOD || _type == FunctionType::INIT_METHOD) {
        locals[0].name = "this";
    }
}

FunctionContext::FunctionContext(
    FunctionContext *other, FunctionType _type, const String &name, int arity, bool acceptsVarargs)
{
    this->enclosing = other;
    this->currentClass = other->currentClass;

    gc = other->gc;
    ValueHashTable *globalVarTable = other->function->chunk->globalVarTable;

    this->function = newObjFunction(_type, name, arity, acceptsVarargs, globalVarTable, gc);
    gc->bindCompiler(this);
    this->type = _type;
    this->locals.emplace_back();
    this->scopeDepth = 0;

    if (_type == FunctionType::METHOD || _type == FunctionType::INIT_METHOD) {
        locals[0].name = "this";
    }
}

FunctionContext::~FunctionContext()
{
    gc->bindCompiler(enclosing);
};

void FunctionContext::beginScope()
{
    scopeDepth++;
}

RLEList<opCode> FunctionContext::endScope()
{
    RLEList<opCode> ops;
    scopeDepth--;
    while (!locals.empty()) {
        if (locals.back().depth <= scopeDepth) {
            break;
        }
        if (locals.back().isCaptured) {
            ops.insert(opCode::CLOSE_UPVALUE);
        } else {
            ops.insert(opCode::POP);
        }
        locals.pop_back();
    }

    return ops;
}

int FunctionContext::popLocalsOnControlFlow()
{
    int loopDepth = loopDepths.top();
    int count = 0;
    for (auto i = locals.size(); i > 0; i--) {
        if (locals[i - 1].depth <= loopDepth) {
            break;
        }
        count++;
    }
    return count;
}

bool FunctionContext::addLocal(String name, int line)
{
    if (locals.size() == UINT16_MAX) {
        throw CompilingException{
            "Too many local variables have been declared within the current scope", line};
    }
    locals.emplace_back(std::move(name), -1, false);
    return true;
}

void FunctionContext::markInitialized()
{
    if (scopeDepth == 0) {
        return;
    }
    locals.back().depth = scopeDepth;
}

bool FunctionContext::findVariableInSameDepth(StringView name) const
{
    for (int i = static_cast<int>(locals.size() - 1); i >= 0; i--) {
        const Local &local = locals[i];
        if (local.depth != -1 && local.depth < scopeDepth) {
            return false;
        }
        if (name == local.name) {
            return true;
        }
    }
    return false;
}

int FunctionContext::findLocalVariable(StringView name)
{
    for (int i = static_cast<int>(locals.size() - 1); i >= 0; i--) {
        Local *local = &locals[i];
        if (name == local->name) {
            if (local->depth == -1) {
                return -1;
            }
            return i;
        }
    }
    return -2;
}

/**
 *
 * @param name variable name
*  @return >=0: Index of upvalue variable in this scope
 * @return -1 : No upper value found
 * @return -2 : Too many closure variables in function.
 */
int FunctionContext::findUpvalueVariable(StringView name)
{
    if (enclosing == nullptr) {
        return -1;
    }

    int localIndex = enclosing->findLocalVariable(name);
    if (localIndex >= 0) {
        enclosing->locals[localIndex].isCaptured = true;
        return addUpvalue(static_cast<uint8_t>(localIndex), true);
    }

    int upvalueIndex = enclosing->findUpvalueVariable(name);
    if (upvalueIndex >= 0) {
        return addUpvalue(static_cast<uint8_t>(upvalueIndex), false);
    }

    return upvalueIndex;
}

/**
 *
 * @param index
 * @param isLocal
 * @return  -2 :Too many closure variables in function.
 * @return >=0 :The index of upvalue in current scope
 */
int FunctionContext::addUpvalue(uint16_t index, bool isLocal)
{
    int upvalueCount = function->upvalueCount;
    for (int i = 0; i < upvalues.size(); i++) {
        Upvalue *upvalue = &upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }

    if (upvalueCount == UINT16_MAX) {
        return -2;
    }

    upvalues.emplace_back(index, isLocal);
    return function->upvalueCount++;
}

Chunk *FunctionContext::getChunk() const
{
    return function->chunk;
}

void FunctionContext::mark()
{
    FunctionContext *current = this;
    while (current != nullptr) {
        current->function->mark(gc);
        current = current->enclosing;
    }
}
} // namespace aria