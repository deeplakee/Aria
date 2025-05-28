cmake_minimum_required(VERSION 3.27)
project(aria)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR})

# 默认设置为 Release 模式
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

option(USE_READLINE "Enable readline support" OFF)

if(USE_READLINE)
    find_library(READLINE_LIBRARY NAMES readline)
    if(READLINE_LIBRARY)
        set(HAS_READLINE 1 CACHE INTERNAL "Readline available")
        message(STATUS "Found readline: ${READLINE_LIBRARY}")
    else()
        set(HAS_READLINE 0 CACHE INTERNAL "Readline not found")
        message(WARNING "readline not found, disabling readline support")
    endif()
else()
    set(HAS_READLINE 0 CACHE INTERNAL "Readline disabled by USE_READLINE=OFF")
    message(STATUS "USE_READLINE is OFF, readline support disabled")
endif()

add_executable(${PROJECT_NAME} main.cpp
        common.h
        runtime/interpretResult.h
        chunk/code.h
        util/util.h
        util/lock.h
        util/nan.h
        util/rleList.h
        util/util.cpp
        value/value.h
        value/value.cpp
        memory/gc.h
        memory/gc.cpp
        object/object.h
        object/object.cpp
        value/valueArray.h
        value/valueArray.cpp
        value/valueStack.h
        value/valueStack.cpp
        value/valueHashTable.h
        value/valueHashTable.cpp
        chunk/chunk.h
        chunk/chunk.cpp
        compile/compilingException.h
        object/objString.h
        object/objString.cpp
        runtime/runtimeException.h
        object/objFunction.h
        object/objFunction.cpp
        object/objNativeFn.h
        object/objNativeFn.cpp
        compile/token.h
        compile/token.cpp
        compile/scanner.h
        compile/scanner.cpp
        compile/parser.h
        compile/astNode.h
        compile/parser.cpp
        compile/functionContext.h
        compile/functionContext.cpp
        compile/generateByteCode.cpp
        compile/astNode.cpp
        compile/compiler.h
        compile/compiler.cpp
        runtime/callFrame.h
        runtime/callFrame.cpp
        runtime/native.h
        runtime/native.cpp
        runtime/vm.h
        runtime/vm.cpp
        memory/conStringPool.h
        memory/conStringPool.cpp
        object/objUpvalue.h
        object/objUpvalue.cpp
        object/objClass.h
        object/objClass.cpp
        object/objInstance.h
        object/objInstance.cpp
        object/objBoundMethod.h
        object/objBoundMethod.cpp
        chunk/disassembler.h
        chunk/disassembler.cpp
        object/objList.h
        object/objList.cpp
        object/objMap.h
        object/objMap.cpp
        object/functionType.h
        object/objModule.h
        object/objModule.cpp
        object/objIterator.h
        object/iterator.h
        object/objIterator.cpp
        object/iterator.cpp
        ariaApi.h
        externFn.cpp
        runtime/tryFrame.h
        runtime/tryFrame.cpp
)

if(HAS_READLINE EQUAL 1)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_READLINE)
endif()

# 在 Release 模式下取消一些 debug 宏定义
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_DEBUG_TRACE_EXECUTION)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_DEBUG_PRINT_COMPILED_CODE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_DEBUG_TRACE_STRING_OBJECT_CREATE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_DEBUG_STRESS_GC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_DEBUG_LOG_GC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DISABLE_DEBUG_PRINT_IMPORT_MODULE_PATH)
endif()

if (UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif ()

if(HAS_READLINE)
    target_link_libraries(${PROJECT_NAME} ${READLINE_LIBRARY})
endif()
