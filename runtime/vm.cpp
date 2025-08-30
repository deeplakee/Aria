#include <cmath>
#include <utility>

#include "chunk/chunk.h"
#include "chunk/disassembler.h"
#include "compile/compiler.h"
#include "memory/gc.h"
#include "native.h"
#include "object/objBoundMethod.h"
#include "object/objClass.h"
#include "object/objInstance.h"
#include "object/objList.h"
#include "object/objMap.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "object/objUpvalue.h"
#include "runtimeException.h"
#include "vm.h"

#include "object/objIterator.h"
#include "object/objModule.h"

namespace aria {
VM::VM()
    : gc{new GC{}}
    , replMode{false}
    , frames{new CallFrame[FRAMES_MAX]}
    , frameCount{0}
    , tryFrames{new TryFrame[FRAMES_MAX]}
    , tryFrameCount{0}
    , globalVarTableForRepl{nullptr}
    , nativeVarTable{new ValueHashTable{gc}}
    , modules{new ValueHashTable{gc}}
    , openUpvalues{nullptr}
    , line{0}
{
    gc->bindVM(this);
    ObjString::init(gc);
    ObjList::init(gc);
    ObjMap::init(gc);
    ObjIterator::init(gc);
    registerNative();
}

VM::~VM()
{
    delete[] frames;
    delete[] tryFrames;
    delete globalVarTableForRepl;
    delete nativeVarTable;
    delete modules;
    delete ObjString::builtinMethod;
    delete ObjList::builtinMethod;
    delete ObjMap::builtinMethod;
    delete ObjIterator::builtinMethod;
    delete gc;
}

interpretResult VM::interpret(String &source)
{
    if (fileName.empty()) {
        fileName = "__tmp_aria_file__";
    }
    if (fileDirectory.empty()) {
         fileDirectory = getCurrentWorkingDirectory();
    }
    Compiler compiler{gc};
    ObjFunction *script = compiler.compile(std::move(source));
    if (script == nullptr) {
        return interpretResult::COMPILE_ERROR;
    }
    stack.push(obj_val(script));
    callFunction(script, 0);

    interpretResult result = interpretResult::SUCCESS;
    try {
        result = run();
    } catch (RuntimeException &e) {
        result = interpretResult::RUNTIME_ERROR;
        cerr << format("[line {}] Error: {}", line, e.what()) << endl;
    }
    return result;
}

interpretResult VM::interpret(String path, String &source)
{
    fileName = path;
    fileDirectory = getFileDirectory(path);
    return interpret(source);
}

interpretResult VM::interpretByLine(String &source)
{
    reset();
    if (!replMode) {
        replMode = true;
        fileName = "__tmp_aria_file__";
        fileDirectory = getCurrentWorkingDirectory();
        globalVarTableForRepl = new ValueHashTable{gc};
        gc->bindGlobalVarsForRepl(globalVarTableForRepl);
    }

    Compiler compiler{gc, globalVarTableForRepl};
    ObjFunction *script = compiler.compile(std::move(source));
    if (script == nullptr) {
        return interpretResult::COMPILE_ERROR;
    }

    stack.push(obj_val(script));
    callFunction(script, 0);
    interpretResult result = interpretResult::SUCCESS;
    try {
        result = run();
    } catch (RuntimeException &e) {
        result = interpretResult::RUNTIME_ERROR;
        cerr << format("[line {}] Error: {}", line, e.what()) << endl;
    }
    return result;
}

void VM::registerNative()
{
    defineNativeFn("clock", 0, Native::_clock_);
    defineNativeFn("random", 0, Native::_random_);
    defineNativeFn("println", 1, Native::_println_, true);
    defineNativeFn("readline", 0, Native::_readline_);
    defineNativeFn("typeof", 1, Native::_typeof_);
    defineNativeFn("str", 1, Native::_str_);
    defineNativeFn("num", 1, Native::_num_);
    defineNativeFn("bool", 1, Native::_bool_);
    defineNativeFn("copy", 1, Native::_copy_);
    defineNativeFn("equals", 2, Native::_equals_);
    defineNativeFn("iterator", 1, Native::_iterator_);
    defineNativeFn("exit", 1, Native::_exit_);
    defineNativeVar("pi", number_val(Native::pi));
    defineNativeVar("e", number_val(Native::e));
    defineNativeVar("_", nil_val);
    defineNativeClass("object");
    for (auto [fst, snd] : Native::getExternNativeFnTable()) {
        defineNativeFn(fst.c_str(), std::get<1>(snd), std::get<0>(snd), std::get<2>(snd));
    }
}

void VM::defineNativeFn(const char *name, int arity, NativeFn function, bool acceptsVarargs)
{
    Value key = obj_val(newObjString(name, gc));
    gc->cache(key);
    Value value = obj_val(newObjNativeFn(
        FunctionType::FUNCTION, function, as_objString(key), arity, acceptsVarargs, gc));
    gc->cache(value);
    nativeVarTable->insert(key, value);
    gc->releaseCache(2);
}

void VM::defineNativeVar(const char *name, Value value)
{
    Value key = obj_val(newObjString(name, gc));
    gc->cache(key);
    gc->cache(value);
    nativeVarTable->insert(key, value);
    gc->releaseCache(2);
}

void VM::defineNativeClass(const char *name)
{
    ObjString *objName = newObjString(name, gc);
    gc->cache(obj_val(objName));
    ObjClass *objKlass = newObjClass(objName, gc);
    gc->cache(obj_val(objKlass));
    nativeVarTable->insert(obj_val(objName), obj_val(objKlass));
    gc->releaseCache(2);
}

bool VM::callValue(Value callee, int argCount)
{
    if (is_obj(callee)) {
        switch (obj_type(callee)) {
        case objType::FUNCTION:
            return callFunction(as_objFunction(callee), argCount);
        case objType::NATIVE_FN:
            return callNativeFn(as_objNativeFn(callee), argCount);
        case objType::CLASS:
            return callNewInstance(as_objClass(callee), argCount);
        case objType::BOUND_METHOD:
            return callBoundMethod(as_objBoundMethod(callee), argCount);
        default:
            break; // Non-callable object type.
        }
    }
    runtimeError(String{"Can only call functions and classes."});
    return false;
}

// the function may invoke following three type function
// 1.property of instance :this may be objFunction(Function) or boundMethod
// 2.method of instance :objFunction(method)
// 3.method of object like string,list and map :nativeFn
bool VM::invoke(ObjString *name, int argCount)
{
    Value receiver = stack.peek(argCount);
    if (is_obj(receiver)) {
        Value value;
        if (as_obj(receiver)->getAttribute(name, value)) {
            if (is_objFunction(value) && as_objFunction(value)->funType == FunctionType::FUNCTION) {
                stack[stack.size() - argCount - 1] = value;
            }
            return callValue(value, argCount);
        }
    }
    runtimeError(
        format("'{}' object has not attribute '{}'.", valueTypeString(receiver), name->chars));
    return false;
}

bool VM::callFunction(ObjFunction *function, int argCount)
{
    if (function->acceptsVarargs && argCount >= function->arity) {
        ObjList *list = newObjList(argCount - function->arity + 1, &stack, gc);
        stack.push(obj_val(list));
    } else if (argCount == function->arity) {
        // pass
    } else {
        runtimeError(format("Expected {} arguments but got {}.", function->arity, argCount));
        return false;
    }

    if (frameCount == FRAMES_MAX) {
        runtimeError(String{"Stack overflow."});
        return false;
    }

    CallFrame *frame = &frames[frameCount++];
    frame->init(function, function->chunk->code, stack.getTop() - function->arity - 1);
    return true;
}

bool VM::callNativeFn(ObjNativeFn *native, int argCount)
{
    if (native->acceptsVarargs && argCount >= native->arity) {
        ObjList *list = newObjList(argCount - native->arity + 1, &stack, gc);
        stack.push(obj_val(list));
    } else if (native->arity == argCount) {
        // pass
    } else {
        runtimeError(format("Expected {} arguments but got {}.", native->arity, argCount));
        return false;
    }
    Value result = native->function(argCount, stack.getTop() - native->arity, gc);
    stack.resize(stack.size() - native->arity - 1);
    stack.push(result);
    return true;
}

bool VM::callNewInstance(ObjClass *klass, int argCount)
{
    stack[stack.size() - argCount - 1] = obj_val(newObjInstance(klass, gc));
    if (klass->initMethod != nullptr) {
        return callFunction(klass->initMethod, argCount);
    }
    if (argCount != 0) {
        runtimeError(format("Expected 0 arguments but got {}.", argCount));
        return false;
    }
    return true;
}

bool VM::callBoundMethod(ObjBoundMethod *bound, int argCount)
{
    if (bound->methodType == BoundMethodType::FUNCTION) {
        if ((bound->method->acceptsVarargs && argCount >= bound->method->arity)
            || argCount == bound->method->arity) {
            stack[stack.size() - bound->method->arity - 1] = bound->receiver;
            return callFunction(bound->method, argCount);
        }
        runtimeError(format("Expected {} arguments but got {}.", bound->method->arity, argCount));
        return false;
    }
    if (bound->methodType == BoundMethodType::NATIVE_FN) {
        if ((bound->native_method->acceptsVarargs && argCount >= bound->native_method->arity)
            || argCount == bound->native_method->arity) {
            stack[stack.size() - bound->native_method->arity - 1] = bound->receiver;
            return callNativeFn(bound->native_method, argCount);
        }
        runtimeError(
            format("Expected {} arguments but got {}.", bound->native_method->arity, argCount));
        return false;
    }
    runtimeError(String{"Unknown bound method type"});
    return false;
}

ObjUpvalue *VM::captureUpvalue(Value *local)
{
    ObjUpvalue *prevUpvalue = nullptr;
    ObjUpvalue *upvalue = openUpvalues;
    while (upvalue != nullptr && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->nextUpvalue;
    }

    if (upvalue != nullptr && upvalue->location == local) {
        return upvalue;
    }
    ObjUpvalue *createdUpvalue = newObjUpvalue(local, gc);
    createdUpvalue->nextUpvalue = upvalue;

    if (prevUpvalue == nullptr) {
        openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->nextUpvalue = createdUpvalue;
    }
    return createdUpvalue;
}

void VM::closeUpvalues(Value *last)
{
    while (openUpvalues != nullptr && openUpvalues->location >= last) {
        ObjUpvalue *upvalue = openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        openUpvalues = upvalue->nextUpvalue;
    }
}

Value VM::bindMethodIfNeeded(Value obj, Value value) const
{
    if (is_objFunction(value)
        && (as_objFunction(value)->funType == FunctionType::METHOD
            || as_objFunction(value)->funType == FunctionType::INIT_METHOD)) {
        ObjFunction *unpackedMethod = as_objFunction(value);
        ObjBoundMethod *boundMethod = newObjBoundMethod(obj, unpackedMethod, gc);
        return obj_val(boundMethod);
    }
    if (is_objNativeFn(value) && (as_objNativeFn(value)->funType == FunctionType::METHOD)) {
        ObjNativeFn *unpackedMethod = as_objNativeFn(value);
        ObjBoundMethod *boundMethod = newObjBoundMethod(obj, unpackedMethod, gc);
        return obj_val(boundMethod);
    }
    return value;
}

ObjModule *VM::getCachedModule(ObjString *path)
{
    Value value;
    if (modules->get(obj_val(path), value)) {
        return as_objModule(value);
    }
    return nullptr;
}

ObjFunction *VM::loadModule(const String &absoluteModulePath, ObjString *moduleName)
{
    String source;
    try {
        source = readFile(absoluteModulePath);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl << std::endl;
        exit(74);
    }
    return Compiler{gc}.compile(std::move(source), moduleName->toString());
}

inline uint8_t VM::read_byte(CallFrame *frame)
{
    return *frame->ip++;
}

inline uint16_t VM::read_word(CallFrame *frame)
{
    frame->ip += 2;
    return static_cast<uint16_t>((frame->ip[-1] << 8) | frame->ip[-2]);
}

inline Value VM::read_constant(CallFrame *frame)
{
    return frame->function->chunk->constantPool[read_word(frame)];
}

inline ObjString *VM::read_objString(CallFrame *frame)
{
    return as_objString(read_constant(frame));
}

interpretResult VM::run()
{
    CallFrame *frame = &frames[frameCount - 1];
    for (;;) {
        // update chunk because chunk maybe changed if the frame changes
        Chunk *chunk = frame->function->chunk;
        line = chunk->lines[static_cast<int>(frame->ip - chunk->code)];
#ifdef DEBUG_TRACE_EXECUTION
        cout << "stack:  ";
        stack.print();
        cout << std::endl;
        Disassembler::disassembleInstruction(*chunk, static_cast<int>(frame->ip - chunk->code));
#endif

        switch (auto instruction = static_cast<opCode>(read_byte(frame))) {
        case opCode::LOAD_CONST: {
            Value constant = read_constant(frame);
            stack.push(constant);
            break;
        }
        case opCode::LOAD_NIL:
            stack.push(nil_val);
            break;
        case opCode::LOAD_TRUE:
            stack.push(bool_val(true));
            break;
        case opCode::LOAD_FALSE:
            stack.push(bool_val(false));
            break;
        case opCode::POP:
            stack.pop();
            break;
        case opCode::POP_N:
            stack.pop_n(read_byte(frame));
            break;
        case opCode::LOAD_LOCAL: {
            uint16_t getSlot = read_word(frame);
            stack.push(frame->slots[getSlot]);
            break;
        }
        case opCode::STORE_LOCAL: {
            uint16_t setSlot = read_word(frame);
            frame->slots[setSlot] = stack.peek();
            break;
        }
        case opCode::LOAD_UPVALUE: {
            uint16_t getSlot = read_word(frame);
            stack.push(*frame->function->upvalues[getSlot]->location);
            break;
        }
        case opCode::STORE_UPVALUE: {
            uint16_t setSlot = read_word(frame);
            *frame->function->upvalues[setSlot]->location = stack.peek();
            break;
        }
        case opCode::CLOSE_UPVALUE: {
            closeUpvalues(stack.getTop() - 1);
            stack.pop();
            break;
        }
        case opCode::DEF_GLOBAL: {
            ObjString *name = read_objString(frame);
            if (unlikely(!chunk->globalVarTable->insert(obj_val(name), stack.peek()))) {
                return runtimeError(format("Existed variable '{}'.", name->chars));
            }
            stack.pop();
            break;
        }
        case opCode::LOAD_GLOBAL: {
            ObjString *objName = read_objString(frame);
            Value name = obj_val(objName);
            Value value = nil_val;
            if (!chunk->globalVarTable->get(name, value)) {
                if (unlikely(!nativeVarTable->get(name, value))) {
                    return runtimeError(format("Undefined variable '{}'.", objName->chars));
                }
            }
            stack.push(value);
            break;
        }
        case opCode::STORE_GLOBAL: {
            ObjString *name = read_objString(frame);
            if (likely(chunk->globalVarTable->insert(obj_val(name), stack.peek()))) {
                chunk->globalVarTable->remove(obj_val(name));
                return runtimeError(format("Undefined variable '{}'.", name->chars));
            }
            break;
        }
        case opCode::LOAD_PROPERTY: {
            if (unlikely(!is_obj(stack.peek()))) {
                return runtimeError("Only objects have properties.");
            }
            Obj *obj = as_obj(stack.peek());
            ObjString *name = read_objString(frame);
            Value value;

            if (!obj->getAttribute(name, value)) {
                return runtimeError(format(
                    "'{}' object has no attribute '{}'.",
                    valueTypeString(stack.peek()),
                    name->chars));
            }

            value = bindMethodIfNeeded(stack.peek(), value);
            stack.pop(); // pop obj.
            stack.push(value);
            break;
        }
        case opCode::STORE_PROPERTY: {
            if (unlikely(!is_objInstance(stack.peek()))) {
                return runtimeError("Only instances have fields.");
            }

            ObjInstance *instance = as_objInstance(stack.pop());
            ObjString *propertyName = read_objString(frame);
            instance->fields.insert(obj_val(propertyName), stack.peek());
            break;
        }
        case opCode::LOAD_SUBSCR: {
            if (unlikely(!is_obj(stack.peek(1)))) {
                return runtimeError("Only objects support subscript access.");
            }
            Obj *obj = as_obj(stack.peek(1));
            Value index = stack.peek();
            Value value;

            if (!obj->getElement(index, value, gc)) {
                return runtimeError(format(
                    "'{}' object does not support subscript access with index '{}'.",
                    obj->toString(),
                    valueString(index)));
            }

            stack.pop_n(2);
            stack.push(value);
            break;
        }
        case opCode::STORE_SUBSCR: {
            if (unlikely(!is_obj(stack.peek(1)))) {
                return runtimeError("Only objects support subscript assignment.");
            }

            Obj *obj = as_obj(stack.peek(1));
            Value index = stack.peek();
            Value value = stack.peek(2);
            if (!obj->storeElement(index, value)) {
                return runtimeError(format(
                    "'{}' object does not support subscript assignment with index '{}'.",
                    valueTypeString(obj_val(obj)),
                    valueString(index)));
            }
            stack.pop_n(2);
            break;
        }
        case opCode::EQUAL: {
            Value b = stack.pop();
            Value a = stack.pop();
            stack.push(bool_val(valuesSame(a, b)));
            break;
        }
        case opCode::NOT_EQUAL: {
            Value b = stack.pop();
            Value a = stack.pop();
            stack.push(bool_val(!valuesSame(a, b)));
            break;
        }
        case opCode::GREATER: {
            if (unlikely(!is_number(stack.peek(0)) || !is_number(stack.peek(1)))) {
                return runtimeError("Operands must be numbers.");
            }
            double b = as_number(stack.pop());
            double a = as_number(stack.pop());
            stack.push(bool_val(a > b));
            break;
        }
        case opCode::GREATER_EQUAL: {
            if (unlikely(!is_number(stack.peek(0)) || !is_number(stack.peek(1)))) {
                return runtimeError("Operands must be numbers.");
            }
            double b = as_number(stack.pop());
            double a = as_number(stack.pop());
            stack.push(bool_val(a >= b));
            break;
        }
        case opCode::LESS: {
            if (unlikely(!is_number(stack.peek(0)) || !is_number(stack.peek(1)))) {
                return runtimeError("Operands must be numbers.");
            }
            double b = as_number(stack.pop());
            double a = as_number(stack.pop());
            stack.push(bool_val(a < b));
            break;
        }
        case opCode::LESS_EQUAL: {
            if (unlikely(!is_number(stack.peek(0)) || !is_number(stack.peek(1)))) {
                return runtimeError("Operands must be numbers.");
            }
            double b = as_number(stack.pop());
            double a = as_number(stack.pop());
            stack.push(bool_val(a <= b));
            break;
        }
        case opCode::ADD: {
            if (is_objString(stack.peek(0)) && is_objString(stack.peek(1))) {
                ObjString *b = as_objString(stack.peek(0));
                ObjString *a = as_objString(stack.peek(1));
                ObjString *result = concatenateString(a, b, gc);
                stack.pop_n(2);
                stack.push(obj_val(result));
            } else if (is_number(stack.peek(0)) && is_number(stack.peek(1))) {
                double b = as_number(stack.pop());
                double a = as_number(stack.pop());
                stack.push(number_val(a + b));
            } else {
                return runtimeError("Operands must be numbers or strings.");
            }
            break;
        }
        case opCode::SUBTRACT: {
            if (unlikely(!is_number(stack.peek(0)) || !is_number(stack.peek(1)))) {
                return runtimeError("Operands must be numbers.");
            }
            double b = as_number(stack.pop());
            double a = as_number(stack.pop());
            stack.push(number_val(a - b));
            break;
        }
        case opCode::MULTIPLY: {
            if (unlikely(!is_number(stack.peek(0)) || !is_number(stack.peek(1)))) {
                return runtimeError("Operands must be numbers.");
            }
            double b = as_number(stack.pop());
            double a = as_number(stack.pop());
            stack.push(number_val(a * b));
            break;
        }
        case opCode::DIVIDE: {
            if (unlikely(!is_number(stack.peek(0)) || !is_number(stack.peek(1)))) {
                return runtimeError("Operands must be numbers.");
            }
            double b = as_number(stack.pop());
            if (b == 0) {
                return runtimeError("Divide by zero.");
            }
            double a = as_number(stack.pop());
            stack.push(number_val(a / b));
            break;
        }
        case opCode::MOD: {
            if (unlikely(!is_number(stack.peek(0)) || !is_number(stack.peek(1)))) {
                return runtimeError("Operands must be numbers.");
            }
            double b = as_number(stack.pop());
            double a = as_number(stack.pop());
            stack.push(number_val(std::fmod(a, b)));
            break;
        }
        case opCode::NOT:
            stack.push(bool_val(isFalsey(stack.pop())));
            break;
        case opCode::NEGATE: {
            if (unlikely(!is_number(stack.peek()))) {
                return runtimeError("Operand must be a number.");
            }
            stack.push(number_val(-as_number(stack.pop())));
            break;
        }
        case opCode::INC: {
            if (!is_number(stack.peek())) {
                return runtimeError("Operand must be a number.");
            }
            stack.push(number_val(as_number(stack.pop()) + 1));
            break;
        }
        case opCode::DEC: {
            if (!is_number(stack.peek())) {
                return runtimeError("Operand must be a number.");
            }
            stack.push(number_val(as_number(stack.pop()) - 1));
            break;
        }
        case opCode::PRINT: {
            cout << valueString(stack.pop()) << '\n';
            break;
        }
        case opCode::NOP:
            break;
        case opCode::JUMP_FWD: {
            const uint16_t offset = read_word(frame);
            frame->ip -= offset;
            break;
        }
        case opCode::JUMP_BWD: {
            const uint16_t offset = read_word(frame);
            frame->ip += offset;
            break;
        }
        case opCode::JUMP_TRUE: {
            const uint16_t offset = read_word(frame);
            if (!isFalsey(stack.pop()))
                frame->ip += offset;
            break;
        }
        case opCode::JUMP_TRUE_NOPOP: {
            const uint16_t offset = read_word(frame);
            if (!isFalsey(stack.peek(0)))
                frame->ip += offset;
            break;
        }
        case opCode::JUMP_FALSE: {
            const uint16_t offset = read_word(frame);
            if (isFalsey(stack.pop()))
                frame->ip += offset;
            break;
        }
        case opCode::JUMP_FALSE_NOPOP: {
            const uint16_t offset = read_word(frame);
            if (isFalsey(stack.peek(0)))
                frame->ip += offset;
            break;
        }
        case opCode::CALL: {
            int argCount = read_byte(frame);
            if (unlikely(!callValue(stack.peek(argCount), argCount))) {
                return interpretResult::RUNTIME_ERROR;
            }
            // If the objFunction is not called,  frame will not change
            frame = &frames[frameCount - 1];
            break;
        }
        case opCode::CLOSURE: {
            Value val_fun = read_constant(frame);
            ObjFunction *function = as_objFunction(val_fun);
            for (int i = 0; i < function->upvalueCount; i++) {
                uint8_t isLocal = read_byte(frame);
                uint16_t index = read_word(frame);
                if (isLocal) {
                    function->upvalues[i] = captureUpvalue(frame->slots + index);
                } else {
                    function->upvalues[i] = frame->function->upvalues[index];
                }
            }
            break;
        }
        case opCode::MAKE_CLASS: {
            ObjClass *objKlass = newObjClass(read_objString(frame), gc);
            Value klass = obj_val(objKlass);
            stack.push(klass);
            break;
        }
        case opCode::INHERIT: {
            if (unlikely(!is_objClass(stack.peek()))) {
                return runtimeError("Superclass must be a class.");
            }
            ObjClass *superKlass = as_objClass(stack.pop());
            ObjClass *klass = as_objClass(stack.peek());
            klass->methods.copy(&superKlass->methods);
            klass->superKlass = superKlass;
            break;
        }
        case opCode::MAKE_METHOD: {
            ObjString *methodName = read_objString(frame);
            Value method = stack.peek(0);
            ObjClass *klass = as_objClass(stack.peek(1));
            klass->methods.insert(obj_val(methodName), method);
            stack.pop();
            break;
        }
        case opCode::MAKE_INIT_METHOD: {
            Value method = stack.pop();
            ObjClass *klass = as_objClass(stack.peek());
            klass->initMethod = as_objFunction(method);
            break;
        }
        case opCode::INVOKE_METHOD: {
            ObjString *method = read_objString(frame);
            int argCount = read_byte(frame);
            if (!invoke(method, argCount)) {
                return interpretResult::RUNTIME_ERROR;
            }
            frame = &frames[frameCount - 1];
            break;
        }
        case opCode::LOAD_SUPER_METHOD: {
            ObjString *methodName = read_objString(frame);
            Value instance = stack.pop();
            ObjClass *superKlass = as_objInstance(instance)->klass->superKlass;
            Value superMethod = nil_val;
            if (unlikely(!superKlass->methods.get(obj_val(methodName), superMethod))) {
                if (methodName->length == 4 && memcmp(methodName->chars, "init", 4) == 0
                    && superKlass->initMethod != nullptr) {
                    superMethod = obj_val(superKlass->initMethod);
                } else {
                    return runtimeError(format(
                        "superclass '{}' has no method '{}",
                        superKlass->toString(),
                        methodName->toString()));
                }
            }
            ObjFunction *unpackedMethod = as_objFunction(superMethod);
            ObjBoundMethod *boundMethod = newObjBoundMethod(instance, unpackedMethod, gc);
            superMethod = obj_val(boundMethod);
            stack.push(superMethod);
            break;
        }
        case opCode::MAKE_LIST: {
            int listSize = read_word(frame);
            ObjList *list = newObjList(listSize, &stack, gc);
            stack.push(obj_val(list));
            break;
        }
        case opCode::MAKE_MAP: {
            int mapSize = read_word(frame);
            ObjMap *map = newObjMap(mapSize, &stack, gc);
            stack.push(obj_val(map));
            break;
        }
        case opCode::IMPORT: {
            ObjString *input = read_objString(frame);
            ObjString *moduleName = read_objString(frame);
            String absoluteModulePath;
            try {
                absoluteModulePath = getAbsoluteModulePath(input->toString(), fileDirectory);
            } catch (const std::exception &e) {
                return runtimeError(format("unable to import module '{}'", moduleName->toString()));
            }
#ifdef DEBUG_PRINT_IMPORT_MODULE_PATH
            cout << "input module: " << input->toString() << endl;
            cout << "running file directory: " << fileDirectory << endl;
            cout << "absolute module path: " << absoluteModulePath << endl;
#endif
            ObjString *path = newObjString(absoluteModulePath, gc);
            gc->cache(obj_val(path));
            ObjModule *module = getCachedModule(path);
            if (module != nullptr) {
                chunk->globalVarTable->insert(obj_val(moduleName), obj_val(module));
            } else {
                ObjFunction *imported = loadModule(absoluteModulePath, moduleName);
                if (imported == nullptr) {
                    return runtimeError(format("Error in import module '{}'", input->chars));
                }
                stack.push(obj_val(imported));
                module = newObjModule(imported, gc);
                gc->cache(obj_val(module));
                modules->insert(obj_val(path), obj_val(module));
                chunk->globalVarTable->insert(obj_val(moduleName), obj_val(module));
                gc->releaseCache(1);
                callFunction(imported, 0);
                frame = &frames[frameCount - 1];
            }
            gc->releaseCache(1);
            break;
        }
        case opCode::GET_ITER: {
            if (!is_obj(stack.peek())) {
                return runtimeError("Expected an iterable object");
            }
            Value iterable = as_obj(stack.peek())->createIterator(gc);
            stack.pop();
            stack.push(iterable);
            break;
        }
        case opCode::ITER_HAS_NEXT: {
            if (!is_objIterator(stack.peek())) {
                return runtimeError("Expected an iterator object");
            }
            ObjIterator *iterator = as_objIterator(stack.peek());
            Value result = bool_val(iterator->iter->hasNext());
            stack.pop();
            stack.push(result);
            break;
        }
        case opCode::ITER_GET_NEXT: {
            if (!is_objIterator(stack.peek())) {
                return runtimeError("Expected an iterator object");
            }
            ObjIterator *iterator = as_objIterator(stack.peek());
            Value nextVal = iterator->iter->next(gc);
            stack.pop();
            stack.push(nextVal);
            break;
        }
        case opCode::BEGIN_TRY: {
            uint16_t offset = read_word(frame);
            uint8_t *newIp = frame->ip + offset;
            if (tryFrameCount == FRAMES_MAX) {
                return runtimeError("Stack overflow.");
            }
            tryFrames[tryFrameCount++].init(newIp, stack.size(), frame, frameCount);
            break;
        }
        case opCode::END_TRY: {
            tryFrameCount--;
            break;
        }
        case opCode::THROW: {
            tryFrameCount--;
            Value e = stack.pop();
            gc->cache(e);
            frame = tryFrames[tryFrameCount].callFrame;
            frame->ip = tryFrames[tryFrameCount].ip;
            frameCount = tryFrames[tryFrameCount].callFrameCount;
            stack.resize(tryFrames[tryFrameCount].stackSize);
            stack.push(e);
            gc->releaseCache(1);
            break;
        }
        case opCode::RETURN: {
            Value result = stack.pop();
            closeUpvalues(frame->slots);
            frameCount--;
            if (frameCount == 0) {
                stack.pop(); // pop script
                return interpretResult::SUCCESS;
            }

            stack.resize(static_cast<int>(frame->slots - stack.getBase()));
            stack.push(result);
            frame = &frames[frameCount - 1];
            break;
        }
        default:
            return runtimeError("Invalid opcode.");
        }
    }
}

interpretResult VM::runtimeError(StringView msg) const
{
    cerr << msg << endl;
    for (int i = frameCount - 1; i >= 0; i--) {
        const CallFrame *frame = &frames[i];
        const ObjFunction *function = frame->function;
        const size_t instruction = frame->ip - function->chunk->code - 1;
        String name = function->name == nullptr ? "script" : function->name->chars;
        cerr << format("[line {}] in {}", function->chunk->lines[static_cast<long>(instruction)], name)
             << endl;
    }
    return interpretResult::RUNTIME_ERROR;
}

void VM::reset()
{
    stack.reset();
    frameCount = 0;
}
} // namespace aria