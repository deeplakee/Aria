#ifndef NATIVE_H
#define NATIVE_H

#include "object/functionType.h"
#include "value/value.h"

namespace aria {
class GC;

class Native
{
public:
    static constexpr double pi = 3.141592653589793238462643383279502884L;
    static constexpr double e = 2.718281828459045235360287471352662498L;

    static Value _clock_(int argCount, Value *args, GC *gc);
    static Value _random_(int argCount, Value *args, GC *gc);
    static Value _println_(int argCount, Value *args, GC *gc);
    static Value _readline_(int argCount, Value *args, GC *gc);
    static Value _typeof_(int argCount, Value *args, GC *gc);
    static Value _str_(int argCount, Value *args, GC *gc);
    static Value _num_(int argCount, Value *args, GC *gc);
    static Value _bool_(int argCount, Value *args, GC *gc);
    static Value _copy_(int argCount, Value *args, GC *gc);
    static Value _equals_(int argCount, Value *args, GC *gc);
    static Value _iterator_(int argCount, Value *args, GC *gc);
    static Value _exit_(int argCount, Value *args, GC *gc);

    static Map<String, Tuple<NativeFn, int, bool>> &getExternNativeFnTable();

};
} // namespace aria

#endif //NATIVE_H
