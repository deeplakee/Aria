#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

namespace aria {
class Scanner
{
public:
    explicit Scanner(String _source);
    ~Scanner();

    List<Token> scan();
    Token scanToken(const Token &previous);
    int getLine() const;
    bool hadError() const;

private:
    String source;
    int current;
    int start;
    int line;
    bool had_error;

    inline char advance();

    inline bool isAtEnd() const;

    inline char peek() const;

    inline char peekNext() const;

    bool match(char expected);

    Token makeToken(TokenType type) const;

    Token makeToken(TokenType type, String msg) const;

    Token makeNumber();

    Token makeDecimal();

    Token makeString(char front);

    Token makeIdentifier();

    void skipWhitespace(Token &errorToken);

    void reportScanError(Token &beforeErrorToken);
};
} // namespace aria

#endif //SCANNER_H
