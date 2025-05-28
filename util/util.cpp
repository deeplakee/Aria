#include "util.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace aria {

String getProgramDirectory()
{
    String path;
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    path = std::filesystem::path(buffer).parent_path().string();
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        path = std::filesystem::path(buffer).parent_path().string();
    }
#endif
    return path;
}

String getCurrentWorkingDirectory()
{
    return std::filesystem::current_path().string();
}

String getAbsolutePath(const String &currentPath, const String &filePath)
{
    std::filesystem::path pathA = currentPath;
    std::filesystem::path pathB = filePath;

    if (pathB.is_absolute()) {
        // 如果路径已经是绝对路径，直接返回规范化后的路径
        return std::filesystem::weakly_canonical(pathB).string();
    }

    // 拼接路径并返回规范化后的绝对路径
    return std::filesystem::weakly_canonical(pathA / pathB).string();
}

String makeFullProgramPath(const String &input)
{
    // 检查输入是否包含路径分隔符
    bool isPath = input.find('/') != String::npos || input.find('\\') != String::npos;

    if (isPath) {
        // 如果已经是路径，直接返回
        return input;
    }
    // 如果不是路径，拼接程序所在目录
    String programDir = getProgramDirectory();
    return (std::filesystem::path(programDir) / input).string();
}

String makeFullProgramLibPath(const String &input)
{
    // 检查输入是否包含路径分隔符
    bool isPath = input.find('/') != String::npos || input.find('\\') != String::npos;

    if (isPath) {
        // 如果已经是路径，直接返回
        return input;
    }
    // 如果不是路径，拼接运行程序所在目录
    String programDir = getProgramDirectory();
    std::filesystem::path fullPath = std::filesystem::path(programDir) / "lib" / input;
    return fullPath.string();
}

String makeFullWorkPath(const String &input)
{
    // 检查输入是否包含路径分隔符
    bool isPath = input.find('/') != String::npos || input.find('\\') != String::npos;

    if (isPath) {
        // 如果已经是路径，直接返回
        return input;
    }
    // 如果不是路径，拼接运行程序所在目录
    String programDir = getCurrentWorkingDirectory();
    return (std::filesystem::path(programDir) / input).string();
}

String readFile(const String &path)
{
    String filePath = makeFullWorkPath(path);

    std::ifstream file(filePath, std::ios::in);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

String getAbsoluteModulePath(const String &input, const String &runningFileDirectory)
{
    // 检查输入是否包含路径分隔符
    bool isPath = input.find('/') != String::npos || input.find('\\') != String::npos;

    if (isPath) {
        // 如果是路径，调用 readModuleByPath
        return getModulePathByPath(input, runningFileDirectory);
    }
    // 如果是模块名，调用 readModuleByName
    return getModulePathByName(input, runningFileDirectory);
}

String getModulePathByName(const String &name, const String &runningFileDirectory) {
    // 首先尝试在源代码所在目录下查找文件
    String fileName = name + sourceSuffix;
    std::filesystem::path filePath = std::filesystem::path(runningFileDirectory) / fileName;

    std::ifstream file(filePath, std::ios::in);

    if (!file) {
        // 如果在源代码所在目录下找不到文件，尝试从程序所在目录的 lib/ 目录查找
        filePath = std::filesystem::path(makeFullProgramLibPath(fileName));
        file.open(filePath, std::ios::in);

        if (!file) {
            // 如果仍然找不到文件，抛出异常
            throw std::runtime_error("Failed to find file: " + filePath.string());
        }
    }

    return filePath.string();
}

String getModulePathByPath(const String &path, const String &runningFileDirectory)
{
    return getAbsolutePath(runningFileDirectory, path);
}

String getFileDirectory(const String &input)
{
    // 检查输入是否包含路径分隔符
    bool isPath = input.find('/') != String::npos || input.find('\\') != String::npos;

    if (isPath) {
        // 如果是路径，提取目录部分并转换为绝对路径
        std::filesystem::path path(input);
        std::filesystem::path absolutePath = absolute(path);
        return absolutePath.parent_path().string();
    }
    // 如果是文件名，返回运行程序所在的目录
    return getProgramDirectory();
}

String getColorCode(Color color)
{
    switch (color) {
    case Color::Red:
        return "\033[31m"; // 红色
    case Color::Green:
        return "\033[32m"; // 绿色
    case Color::Blue:
        return "\033[34m"; // 蓝色
    case Color::Yellow:
        return "\033[33m"; // 黄色
    case Color::Cyan:
        return "\033[36m"; // 青色
    case Color::Magenta:
        return "\033[35m"; // 洋红色
    case Color::White:
        return "\033[37m"; // 白色
    case Color::Default:
    default:
        return "\033[0m"; // 默认颜色
    }
}

String getColoredString(const String &text, Color color)
{
    String colorCode = getColorCode(color);
    return colorCode + text + "\033[0m"; // 添加颜色代码并重置
}

} // namespace aria
