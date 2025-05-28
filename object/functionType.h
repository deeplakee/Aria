#ifndef FUNCTIONTYPE_H
#define FUNCTIONTYPE_H

#include "value/value.h"

namespace aria {

enum class FunctionType { FUNCTION, METHOD, INIT_METHOD, SCRIPT };

enum class BoundMethodType { FUNCTION,NATIVE_FN };

using NativeFn = Value (*)(int argCount, Value *args, GC *gc);
}

#endif //FUNCTIONTYPE_H
