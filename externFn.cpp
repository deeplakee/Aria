#include "ariaApi.h"

namespace aria {

// C++实现的使用递归方式计算斐波那契数列函数
int64_t cpp_fib(int64_t n)
{
    if (n < 2)
        return n;
    return cpp_fib(n - 2) + cpp_fib(n - 1);
}

// 定义Aria解释器可调用的原生函数native_fib
Value native_fib(int argCount, Value *args, GC *gc)
{
    // 检查传入参数类型
    if (!is_number(args[0])) {
        return nil_val;
    }
    double num = as_number(args[0]);
    // 处理非正数情况
    if (num <= 0)
        return number_val(0);
    // 调用C++函数
    double result = static_cast<double>(cpp_fib(static_cast<int64_t>(num)));
    return number_val(result);
}

REGISTER_NATIVE_ARIA_FUNCTION("native_fib", native_fib, 1);
} // namespace aria
