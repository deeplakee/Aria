#ifndef ARIAAPI_H
#define ARIAAPI_H

#include "common.h"
#include "memory/gc.h"
#include "object/functionType.h"
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
#include "runtime/native.h"
#include "value/value.h"
#include "value/valueArray.h"
#include "value/valueHashTable.h"
#include "value/valueStack.h"

#define REGISTER_NATIVE_ARIA_FUNCTION(name, function, argCount) \
    namespace aria { \
    struct FunctionRegistrar_##function \
    { \
        FunctionRegistrar_##function() \
        { \
            Native::getExternNativeFnTable()[name] = std::make_tuple(function, argCount, false); \
        } \
    }; \
    static FunctionRegistrar_##function registrar_##function; \
    }

#define REGISTER_NATIVE_ARIA_FUNCTION_VARIADIC(name, function, argCount) \
    namespace aria { \
    struct FunctionRegistrar_##function \
    { \
        FunctionRegistrar_##function() \
        { \
            Native::getExternNativeFnTable()[name] = std::make_tuple(function, argCount, true); \
        } \
    }; \
    static FunctionRegistrar_##function registrar_##function; \
    }

#endif //ARIAAPI_H
