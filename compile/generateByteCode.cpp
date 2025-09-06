#include "astNode.h"
#include "chunk/chunk.h"
#include "compilingException.h"
#include "functionContext.h"
#include "memory/gc.h"
#include "object/objFunction.h"
#include "object/objString.h"

namespace aria {
void ProgramNode::generateByteCode(FunctionContext *currentFunction)
{
    int lastLine = line;
    for (const auto &decl : declarations) {
        decl->generateByteCode(currentFunction);
        lastLine = decl->line;
    }
    currentFunction->getChunk()->writeCode(opCode::LOAD_NIL, lastLine);
    currentFunction->getChunk()->writeCode(opCode::RETURN, lastLine);
}

void FunDeclNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    FunctionContext *innerFunction = new FunctionContext{
        currentFunction,
        FunctionType::FUNCTION,
        name,
        static_cast<int>(params.size()),
        acceptsVarargs};
    ObjFunction *function = innerFunction->function;

    chunk->loadConstant(obj_val(function), line);
    if (currentFunction->scopeDepth > 0) {
        if (currentFunction->findVariableInSameDepth(name)) {
            throw CompilingException{
                format("at '{}': Already a variable with this name in this scope.", name), line};
        }
        currentFunction->addLocal(name, line);
        currentFunction->markInitialized();
    } else {
        chunk->writeCode(opCode::DEF_GLOBAL, line);
        chunk->writeConstant(obj_val(function->name), line);
    }

    innerFunction->beginScope();

    for (auto &param : params) {
        if (innerFunction->findVariableInSameDepth(param)) {
            throw CompilingException{
                format("at '{}': the parameter has been used before.", param), line};
        }
        innerFunction->addLocal(param, line);
        innerFunction->markInitialized();
    }

    body->generateByteCode(innerFunction);
    innerFunction->getChunk()->writeCode(opCode::LOAD_NIL, endLine);
    innerFunction->getChunk()->writeCode(opCode::RETURN, endLine);

    function->initUpvalues(currentFunction->gc);
    if (function->upvalueCount != 0) {
        chunk->writeCode(opCode::CLOSURE, endLine);
        chunk->writeConstant(obj_val(function), endLine);
        for (int i = 0; i < function->upvalueCount; i++) {
            chunk->writeCode(innerFunction->upvalues[i].isLocal ? 1 : 0, endLine);
            chunk->writeCode_L(innerFunction->upvalues[i].index, endLine);
        }
    }

#ifdef DEBUG_PRINT_COMPILED_CODE
    function->chunk->disassemble(name);
#endif
    delete innerFunction;
}

static ObjString *generateByteCodeForMethod(ASTNode *methodNode, FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    FunDeclNode *method = dynamic_cast<FunDeclNode *>(methodNode);
    String name = method->name;
    List<String> params = method->params;
    int line = method->line;
    int endLine = method->endLine;

    FunctionContext *innerFunction = nullptr;
    if (name == "init") {
        innerFunction = new FunctionContext{
            currentFunction,
            FunctionType::INIT_METHOD,
            name,
            static_cast<int>(params.size()),
            method->acceptsVarargs};
    } else {
        innerFunction = new FunctionContext{
            currentFunction,
            FunctionType::METHOD,
            name,
            static_cast<int>(params.size()),
            method->acceptsVarargs};
    }
    ObjFunction *function = innerFunction->function;

    chunk->loadConstant(obj_val(function), line);

    innerFunction->beginScope();

    for (auto &param : params) {
        if (innerFunction->findVariableInSameDepth(param)) {
            throw CompilingException{
                format("at '{}': the parameter has been used before.", param), line};
        }
        innerFunction->addLocal(param, line);
        innerFunction->markInitialized();
    }

    method->body->generateByteCode(innerFunction);
    if (name == "init") {
        innerFunction->getChunk()->writeCode(opCode::LOAD_LOCAL, endLine);
        innerFunction->getChunk()->writeCode_L(0, endLine);
    } else {
        innerFunction->getChunk()->writeCode(opCode::LOAD_NIL, endLine);
    }
    innerFunction->getChunk()->writeCode(opCode::RETURN, endLine);

    function->initUpvalues(currentFunction->gc);
    if (function->upvalueCount != 0) {
        chunk->writeCode(opCode::CLOSURE, endLine);
        chunk->writeConstant(obj_val(function), endLine);
        for (int i = 0; i < function->upvalueCount; i++) {
            chunk->writeCode(innerFunction->upvalues[i].isLocal ? 1 : 0, endLine);
            chunk->writeCode_L(innerFunction->upvalues[i].index, endLine);
        }
    }

#ifdef DEBUG_PRINT_COMPILED_CODE
    function->chunk->disassemble(name);
#endif
    delete innerFunction;

    return function->name;
}

void ClassDeclNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    ObjString *objName = newObjString(name, currentFunction->gc);
    chunk->writeCode(opCode::MAKE_CLASS, line);
    chunk->writeConstant(obj_val(objName), line);

    ClassContext *thisClass = new ClassContext{currentFunction->currentClass, false};
    currentFunction->currentClass = thisClass;

    if (!superName.empty()) {
        UniquePtr<ASTNode> loadSuperKlassNode = std::make_unique<LoadVarNode>(superName, line);
        loadSuperKlassNode->generateByteCode(currentFunction);
        chunk->writeCode(opCode::INHERIT, line);
        thisClass->hasSuperClass = true;
    }

    bool isLocal = false;
    if (currentFunction->scopeDepth > 0) {
        if (currentFunction->findVariableInSameDepth(name)) {
            throw CompilingException{
                format("at '{}': Already a variable with this name in this scope.", name), line};
        }
        isLocal = true;
        currentFunction->addLocal(name, line);
        currentFunction->markInitialized();
    } else {
        chunk->writeCode(opCode::DEF_GLOBAL, line);
        chunk->writeConstant(obj_val(objName), line);
    }

    if (isLocal) {
        int localSlot = currentFunction->findLocalVariable(name);
        chunk->writeCode(opCode::LOAD_LOCAL, line);
        chunk->writeCode_L(static_cast<uint16_t>(localSlot), line);
    } else {
        chunk->writeCode(opCode::LOAD_GLOBAL, line);
        chunk->writeConstant(obj_val(objName), line);
    }

    for (auto &method : methods) {
        ASTNode *rawMethodNodePtr = method.get();
        int defMethodLine = rawMethodNodePtr->line;
        ObjString *methodName = generateByteCodeForMethod(rawMethodNodePtr, currentFunction);
        if (methodName->length == 4 && memcmp(methodName->C_str_ref(), "init", 4) == 0) {
            chunk->writeCode(opCode::MAKE_INIT_METHOD, defMethodLine);
        } else {
            chunk->writeCode(opCode::MAKE_METHOD, defMethodLine);
            chunk->writeConstant(obj_val(methodName), defMethodLine);
        }
    }

    chunk->writeCode(opCode::POP, endLine);

    currentFunction->currentClass = currentFunction->currentClass->enclosing;
    delete thisClass;
}

void VarDeclNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    for (size_t i = 0; i < nameList.size(); i++) {
        auto &name = nameList[i];
        if (currentFunction->scopeDepth > 0) {
            if (currentFunction->findVariableInSameDepth(name)) {
                throw CompilingException{
                    format("at '{}': Already a variable with this name in this scope.", name), line};
            }
            currentFunction->addLocal(name, line);
            exprList[i]->generateByteCode(currentFunction);
            currentFunction->markInitialized();
        } else if (currentFunction->scopeDepth == 0) {
            exprList[i]->generateByteCode(currentFunction);
            chunk->writeCode(opCode::DEF_GLOBAL, line);
            Value objStr = obj_val(newObjString(name, currentFunction->gc));
            chunk->writeConstant(objStr, line);
        } else {
            throw CompilingException{"declare variable in an illegal scope.", line};
        }
    }
}

void BlockNode::generateByteCode(FunctionContext *currentFunction)
{
    currentFunction->beginScope();

    for (const auto &decl : declarations) {
        decl->generateByteCode(currentFunction);
    }

    auto exitOps = currentFunction->endScope();
    for (const auto &[fst, snd] : exitOps.encoded) {
        if (fst == opCode::POP) {
            currentFunction->getChunk()->genPopInstructions(snd, endLine);
        } else if (fst == opCode::CLOSE_UPVALUE) {
            currentFunction->getChunk()->writeCode(opCode::CLOSE_UPVALUE, endLine);
        } else {
            throw CompilingException{"unknown exit op", endLine};
        }
    }
}

void PrintStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    expr->generateByteCode(currentFunction);
    currentFunction->getChunk()->writeCode(opCode::PRINT, line);
}

void ImportStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    ObjString *input = newObjString(inputStr, currentFunction->gc);
    ObjString *module = newObjString(moduleName, currentFunction->gc);
    chunk->writeCode(opCode::IMPORT, line);
    chunk->writeConstant(obj_val(input), line);
    chunk->writeConstant(obj_val(module), line);
    chunk->writeCode(opCode::POP, line);
}

void IfStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    condition->generateByteCode(currentFunction);
    int falseJump = chunk->writeJumpBwd(opCode::JUMP_FALSE, line);
    body->generateByteCode(currentFunction);
    if (elseBody != nullptr) {
        int endJump = chunk->writeJumpBwd(opCode::JUMP_BWD, elseBody->line);
        chunk->backPatch(falseJump);
        elseBody->generateByteCode(currentFunction);
        chunk->backPatch(endJump);
    } else {
        chunk->backPatch(falseJump);
    }
}

void WhileStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    currentFunction->loopDepths.emplace(currentFunction->scopeDepth);
    currentFunction->loopBreaks.emplace();
    currentFunction->loopContinues.emplace();

    int loopStart = chunk->count;
    condition->generateByteCode(currentFunction);
    int exitJump = chunk->writeJumpBwd(opCode::JUMP_FALSE, line);
    body->generateByteCode(currentFunction);
    chunk->writeJumpFwd(loopStart, endLine);
    chunk->backPatch(exitJump);

    // backpatch undefined jumps which are in the break statements and the continue statements
    for (auto &breaks = currentFunction->loopBreaks.top(); const auto breakJump : breaks) {
        chunk->backPatch(breakJump);
    }
    for (auto &continues = currentFunction->loopContinues.top();
         const auto continueJump : continues) {
        chunk->backPatch(loopStart, continueJump);
    }

    currentFunction->loopDepths.pop();
    currentFunction->loopBreaks.pop();
    currentFunction->loopContinues.pop();
}

void ForStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    currentFunction->beginScope();

    if (varInit != nullptr) {
        varInit->generateByteCode(currentFunction);
    }

    currentFunction->loopDepths.emplace(currentFunction->scopeDepth);
    currentFunction->loopBreaks.emplace();
    currentFunction->loopContinues.emplace();

    int loopStart = chunk->count;

    int exitJump = -1;
    if (condition != nullptr) {
        condition->generateByteCode(currentFunction);
        exitJump = chunk->writeJumpBwd(opCode::JUMP_FALSE, line);
    }

    body->generateByteCode(currentFunction);

    int incrementStart = chunk->count;
    if (increment != nullptr) {
        increment->generateByteCode(currentFunction);
        chunk->writeCode(opCode::POP, line);
    }

    chunk->writeJumpFwd(loopStart, line);
    if (exitJump != -1) {
        chunk->backPatch(exitJump);
    }

    // backpatch undefined jumps which are in the break statements and the continue statements
    for (auto &breaks = currentFunction->loopBreaks.top(); const auto breakJump : breaks) {
        chunk->backPatch(breakJump);
    }
    for (auto &continues = currentFunction->loopContinues.top();
         const auto continueJump : continues) {
        chunk->backPatch(incrementStart, continueJump);
    }

    currentFunction->loopDepths.pop();
    currentFunction->loopBreaks.pop();
    currentFunction->loopContinues.pop();

    auto exitOps = currentFunction->endScope();
    for (const auto &[fst, snd] : exitOps.encoded) {
        if (fst == opCode::POP) {
            chunk->genPopInstructions(snd, endLine);
        } else if (fst == opCode::CLOSE_UPVALUE) {
            chunk->writeCode(opCode::CLOSE_UPVALUE, endLine);
        } else {
            throw CompilingException{"unknown exit op", endLine};
        }
    }
}

void ForInStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    String iterName = "__" + loopVarName + "__ITER__";
    Chunk *chunk = currentFunction->getChunk();
    currentFunction->beginScope();

    // init iterator and loopVar
    currentFunction->addLocal(iterName, line);
    iterExpr->generateByteCode(currentFunction);
    chunk->writeCode(opCode::GET_ITER, line);
    currentFunction->markInitialized();
    currentFunction->addLocal(loopVarName, line);
    chunk->writeCode(opCode::LOAD_NIL, line);
    currentFunction->markInitialized();

    currentFunction->loopDepths.emplace(currentFunction->scopeDepth);
    currentFunction->loopBreaks.emplace();
    currentFunction->loopContinues.emplace();

    int loopStart = chunk->count;

    // get iterator.hasNext
    int iterSlot = currentFunction->findLocalVariable(iterName);
    chunk->writeCode(opCode::LOAD_LOCAL, line);
    chunk->writeCode_L(static_cast<uint16_t>(iterSlot), line);
    chunk->writeCode(opCode::ITER_HAS_NEXT, line);
    int exitJump = chunk->writeJumpBwd(opCode::JUMP_FALSE, line);

    // store loopVar
    int loopVarSlot = currentFunction->findLocalVariable(loopVarName);
    chunk->writeCode(opCode::LOAD_LOCAL, line);
    chunk->writeCode_L(static_cast<uint16_t>(iterSlot), line);
    chunk->writeCode(opCode::ITER_GET_NEXT, line);
    chunk->writeCode(opCode::STORE_LOCAL, line);
    chunk->writeCode_L(static_cast<uint16_t>(loopVarSlot), line);
    chunk->writeCode(opCode::POP, line);

    body->generateByteCode(currentFunction);

    int incrementStart = chunk->count;

    chunk->writeJumpFwd(loopStart, line);
    if (exitJump != -1) {
        chunk->backPatch(exitJump);
    }

    // backpatch undefined jumps which are in the break statements and the continue statements
    for (auto &breaks = currentFunction->loopBreaks.top(); const auto breakJump : breaks) {
        chunk->backPatch(breakJump);
    }
    for (auto &continues = currentFunction->loopContinues.top();
         const auto continueJump : continues) {
        chunk->backPatch(incrementStart, continueJump);
    }

    currentFunction->loopDepths.pop();
    currentFunction->loopBreaks.pop();
    currentFunction->loopContinues.pop();

    auto exitOps = currentFunction->endScope();
    for (const auto &[fst, snd] : exitOps.encoded) {
        if (fst == opCode::POP) {
            chunk->genPopInstructions(snd, endLine);
        } else if (fst == opCode::CLOSE_UPVALUE) {
            chunk->writeCode(opCode::CLOSE_UPVALUE, endLine);
        } else {
            throw CompilingException{"unknown exit op", endLine};
        }
    }
}

void TryCatchStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();

    int begin = chunk->writeJumpBwd(opCode::BEGIN_TRY, line);

    tryBody->generateByteCode(currentFunction);

    chunk->writeCode(opCode::END_TRY, line);
    int exitJump = chunk->writeJumpBwd(opCode::JUMP_BWD, catchLine);

    chunk->backPatch(begin);

    currentFunction->beginScope();

    currentFunction->addLocal(exceptionName, catchLine);
    currentFunction->markInitialized();

    catchBody->generateByteCode(currentFunction);

    auto exitOps = currentFunction->endScope();
    for (const auto &[fst, snd] : exitOps.encoded) {
        if (fst == opCode::POP) {
            currentFunction->getChunk()->genPopInstructions(snd, endLine);
        } else if (fst == opCode::CLOSE_UPVALUE) {
            currentFunction->getChunk()->writeCode(opCode::CLOSE_UPVALUE, endLine);
        } else {
            throw CompilingException{"unknown exit op", endLine};
        }
    }

    chunk->backPatch(exitJump);
}

void ThrowStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    e->generateByteCode(currentFunction);
    currentFunction->getChunk()->writeCode(opCode::THROW, line);
}

void BreakStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    if (currentFunction->loopDepths.empty()) {
        throw CompilingException{"break statement should inside a loop", line};
    }
    int popCount = currentFunction->popLocalsOnControlFlow();
    chunk->genPopInstructions(popCount, line);

    int breakJump = chunk->writeJumpBwd(opCode::JUMP_BWD, line);
    currentFunction->loopBreaks.top().push_back(breakJump);
}

void ContinueStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    if (currentFunction->loopDepths.empty()) {
        throw CompilingException{"continue statement should inside a loop", line};
    }
    int popCount = currentFunction->popLocalsOnControlFlow();
    chunk->genPopInstructions(popCount, line);

    // opCode::JUMP_BWD may be modified
    // because in forStmt the continue jumps backward,but in whileStmt the continue jumps forward.
    int continueJump = chunk->writeJumpBwd(opCode::JUMP_BWD, line);
    currentFunction->loopContinues.top().push_back(continueJump);
}

void ReturnStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    if (currentFunction->type == FunctionType::SCRIPT) {
        throw CompilingException{"Can't return from top-level code.", line};
    }
    if (currentFunction->type == FunctionType::INIT_METHOD) {
        throw CompilingException{"Can't return a value from an initializer.", line};
    }
    expr->generateByteCode(currentFunction);
    currentFunction->getChunk()->writeCode(opCode::RETURN, line);
}

void ExprStmtNode::generateByteCode(FunctionContext *currentFunction)
{
    expr->generateByteCode(currentFunction);
    currentFunction->getChunk()->writeCode(opCode::POP, line);
}

void AssignNode::generateByteCode(FunctionContext *currentFunction)
{
    expr->generateByteCode(currentFunction);
    left->generateByteCode(currentFunction);
}

void BinaryOpNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    if (op == TokenType::AND) {
        // measure short circuit operator and
        left->generateByteCode(currentFunction);
        int endJump = chunk->writeJumpBwd(opCode::JUMP_FALSE_NOPOP, line);
        chunk->writeCode(opCode::POP, line);
        right->generateByteCode(currentFunction);
        chunk->backPatch(endJump);
    } else if (op == TokenType::OR) {
        // measure short circuit operator or
        left->generateByteCode(currentFunction);
        int endJump = chunk->writeJumpBwd(opCode::JUMP_TRUE_NOPOP, line);
        chunk->writeCode(opCode::POP, line);
        right->generateByteCode(currentFunction);
        chunk->backPatch(endJump);
    } else {
        left->generateByteCode(currentFunction);
        right->generateByteCode(currentFunction);
        chunk->writeCode(tokenToOpCodeInBinary[op], line);
    }
}

void UnaryOpNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    operand->generateByteCode(currentFunction);
    switch (op) {
    case TokenType::MINUS:
        chunk->writeCode(opCode::NEGATE, line);
        break;
    case TokenType::NOT:
        chunk->writeCode(opCode::NOT, line);
        break;
    default:
        throw CompilingException{"Unknown unary operation.", line};
    }
}

void CallNode::generateByteCode(FunctionContext *currentFunction)
{
    callee->generateByteCode(currentFunction);
    for (const auto &arg : args) {
        arg->generateByteCode(currentFunction);
    }
    currentFunction->getChunk()->writeCode(opCode::CALL, line);
    currentFunction->getChunk()->writeCode(static_cast<uint8_t>(args.size()), line);
}

void InvokeMethodNode::generateByteCode(FunctionContext *currentFunction)
{
    instance->generateByteCode(currentFunction);
    Chunk *chunk = currentFunction->getChunk();
    for (const auto &arg : args) {
        arg->generateByteCode(currentFunction);
    }

    Value objStr = obj_val(newObjString(methodName, currentFunction->gc));
    chunk->writeCode(opCode::INVOKE_METHOD, line);
    chunk->writeConstant(objStr, line);
    chunk->writeCode(static_cast<uint8_t>(args.size()), line);
}
void LoadSuperMethodNode::generateByteCode(FunctionContext *currentFunction)
{
    if (currentFunction->currentClass == nullptr) {
        throw CompilingException{"Can't use 'super' outside of a class.", line};
    }
    if (!currentFunction->currentClass->hasSuperClass) {
        throw CompilingException{"Can't use 'super' in a class with no superclass.", line};
    }
    UniquePtr<ASTNode> loadThis = std::make_unique<LoadVarNode>("this", line);
    loadThis->generateByteCode(currentFunction);
    Value objStr = obj_val(newObjString(methodName, currentFunction->gc));
    Chunk *chunk = currentFunction->getChunk();
    chunk->writeCode(opCode::LOAD_SUPER_METHOD, line);
    chunk->writeConstant(objStr, line);
}

void LoadPropertyNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();

    instance->generateByteCode(currentFunction);

    Value objStr = obj_val(newObjString(propertyName, currentFunction->gc));
    chunk->writeCode(opCode::LOAD_PROPERTY, line);
    chunk->writeConstant(objStr, line);
}

void StorePropertyNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();

    instance->generateByteCode(currentFunction);

    Value objStr = obj_val(newObjString(propertyName, currentFunction->gc));
    chunk->writeCode(opCode::STORE_PROPERTY, line);
    chunk->writeConstant(objStr, line);
}

void LoadSubscrNode::generateByteCode(FunctionContext *currentFunction)
{
    instance->generateByteCode(currentFunction);
    index->generateByteCode(currentFunction);
    currentFunction->getChunk()->writeCode(opCode::LOAD_SUBSCR, line);
}

void StoreSubscrNode::generateByteCode(FunctionContext *currentFunction)
{
    instance->generateByteCode(currentFunction);
    index->generateByteCode(currentFunction);
    currentFunction->getChunk()->writeCode(opCode::STORE_SUBSCR, line);
}

void LoadVarNode::generateByteCode(FunctionContext *currentFunction)
{
    if (varName == "this" && currentFunction->currentClass == nullptr) {
        throw CompilingException{"Can't use 'this' outside of a class.", line};
    }
    Chunk *chunk = currentFunction->getChunk();
    int localSlot = currentFunction->findLocalVariable(varName);
    if (localSlot >= 0) {
        // local variable: write the index of local variable in the vm stack
        chunk->writeCode(opCode::LOAD_LOCAL, line);
        chunk->writeCode_L(static_cast<uint16_t>(localSlot), line);
    } else if (localSlot == -2) {
        int upvalueSlot = currentFunction->findUpvalueVariable(varName);
        if (upvalueSlot >= 0) {
            // upvalue variable
            chunk->writeCode(opCode::LOAD_UPVALUE, line);
            chunk->writeCode_L(static_cast<uint16_t>(upvalueSlot), line);
        } else if (upvalueSlot == -1) {
            // global variable: write the key of global variable in chunk globalVariable HashTable
            Value objStr = obj_val(newObjString(varName, currentFunction->gc));
            chunk->writeCode(opCode::LOAD_GLOBAL, line);
            chunk->writeConstant(objStr, line);
        } else {
            throw CompilingException{"Too many closure variables in function.", line};
        }
    } else {
        const auto errMsg
            = format("at '{}': Can't read local variable in its own initializer.", varName);
        throw CompilingException{errMsg, line};
    }
}

void StoreVarNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    int localSlot = currentFunction->findLocalVariable(varName);
    if (localSlot >= 0) {
        // local variable: write the index of local variable in the vm stack
        chunk->writeCode(opCode::STORE_LOCAL, line);
        chunk->writeCode_L(static_cast<uint16_t>(localSlot), line);
    } else if (localSlot == -2) {
        int upvalueSlot = currentFunction->findUpvalueVariable(varName);
        if (upvalueSlot >= 0) {
            // upvalue variable
            chunk->writeCode(opCode::STORE_UPVALUE, line);
            chunk->writeCode_L(static_cast<uint16_t>(upvalueSlot), line);
        } else if (upvalueSlot == -1) {
            // global variable: write the key of global variable in chunk globalVariable HashTable
            Value objStr = obj_val(newObjString(varName, currentFunction->gc));
            chunk->writeCode(opCode::STORE_GLOBAL, line);
            chunk->writeConstant(objStr, line);
        } else {
            throw CompilingException{"Too many closure variables in function.", line};
        }
    } else {
        const auto errMsg
            = format("at '{}': Can't read local variable in its own initializer.", varName);
        throw CompilingException{errMsg, line};
    }
}

void NumberNode::generateByteCode(FunctionContext *currentFunction)
{
    currentFunction->getChunk()->loadConstant(number_val(value), line);
}

void TrueNode::generateByteCode(FunctionContext *currentFunction)
{
    currentFunction->getChunk()->writeCode(opCode::LOAD_TRUE, line);
}

void FalseNode::generateByteCode(FunctionContext *currentFunction)
{
    currentFunction->getChunk()->writeCode(opCode::LOAD_FALSE, line);
}

void NilNode::generateByteCode(FunctionContext *currentFunction)
{
    currentFunction->getChunk()->writeCode(opCode::LOAD_NIL, line);
}

void StringNode::generateByteCode(FunctionContext *currentFunction)
{
    Chunk *chunk = currentFunction->getChunk();
    Value objStr = obj_val(newObjString(text, currentFunction->gc));
    chunk->loadConstant(objStr, line);
}

void ListNode::generateByteCode(FunctionContext *currentFunction)
{
    for (auto &expr : exprs) {
        expr->generateByteCode(currentFunction);
    }
    uint16_t listSize = exprs.size();
    currentFunction->getChunk()->writeCode(opCode::MAKE_LIST, line);
    currentFunction->getChunk()->writeCode_L(listSize, line);
}

void MapNode::generateByteCode(FunctionContext *currentFunction)
{
    for (auto &expr : kvPairs) {
        expr->generateByteCode(currentFunction);
    }
    uint16_t listSize = kvPairs.size() / 2;
    currentFunction->getChunk()->writeCode(opCode::MAKE_MAP, line);
    currentFunction->getChunk()->writeCode_L(listSize, line);
}

void ErrorNode::generateByteCode(FunctionContext *currentFunction) {}
} // namespace aria