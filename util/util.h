#ifndef UTIL_H
#define UTIL_H

#include <cctype>
#include <concepts>

#include "common.h"

namespace aria {

String getProgramDirectory();

String getCurrentWorkingDirectory();

/**
 *
 * @param currentPath current path
 * @param filePath path of file, it could be a relative path of current path or an absolute path
 * @return The absolute path of the file
 */
String getAbsolutePath(const String &currentPath, const String &filePath);

String makeFullProgramPath(const String &input);

String makeFullProgramLibPath(const String &input);

String makeFullWorkPath(const String &input);

String readFile(const String &path);

String getAbsoluteModulePath(const String &input, const String &runningFileDirectory);

String getModulePathByName(const String &name, const String &runningFileDirectory);

String getModulePathByPath(const String &path, const String &runningFileDirectory);

String getFileDirectory(const String &input);

enum class Color {
    Default, // 默认颜色
    Red,     // 红色
    Green,   // 绿色
    Blue,    // 蓝色
    Yellow,  // 黄色
    Cyan,    // 青色
    Magenta, // 洋红色
    White    // 白色
};

String getColorCode(Color color);

String getColoredString(const String &text, Color color);

inline bool isDigit(char ch)
{
    return isdigit(ch);
}

inline bool isAlpha(char ch)
{
    return isalpha(ch) || ch == '_';
}

inline bool isAlphaNum(char ch)
{
    return isalpha(ch) || isdigit(ch);
}

template<typename T>
void swap(T &a, T &b) noexcept
{
    T tmep = a;
    a = b;
    b = tmep;
}

template<typename T>
concept PointerType = std::is_pointer_v<T>; // Limit T to be pointer type

template<PointerType T>
void *toVoidPtr(T ptr)
{
    return static_cast<void *>(ptr);
}

constexpr size_t constexprStrlen(const char *str)
{
    size_t len = 0;
    while (str[len] != '\0') {
        ++len;
    }
    return len;
}

} // namespace aria

#endif // UTIL_H
