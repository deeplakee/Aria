#ifndef VM_H
#define VM_H

#include "callFrame.h"
#include "tryFrame.h"
#include "common.h"
#include "interpretResult.h"
#include "memory/gc.h"
#include "object/functionType.h"
#include "object/objModule.h"
#include "value/valueHashTable.h"
#include "value/valueStack.h"

namespace aria {}
namespace aria {
#define FRAMES_MAX 256
class ObjBoundMethod;
class ObjString;
class ObjClass;
class ObjUpvalue;
class ObjNativeFn;
class Chunk;
class GC;

class VM
{
public:
    VM();

    ~VM();

    interpretResult interpret(String &source);

    interpretResult interpret(String path, String &source);

    interpretResult interpretByLine(String &source);

    void registerNative();

    void defineNativeFn(const char *name, int arity, NativeFn function, bool acceptsVarargs=false);

    void defineNativeVar(const char *name, Value value);

    void defineNativeClass(const char *name);

    friend void GC::bindVM(VM *vm);

private:
    GC *gc;
    bool replMode;

    CallFrame *frames;
    int frameCount;

    TryFrame *tryFrames;
    int tryFrameCount;

    ValueStack stack;

    ValueHashTable *globalVarTableForRepl;
    ValueHashTable *nativeVarTable;
    ValueHashTable *modules;

    ObjUpvalue *openUpvalues;

    int line;
    String fileName;
    String fileDirectory;

    bool callValue(Value callee, int argCount);

    bool invoke(ObjString *name, int argCount);

    bool callFunction(ObjFunction *function, int argCount);

    bool callNativeFn(ObjNativeFn *native, int argCount);

    bool callNewInstance(ObjClass *klass, int argCount);

    bool callBoundMethod(ObjBoundMethod *bound, int argCount);

    ObjUpvalue *captureUpvalue(Value *local);

    void closeUpvalues(Value *last);

    Value bindMethodIfNeeded(Value obj, Value value) const;

    ObjModule *getCachedModule(ObjString *path);

    ObjFunction *loadModule(const String &absoluteModulePath, ObjString *moduleName);

    static uint8_t read_byte(CallFrame *frame);

    static uint16_t read_word(CallFrame *frame);

    static Value read_constant(CallFrame *frame);

    static ObjString *read_objString(CallFrame *frame);

    interpretResult run();

    interpretResult runtimeError(StringView msg) const;

    void reset();
};
} // namespace aria

#endif //VM_H