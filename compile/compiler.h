#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"
#include "functionContext.h"

namespace aria {
class ValueHashTable;
class Chunk;
class GC;

class Compiler
{
public:
    FunctionContext mainCode;

    explicit Compiler(GC *gc);

    Compiler(GC *gc, ValueHashTable *globalVarTable);

    ~Compiler();

    ObjFunction *compile(String source);

    ObjFunction *compile(String source , String moduleName);
};

} // namespace aria

#endif //COMPILER_H
