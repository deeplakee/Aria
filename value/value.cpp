#include "value.h"
#include "common.h"
#include "object/objBoundMethod.h"
#include "object/objClass.h"
#include "object/objFunction.h"
#include "object/objInstance.h"
#include "object/objList.h"
#include "object/objMap.h"
#include "object/objNativeFn.h"
#include "object/objString.h"
#include "object/objUpvalue.h"

#include <cstring>

#include "object/object.h"
#include "util/util.h"

namespace aria {
String valueString(Value value)
{
    if (is_bool(value)) {
        if (as_bool(value)) {
            return getColoredString("true", Color::Magenta);
        }
        return getColoredString("false", Color::Magenta);
    }
    if (is_nil(value)) {
        return "nil";
    }
    if (is_number(value)) {
        return getColoredString(format("{}", as_number(value)), Color::Green);
    }
    if (is_objString(value)) {
        return getColoredString(as_obj(value)->toString(), Color::Yellow);
    }
    if (is_objFunction(value) || is_objNativeFn(value)) {
        return getColoredString(as_obj(value)->toString(), Color::Blue);
    }
    if (is_obj(value)) {
        return as_obj(value)->toString();
    }
    return "unknown value";
}

String valueString(Value value, ValueStack *outer)
{
    if (is_bool(value)) {
        if (as_bool(value)) {
            return getColoredString("true", Color::Magenta);
        }
        return getColoredString("false", Color::Magenta);
    }
    if (is_nil(value)) {
        return "nil";
    }
    if (is_number(value)) {
        return getColoredString(format("{}", as_number(value)), Color::Green);
    }
    if (is_objString(value)) {
        return getColoredString(as_obj(value)->toString(), Color::Yellow);
    }
    if (is_objFunction(value) || is_objNativeFn(value)) {
        return getColoredString(as_obj(value)->toString(), Color::Blue);
    }
    if (is_obj(value)) {
        return as_obj(value)->toString(outer);
    }
    return "unknown value";
}

String rawValueString(Value value)
{
    if (is_bool(value)) {
        if (as_bool(value)) {
            return "true";
        }
        return "false";
    }
    if (is_nil(value)) {
        return "nil";
    }
    if (is_number(value)) {
        return format("{}", as_number(value));
    }
    if (is_obj(value)) {
        return as_obj(value)->toRawString();
    }
    return "unknown value";
}

String rawValueString(Value value, ValueStack *outer)
{
    if (is_bool(value)) {
        if (as_bool(value)) {
            return "true";
        }
        return "false";
    }
    if (is_nil(value)) {
        return "nil";
    }
    if (is_number(value)) {
        return format("{}", as_number(value));
    }
    if (is_obj(value)) {
        return as_obj(value)->toRawString(outer);
    }
    return "unknown value";
}

String valueTypeString(Value value)
{
    if (is_bool(value)) {
        return "bool";
    }
    if (is_nil(value)) {
        return "nil";
    }
    if (is_number(value)) {
        return "number";
    }
    if (is_obj(value)) {
        switch (as_obj(value)->type) {
        case objType::STRING:
            return "string";
        case objType::FUNCTION:
            return "function";
        case objType::BOUND_METHOD:
            return "boundMethod";
        case objType::NATIVE_FN:
            return "nativeFn";
        case objType::CLASS:
            return "class";
        case objType::INSTANCE:
            return "instance";
        case objType::LIST:
            return "list";
        case objType::MAP:
            return "map";
        case objType::UPVALUE:
            return "upvalue";
        case objType::MODULE:
            return "module";
        case objType::ITERATOR:
            return "iterator";
        default:
            return "unknownObj";
        }
    }
    return "unknownType";
}

bool valuesEqual(Value a, Value b)
{
    if (is_number(a) && is_number(b)) {
        return as_number(a) == as_number(b);
    }
    if (is_objList(a) && is_objList(b)) {
        return as_objList(a)->list->equals(as_objList(b)->list);
    }
    if (is_objMap(a) && is_objMap(b)) {
        return as_objMap(a)->map->equals(as_objMap(b)->map);
    }
    if (is_objInstance(a) && is_objInstance(b)) {
        ObjInstance *aInstance = as_objInstance(a);
        ObjInstance *bInstance = as_objInstance(b);
        if (aInstance->klass != bInstance->klass) {
            return false;
        }
        return aInstance->fields.equals(&bInstance->fields);
    }
    return a == b;
}

bool valuesSame(Value a, Value b)
{
    if (is_number(a) && is_number(b)) {
        return as_number(a) == as_number(b);
    }
    return a == b;
}

static uint32_t hashNumber(double value)
{
    uint32_t hash = 0;
    uint64_t temp;
    memcpy(&temp, &value, sizeof(double));
    hash = static_cast<uint32_t>(temp ^ (temp >> 32));
    hash = hash ^ (hash >> 16);
    hash = hash ^ (hash << 15);
    return hash;
}

uint32_t valueHash(Value value)
{
    if (is_bool(value))
        return static_cast<uint32_t>(as_bool(value)) | 0x200030611;
    if (is_nil(value))
        return 0;
    if (is_number(value))
        return hashNumber(as_number(value));
    return as_obj(value)->hash;
}

void markValue(Value value, GC *gc)
{
    if (is_obj(value)) {
        as_obj(value)->mark(gc);
    }
}

bool isFalsey(Value value)
{
    return is_nil(value) || (is_bool(value) && !as_bool(value));
}
} // namespace aria
