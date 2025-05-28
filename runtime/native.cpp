#include "native.h"
#include <ctime>
#include <random>

#include "object/objList.h"
#include "object/objString.h"
#include "runtimeException.h"
#include "vm.h"

namespace aria {

Value Native::_clock_(int argCount, Value *args, GC *gc)
{
    return number_val(static_cast<double>(::clock()) / CLOCKS_PER_SEC);
}

Value Native::_random_(int argCount, Value *args, GC *gc)
{
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_int_distribution<> dis(0, INT_MAX);

    return number_val(dis(gen));
}


Value Native::_println_(int argCount, Value *args, GC *gc)
{
    if (argCount < 1) {
        cout << '\n';
        return nil_val;
    }

    // 获取格式化字符串
    ObjList *list = as_objList(args[0]);
    String formatStr = rawValueString((*list->list)[0]);

    if (argCount == 1) {
        cout << valueString((*list->list)[0]) << '\n';
        return nil_val;
    }

    try {
        String result;
        int argIndex = 1; // 从 args[1] 开始是占位符参数

        for (size_t i = 0; i < formatStr.length(); ++i) {
            if (formatStr[i] == '{' && i + 1 < formatStr.length() && formatStr[i + 1] == '}') {
                if (argIndex < argCount) {
                    result += rawValueString((*list->list)[argIndex++]);
                } else {
                    result += "{}";
                }
                i++; // 跳过 '}'
            } else {
                result += formatStr[i];
            }
        }

        cout << getColoredString(result, Color::Yellow) << '\n';
    } catch (const std::exception &e) {
        throw RuntimeException{e.what()};
    }

    return nil_val;
}

Value Native::_readline_(int argCount, Value *args, GC *gc)
{
    String line;
    std::getline(std::cin, line);
    ObjString *objLineStr = newObjString(line, gc);
    return obj_val(objLineStr);
}

Value Native::_typeof_(int argCount, Value *args, GC *gc)
{
    ObjString *str = newObjString(valueTypeString(args[0]), gc);
    return obj_val(str);
}

Value Native::_str_(int argCount, Value *args, GC *gc)
{
    ObjString *str = newObjString(rawValueString(args[0]), gc);
    return obj_val(str);
}

Value Native::_num_(int argCount, Value *args, GC *gc)
{
    if (!is_objString(args[0])) {
        throw RuntimeException("argument must be a string");
    }
    try {
        return number_val(std::stod(as_objString(args[0])->chars));
    } catch (const std::exception &e) {
        return nil_val;
    }
}

Value Native::_bool_(int argCount, Value *args, GC *gc)
{
    if (!is_objString(args[0])) {
        throw RuntimeException("argument must be a string");
    }
    ObjString *str = as_objString(args[0]);
    if (str->length == 4 && memcmp(str->chars, "true", 4) == 0) {
        return true_val;
    }
    if (str->length == 5 && memcmp(str->chars, "false", 5) == 0) {
        return false_val;
    }
    throw RuntimeException("Invalid boolean string");
}

Value Native::_copy_(int argCount, Value *args, GC *gc)
{
    if (is_obj(args[0])) {
        return as_obj(args[0])->copy(gc);
    }
    return nil_val;
}

Value Native::_equals_(int argCount, Value *args, GC *gc)
{
    return bool_val(valuesEqual(args[0], args[1]));
}

Value Native::_iterator_(int argCount, Value *args, GC *gc)
{
    if (is_obj(args[0])) {
        return as_obj(args[0])->createIterator(gc);
    }
    return nil_val;
}

Value Native::_exit_(int argCount, Value *args, GC *gc)
{
    if (!is_number(args[0])) {
        exit(1);
    }
    exit(static_cast<int>(as_number(args[0])));
    return nil_val;
}

Map<String, Tuple<NativeFn, int, bool>> &Native::getExternNativeFnTable()
{
    static Map<String, Tuple<NativeFn, int, bool>> externNativeFn;
    return externNativeFn;
}

} // namespace aria
