#include "objString.h"

#include "memory/conStringPool.h"
#include "memory/gc.h"
#include "objIterator.h"
#include "objList.h"
#include "objNativeFn.h"
#include "runtime/runtimeException.h"
#include "util/util.h"
#include "value/valueHashTable.h"

namespace aria {

/////////////////////////////////////////////////////////////////////
///       The functions following are the built-in method of list

static Value builtin_length(int argCount, Value *args, GC *gc)
{
    ObjString *objstr = as_objString(args[-1]);
    return number_val(static_cast<double>(objstr->length));
}

static Value builtin_at(int argCount, Value *args, GC *gc)
{
    ObjString *objstr = as_objString(args[-1]);
    if (unlikely(!is_number(args[0]))) {
        throw RuntimeException("argument must be a number");
    }
    int index = static_cast<int>(as_number(args[0]));
    if (index != as_number(args[0])) {
        throw RuntimeException("argument must be a integer");
    }
    if (unlikely(index < 0 || index >= objstr->length)) {
        throw RuntimeException("index out of range");
    }
    char a = objstr->chars[index];
    return obj_val(newObjString(a, gc));
}

static Value builtin_substr(int argCount, Value *args, GC *gc)
{
    ObjString *objstr = as_objString(args[-1]);

    if (unlikely(!is_number(args[0]))) {
        throw RuntimeException("argument must be a number");
    }
    int start = static_cast<int>(as_number(args[0]));
    if (start != as_number(args[0])) {
        throw RuntimeException("argument must be a integer");
    }
    if (unlikely(start < 0 || start >= objstr->length)) {
        throw RuntimeException("start index out of range");
    }

    if (unlikely(!is_number(args[1]))) {
        throw RuntimeException("argument must be a number");
    }
    int end = static_cast<int>(as_number(args[1]));
    if (end != as_number(args[1])) {
        throw RuntimeException("argument must be a integer");
    }
    if (unlikely(end < 0 || end >= objstr->length)) {
        throw RuntimeException("start index out of range");
    }

    if (end < start) {
        throw RuntimeException("start index should be smaller than end index");
    }

    ObjString *str = newObjString(objstr->chars, end - start, gc);
    return obj_val(str);
}

static Value builtin_findstr(int argCount, Value *args, GC *gc)
{
    ObjString *objstr = as_objString(args[-1]);
    if (!is_objString(args[0])) {
        throw RuntimeException("argument must be a string");
    }
    ObjString *substr = as_objString(args[0]);
    char *result = strstr(objstr->chars, substr->chars);
    if (result == nullptr) {
        return number_val(-1);
    }
    return number_val(static_cast<double>(result - objstr->chars));
}

static Value builtin_concat(int argCount, Value *args, GC *gc)
{
    const ObjString *a = as_objString(args[-1]);
    if (!is_objString(args[0])) {
        throw RuntimeException("argument must be a string");
    }
    const ObjString *b = as_objString(args[0]);
    ObjString *concatenatedStr = concatenateString(a->chars, b->chars, gc);
    return obj_val(concatenatedStr);
}

static Value builtin_startWith(int argCount, Value *args, GC *gc)
{
    const ObjString *objstr = as_objString(args[-1]);
    if (!is_objString(args[0])) {
        throw RuntimeException("argument must be a string");
    }
    const ObjString *substr = as_objString(args[0]);
    if (substr->length > objstr->length) {
        return false_val;
    }

    int result = memcmp(objstr->chars, substr->chars, substr->length) == 0;
    return bool_val(result);
}

static Value builtin_endWith(int argCount, Value *args, GC *gc)
{
    const ObjString *objstr = as_objString(args[-1]);
    if (!is_objString(args[0])) {
        throw RuntimeException("argument must be a string");
    }
    const ObjString *substr = as_objString(args[0]);
    if (substr->length > objstr->length) {
        return false_val;
    }

    int result
        = memcmp(objstr->chars + objstr->length - substr->length, substr->chars, substr->length)
          == 0;
    return bool_val(result);
}

static Value builtin_reverse(int argCount, Value *args, GC *gc)
{
    const ObjString *objstr = as_objString(args[-1]);
    auto length = objstr->length;
    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[length] = '\0';
    for (int i = 0; i < length; i++) {
        newStr[i] = objstr->chars[length - i - 1];
    }
    ObjString *newStrObj = newObjStringFromRawCStr(newStr, length, gc);
    return obj_val(newStrObj);
}

static Value builtin_upper(int argCount, Value *args, GC *gc)
{
    const ObjString *objstr = as_objString(args[-1]);
    auto length = objstr->length;
    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[length] = '\0';
    for (int i = 0; i < length; i++) {
        newStr[i] = static_cast<char>(toupper(objstr->chars[i]));
    }
    ObjString *newStrObj = newObjStringFromRawCStr(newStr, length, gc);
    return obj_val(newStrObj);
}

static Value builtin_lower(int argCount, Value *args, GC *gc)
{
    const ObjString *objstr = as_objString(args[-1]);
    size_t length = objstr->length;
    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[length] = '\0';
    for (int i = 0; i < length; i++) {
        newStr[i] = static_cast<char>(tolower(objstr->chars[i]));
    }
    ObjString *newStrObj = newObjStringFromRawCStr(newStr, length, gc);
    return obj_val(newStrObj);
}

static Value builtin_trim(int argCount, Value *args, GC *gc)
{
    const ObjString *objstr = as_objString(args[-1]);
    const char *start = objstr->chars;
    while (*start && isspace(*start)) {
        start++;
    }
    const char *end = objstr->chars + objstr->length - 1;
    while (end > start && std::isspace(*end)) {
        end--;
    }
    size_t length = end - start + 1;
    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[length] = '\0';
    std::memcpy(newStr, start, length);
    ObjString *newStrObj = newObjStringFromRawCStr(newStr, length, gc);
    return obj_val(newStrObj);
}

static Value builtin_ltrim(int argCount, Value *args, GC *gc)
{
    const ObjString *objstr = as_objString(args[-1]);
    const char *start = objstr->chars;
    while (*start && isspace(*start)) {
        start++;
    }
    const char *end = objstr->chars + objstr->length - 1;
    size_t length = end - start + 1;
    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[length] = '\0';
    std::memcpy(newStr, start, length);
    ObjString *newStrObj = newObjStringFromRawCStr(newStr, length, gc);
    return obj_val(newStrObj);
}

static Value builtin_rtrim(int argCount, Value *args, GC *gc)
{
    const ObjString *objstr = as_objString(args[-1]);
    const char *start = objstr->chars;
    const char *end = objstr->chars + objstr->length - 1;
    while (end > start && std::isspace(*end)) {
        end--;
    }
    size_t length = end - start + 1;
    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[length] = '\0';
    std::memcpy(newStr, start, length);
    ObjString *newStrObj = newObjStringFromRawCStr(newStr, length, gc);
    return obj_val(newStrObj);
}

static Value builtin_split(int argCount, Value *args, GC *gc)
{
    const ObjString *src = as_objString(args[-1]);
    if (!is_objString(args[0])) {
        throw RuntimeException("argument must be a string");
    }
    if (src->length == 0) {
        return obj_val(newObjList(gc));
    }
    const ObjString *delim = as_objString(args[0]);
    const char *srcStr = src->chars;
    const char *delimStr = delim->chars;
    ObjList *objlist = newObjList(gc);
    gc->cache(obj_val(objlist));

    if (delim->length == 0) {
        for (int i = 0; srcStr[i] != '\0'; i++) {
            if (!isspace(srcStr[i])) {
                ObjString *i_str = newObjString(srcStr[i], gc);
                gc->cache(obj_val(i_str));
                objlist->list->write(obj_val(i_str));
                gc->releaseCache(1);
            }
        }
    } else {
        const size_t delim_len = strlen(delimStr);
        char *start = src->chars;
        char *end = strstr(start, delimStr);

        while (end != nullptr) {
            size_t token_len = end - start;
            if (token_len > 0) {
                ObjString *i_str = newObjString(start, token_len, gc);
                gc->cache(obj_val(i_str));
                objlist->list->write(obj_val(i_str));
                gc->releaseCache(1);
            }
            start = end + delim_len;
            end = strstr(start, delimStr);
        }

        if (strlen(start) > 0) {
            ObjString *i_str = newObjString(start, strlen(start), gc);
            gc->cache(obj_val(i_str));
            objlist->list->write(obj_val(i_str));
            gc->releaseCache(1);
        }
    }

    gc->releaseCache(1);
    return obj_val(objlist);
}

/////////////////////////////////////////////////////////////////////

ValueHashTable *ObjString::builtinMethod = nullptr;

ObjString::ObjString(char *_chars, size_t _length, uint32_t _hash, GC *_gc)
    : chars{_chars}
    , length{_length}
{
    type = objType::STRING;
    hash = _hash;
}

ObjString::~ObjString() = default;

bool ObjString::getAttribute(ObjString *name, Value &value)
{
    return builtinMethod->get(obj_val(name), value);
}

bool ObjString::getElement(Value k, Value &v, GC *gc)
{
    if (unlikely(!is_number(k))) {
        throw RuntimeException("index of list must be a number");
    }
    int index = static_cast<int>(as_number(k));
    if (index != as_number(k)) {
        throw RuntimeException("index of list must be a integer");
    }
    if (unlikely(index < 0 || index >= length)) {
        throw RuntimeException("index out of range");
    }
    char a = chars[index];
    v = obj_val(newObjString(a, gc));
    return true;
}

Value ObjString::createIterator(GC *gc)
{
    return obj_val(newObjIterator(this, gc));
}

String ObjString::toString()
{
    return String{chars};
}
String ObjString::toRawString()
{
    return String{chars};
}

static uint32_t hashString(const char *key, const size_t length)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++) {
        hash ^= static_cast<uint8_t>(key[i]);
        hash *= 16777619;
    }
    return hash;
}

void ObjString::init(GC *gc)
{
    builtinMethod = new ValueHashTable{gc};

    bindBuiltinMethod(builtinMethod, "length", builtin_length, 0, gc);
    bindBuiltinMethod(builtinMethod, "at", builtin_at, 1, gc);
    bindBuiltinMethod(builtinMethod, "substr", builtin_substr, 2, gc);
    bindBuiltinMethod(builtinMethod, "findstr", builtin_findstr, 1, gc);
    bindBuiltinMethod(builtinMethod, "concat", builtin_concat, 1, gc);
    bindBuiltinMethod(builtinMethod, "startWith", builtin_startWith, 1, gc);
    bindBuiltinMethod(builtinMethod, "endWith", builtin_endWith, 1, gc);
    bindBuiltinMethod(builtinMethod, "reverse", builtin_reverse, 0, gc);
    bindBuiltinMethod(builtinMethod, "upper", builtin_upper, 0, gc);
    bindBuiltinMethod(builtinMethod, "lower", builtin_lower, 0, gc);
    bindBuiltinMethod(builtinMethod, "trim", builtin_trim, 0, gc);
    bindBuiltinMethod(builtinMethod, "ltrim", builtin_ltrim, 0, gc);
    bindBuiltinMethod(builtinMethod, "rtrim", builtin_rtrim, 0, gc);
    bindBuiltinMethod(builtinMethod, "split", builtin_split, 1, gc); // split()
    // todo
    // replace(a,b)
    // join(list)
}

ObjString *newObjString(const String &str, GC *gc)
{
    const size_t length = str.length();
    uint32_t hash = hashString(str.c_str(), length);
    ObjString *interned = gc->getStr(str.c_str(), length, hash);
    if (interned != nullptr)
        return interned;
#ifdef DEBUG_TRACE_STRING_OBJECT_CREATE
    print("newObjStr: {} length: {} hash: {} gc: {:p}\n", str, length, hash, toVoidPtr(gc));
#endif
    char *newStr = gc->allocate_array<char>(length + 1);
    memcpy(newStr, str.c_str(), length);
    newStr[length] = '\0';
    ObjString *obj = gc->allocate_object<ObjString>(newStr, length, hash, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjString\n", toVoidPtr(obj), sizeof(ObjString));
#endif

    gc->insertStr(obj);
    return obj;
}

ObjString *newObjString(const char *str, GC *gc)
{
    const size_t length = strlen(str);
    uint32_t hash = hashString(str, length);
    ObjString *interned = gc->getStr(str, length, hash);
    if (interned != nullptr)
        return interned;
#ifdef DEBUG_TRACE_STRING_OBJECT_CREATE
    print("newObjStr: {} length: {} hash: {} gc: {:p}\n", str, length, hash, toVoidPtr(gc));
#endif
    char *newStr = gc->allocate_array<char>(length + 1);
    memcpy(newStr, str, length);
    newStr[length] = '\0';
    ObjString *obj = gc->allocate_object<ObjString>(newStr, length, hash, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjString\n", toVoidPtr(obj), sizeof(ObjString));
#endif

    gc->insertStr(obj);
    return obj;
}

ObjString *newObjString(char ch, GC *gc)
{
    const size_t length = 1;
    uint32_t hash = hashString(&ch, length); // 计算字符的哈希值
    ObjString *interned = gc->getStr(&ch, length, hash);
    if (interned != nullptr)
        return interned;
#ifdef DEBUG_TRACE_STRING_OBJECT_CREATE
    print("newObjStrFromChar: {} length: {} hash: {} gc: {:p}\n", ch, length, hash, toVoidPtr(gc));
#endif

    char *newStr = gc->allocate_array<char>(length + 1);
    newStr[0] = ch;
    newStr[length] = '\0';
    ObjString *obj = gc->allocate_object<ObjString>(newStr, length, hash, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjString\n", toVoidPtr(obj), sizeof(ObjString));
#endif
    gc->insertStr(obj);
    return obj;
}

ObjString *newObjString(char *c_str, size_t length, GC *gc)
{
    uint32_t hash = hashString(c_str, length);
    ObjString *interned = gc->getStr(c_str, length, hash);
    if (interned != nullptr) {
        return interned;
    }

#ifdef DEBUG_TRACE_STRING_OBJECT_CREATE
    print("newObjStrFromCStr: {} length: {} hash: {} gc: {:p}\n", c_str, length, hash, toVoidPtr(gc));
#endif

    char *newStr = gc->allocate_array<char>(length + 1);
    memcpy(newStr, c_str, length);
    newStr[length] = '\0';

    ObjString *obj = gc->allocate_object<ObjString>(newStr, length, hash, gc);

#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjString\n", toVoidPtr(obj), sizeof(ObjString));
#endif

    gc->insertStr(obj);
    return obj;
}

ObjString *newObjStringFromRawCStr(char *c_str, size_t length, GC *gc)
{
    uint32_t hash = hashString(c_str, length);
    ObjString *interned = gc->getStr(c_str, length, hash);
    if (interned != nullptr) {
        gc->free_array<char>(c_str, length + 1);
        return interned;
    }

#ifdef DEBUG_TRACE_STRING_OBJECT_CREATE
    print("newObjStrFromCStr: {} length: {} hash: {} gc: {:p}\n", c_str, length, hash, toVoidPtr(gc));
#endif
    ObjString *obj = gc->allocate_object<ObjString>(c_str, length, hash, gc);

#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjString\n", toVoidPtr(obj), sizeof(ObjString));
#endif

    gc->insertStr(obj);
    return obj;
}

ObjString *concatenateString(const ObjString *a, const ObjString *b, GC *gc)
{
    const size_t length = a->length + b->length;
    if (length < a->length) {
        throw RuntimeException{"you are trying to concatenate two too long string"};
    }
    char *new_c_str = gc->allocate_array<char>(length + 1);
    memcpy(new_c_str, a->chars, a->length);
    memcpy(new_c_str + a->length, b->chars, b->length);
    new_c_str[length] = '\0';
    uint32_t hash = hashString(new_c_str, length);
    ObjString *interned = gc->getStr(new_c_str, length, hash);
    if (interned != nullptr) {
        gc->free_array<char>(new_c_str, length + 1);
        return interned;
    }
#ifdef DEBUG_TRACE_STRING_OBJECT_CREATE
    print("newObjStr: {} length: {} hash: {} gc: {}\n", new_c_str, length, hash, toVoidPtr(gc));
#endif
    ObjString *obj = gc->allocate_object<ObjString>(new_c_str, length, hash, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjString\n", toVoidPtr(obj), sizeof(ObjString));
#endif

    gc->insertStr(obj);
    return obj;
}

ObjString *concatenateString(const char *a, const char *b, GC *gc)
{
    size_t a_length = strlen(a);
    size_t b_length = strlen(b);
    const size_t length = a_length + b_length;
    if (length < a_length) {
        throw RuntimeException{"you are trying to concatenate two too long string"};
    }
    char *new_c_str = gc->allocate_array<char>(length + 1);
    memcpy(new_c_str, a, a_length);
    memcpy(new_c_str + a_length, b, b_length);
    new_c_str[length] = '\0';
    uint32_t hash = hashString(new_c_str, length);
    ObjString *interned = gc->getStr(new_c_str, length, hash);
    if (interned != nullptr) {
        gc->free_array<char>(new_c_str, length + 1);
        return interned;
    }
#ifdef DEBUG_TRACE_STRING_OBJECT_CREATE
    print("newObjStr: {} length: {} hash: {} gc: {}\n", new_c_str, length, hash, toVoidPtr(gc));
#endif
    ObjString *obj = gc->allocate_object<ObjString>(new_c_str, length, hash, gc);
#ifdef DEBUG_LOG_GC
    print("{:p} allocate {} for ObjString\n", toVoidPtr(obj), sizeof(ObjString));
#endif

    gc->insertStr(obj);
    return obj;
}

} // namespace aria
