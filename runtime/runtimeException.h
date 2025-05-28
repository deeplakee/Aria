#ifndef RUNTIMEEXCEPTION_H
#define RUNTIMEEXCEPTION_H

#include <exception>

#include "common.h"

namespace aria {
class RuntimeException : public std::exception
{
public:
    explicit RuntimeException(const char *str)
        : message{str}
    {}

    const char *what() const noexcept override { return message.c_str(); }

private:
    String message;
};
} // namespace aria

#endif //RUNTIMEEXCEPTION_H
