#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace aria {
using std::cout;

using std::cerr;

using std::endl;

using std::format;

using StringView = std::string_view;

using String = std::string;

template<typename T>
using List = std::vector<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename T1, typename T2>
using Map = std::map<T1, T2>;

template<typename T1, typename T2>
using Pair = std::pair<T1, T2>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename... Args>
using Tuple = std::tuple<Args...>;

inline constexpr const char* programName = "aria";
inline constexpr const char* sourceSuffix = ".aria";


#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_COMPILED_CODE
#define DEBUG_TRACE_STRING_OBJECT_CREATE
#define DEBUG_STRESS_GC
#define DEBUG_PRINT_IMPORT_MODULE_PATH
#define DEBUG_LOG_GC

#ifdef DISABLE_DEBUG_TRACE_EXECUTION
#undef DEBUG_TRACE_EXECUTION
#endif

#ifdef DISABLE_DEBUG_PRINT_COMPILED_CODE
#undef DEBUG_PRINT_COMPILED_CODE
#endif

#ifdef DISABLE_DEBUG_TRACE_STRING_OBJECT_CREATE
#undef DEBUG_TRACE_STRING_OBJECT_CREATE
#endif

#ifdef DISABLE_DEBUG_STRESS_GC
#undef DEBUG_STRESS_GC
#endif

#ifdef DISABLE_DEBUG_PRINT_IMPORT_MODULE_PATH
#undef DEBUG_PRINT_IMPORT_MODULE_PATH
#endif

#ifdef DISABLE_DEBUG_LOG_GC
#undef DEBUG_LOG_GC
#endif



#define DISABLE_LOG
#ifdef DISABLE_LOG
#undef DEBUG_TRACE_STRING_OBJECT_CREATE
#undef DEBUG_LOG_GC
#endif


} // namespace aria

#endif // COMMON_H
