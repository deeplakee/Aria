#include "scanner.h"

#include "util/util.h"

namespace aria {
Scanner::Scanner(String _source)
    : source{std::move(_source)}
    , current{0}
    , start{0}
    , line{1}
    , had_error{false}
{}

Scanner::~Scanner() = default;

List<Token> Scanner::scan()
{
    List<Token> tokens;
    // add a value before scan,then the first token could have a pretoken
    tokens.emplace_back(TokenType::STRING, -1);
    while (true) {
        tokens.push_back(scanToken(tokens.back()));
        if (tokens.back().getType() == TokenType::CODE_EOF) {
            break;
        }
        if (tokens.back().getType() == TokenType::ERROR) {
            reportScanError(tokens.back());
            break;
        }
    }
    // erase the added pretoken
    tokens.erase(tokens.begin());
    return tokens;
}

inline char Scanner::advance()
{
    current++;
    return source[current - 1];
}

inline bool Scanner::isAtEnd() const
{
    return current >= source.length();
}

inline char Scanner::peek() const
{
    if (isAtEnd())
        return '\0';
    return source[current];
}

inline char Scanner::peekNext() const
{
    if (current + 1 >= source.length())
        return '\0';
    return source[current + 1];
}

bool Scanner::match(char expected)
{
    if (isAtEnd())
        return false;
    if (source[current] != expected)
        return false;

    current++;
    return true;
}

Token Scanner::makeToken(TokenType type) const
{
    return Token{type, line};
}

Token Scanner::makeToken(TokenType type, String msg) const
{
    return Token{type, line, std::move(msg)};
}

Token Scanner::makeNumber()
{
    while (isDigit(peek()))
        advance();

    if (peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek()))
            advance();
    }
    return makeToken(TokenType::NUMBER, source.substr(start, current - start));
}

Token Scanner::makeDecimal()
{
    while (isDigit(peek()))
        advance();
    return makeToken(TokenType::NUMBER, source.substr(start, current - start));
}

Token Scanner::makeString(char front)
{
    std::string value;

    while (peek() != front && !isAtEnd()) {
        if (peek() == '\n')
            line++;
        // Handle escape character
        if (peek() == '\\') {
            advance(); // Skip the backslash
            char escaped = peek();
            switch (escaped) {
            case 'n':
                value += '\n'; // Newline escape
                break;
            case 't':
                value += '\t'; // Tab escape
                break;
            case '\\':
                value += '\\'; // Backslash escape
                break;
            case '\"':
                value += '\"'; // Double quote escape
                break;
            default:
                // If we encounter an unsupported escape sequence, just add the backslash and next char
                value += '\\';
                value += escaped;
                break;
            }
        } else {
            value += peek(); // Regular character
        }

        advance();
    }

    if (isAtEnd())
        return makeToken(TokenType::ERROR, "Unterminated string.");

    // The closing quote
    advance();
    return makeToken(TokenType::STRING, value);
}

Token Scanner::makeIdentifier()
{
    while (isAlpha(peek()) || isDigit(peek()))
        advance();
    String text = source.substr(start, current - start);
    return makeToken(Token::getTokenTypeFromStr(text), text);
}

void Scanner::skipWhitespace(Token &errorToken)
{
    for (;;) {
        char c = peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            line++;
            advance();
            break;
        case '/':
            if (peekNext() == '/') {
                while (peek() != '\n' && !isAtEnd())
                    advance();
            } else if (peekNext() == '*') {
                while (peek() != '*' && peekNext() != '/' && !isAtEnd())
                    advance();
                if (isAtEnd())
                    errorToken = makeToken(TokenType::ERROR, "Unterminated comment.");
                return;
            } else {
                return;
            }
            break;
        case '#':
            while (peek() != '\n' && !isAtEnd())
                advance();
            break;
        default:
            return;
        }
    }
}

void Scanner::reportScanError(Token &errorToken)
{
    had_error = true;
    cerr << format("[line {}] Error: '{}'\n", errorToken.getLine(), errorToken.getText());
}

Token Scanner::scanToken(const Token &previous)
{
    Token errToken = makeToken(TokenType::CODE_EOF);
    skipWhitespace(errToken);
    if (errToken.getType() == TokenType::ERROR) {
        return errToken;
    }
    start = current;
    if (isAtEnd())
        return makeToken(TokenType::CODE_EOF, "at end");

    char c = advance();
    if (isAlpha(c))
        return makeIdentifier();
    if (isDigit(c))
        return makeNumber();

    switch (c) {
    case '(':
        return makeToken(TokenType::LEFT_PAREN, "(");
    case ')':
        return makeToken(TokenType::RIGHT_PAREN, ")");
    case '[':
        return makeToken(TokenType::LEFT_BRACKET, "[");
    case ']':
        return makeToken(TokenType::RIGHT_BRACKET, "]");
    case '{':
        return makeToken(TokenType::LEFT_BRACE, "{");
    case '}':
        return makeToken(TokenType::RIGHT_BRACE, "}");
    case ';':
        return makeToken(TokenType::SEMICOLON, ";");
    case ',':
        return makeToken(TokenType::COMMA, ",");
    case ':':
        return makeToken(TokenType::COLON, ":");
    case '.': {
        if (isDigit(peek())) {
            return makeDecimal();
        }
        if (peek() == '.' && peekNext() == '.') {
            advance();
            advance();
            return makeToken(TokenType::ELLIPSIS, "...");
        }
        return makeToken(TokenType::DOT, ".");
    }
    case '-': {
        if (match('-')) {
            return makeToken(TokenType::MINUS_MINUS, "--");
        }
        if (match('=')) {
            return makeToken(TokenType::MINUS_EQUAL, "-=");
        }
        const TokenType preType = previous.getType();
        if (preType == TokenType::EQUAL_EQUAL || preType == TokenType::NOT_EQUAL ||   //== !=
            preType == TokenType::LESS || preType == TokenType::LESS_EQUAL ||         //<  <=
            preType == TokenType::GREATER || preType == TokenType::GREATER_EQUAL ||   //>  >=
            preType == TokenType::EQUAL || preType == TokenType::LEFT_BRACE ||        //=  {
            preType == TokenType::LEFT_PAREN || preType == TokenType::LEFT_BRACKET || //(  [
            preType == TokenType::PLUS_EQUAL || preType == TokenType::MINUS_EQUAL ||  //+= -=
            preType == TokenType::STAR_EQUAL || preType == TokenType::SLASH_EQUAL ||  //*= /=
            preType == TokenType::PERCENT_EQUAL)                                      //%=
        {
            if (isDigit(peek())) {
                return makeNumber();
            }
            if (match('.')) {
                return makeDecimal();
            }
        }
        return makeToken(TokenType::MINUS, "-");
    }
    case '+': {
        if (match('+')) {
            return makeToken(TokenType::PLUS_PLUS, "++");
        }
        if (match('=')) {
            return makeToken(TokenType::PLUS_EQUAL, "+=");
        }
        return makeToken(TokenType::PLUS, "+");
    }
    case '/': {
        if (match('=')) {
            return makeToken(TokenType::SLASH_EQUAL, "/=");
        }
        return makeToken(TokenType::SLASH, "/");
    }
    case '*': {
        if (match('=')) {
            return makeToken(TokenType::STAR_EQUAL, "*=");
        }
        return makeToken(TokenType::STAR, "*");
    }
    case '%': {
        if (match('=')) {
            return makeToken(TokenType::PERCENT_EQUAL, "%=");
        }
        return makeToken(TokenType::PERCENT, "%");
    }
    case '!': {
        if (match('=')) {
            return makeToken(TokenType::NOT_EQUAL, "!=");
        }
        return makeToken(TokenType::NOT, "!");
    }
    case '=': {
        if (match('=')) {
            return makeToken(TokenType::EQUAL_EQUAL, "==");
        }
        return makeToken(TokenType::EQUAL, "=");
    }
    case '<': {
        if (match('=')) {
            return makeToken(TokenType::LESS_EQUAL, "<=");
        }
        return makeToken(TokenType::LESS, "<");
    }
    case '>': {
        if (match('=')) {
            return makeToken(TokenType::GREATER_EQUAL, ">=");
        }
        return makeToken(TokenType::GREATER, ">");
    }
    case '|':
        if (match('|'))
            return makeToken(TokenType::OR, "||");
        break;
    case '&':
        if (match('&'))
            return makeToken(TokenType::AND, "&&");
        break;
    case '"':
        return makeString('"');
    case '\'':
        return makeString('\'');
    default:
        return makeToken(TokenType::ERROR, "unexpected character: " + String{c});
    }
    return makeToken(TokenType::ERROR, "unexpected character: " + String{c});
}

int Scanner::getLine() const
{
    return line;
}

bool Scanner::hadError() const
{
    return had_error;
}
} // namespace aria
