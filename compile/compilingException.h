#ifndef COMPILINGEXCEPTION_H
#define COMPILINGEXCEPTION_H

#include <exception>

#include "common.h"

namespace aria {
class CompilingException : public std::exception
{
public:
    explicit CompilingException(const std::string &message, int line)
        : message(format("[line {}] Error: ", line) + message)
    {}

    const char *what() const noexcept override { return message.c_str(); }

private:
    String message;
};
} // namespace aria

#endif // COMPILINGEXCEPTION_H
