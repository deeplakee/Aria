#include "token.h"

namespace aria {
Token::Token(TokenType _type, int _line)
    : type{_type}
    , line{_line}
{}

Token::Token(TokenType _type, int _line, String _text)
    : type{_type}
    , text{std::move(_text)}
    , line{_line}
{}

TokenType Token::getType() const
{
    return type;
}

int Token::getLine() const
{
    return line;
}

const String &Token::getText()
{
    return text;
}

inline String Token::typeStr() const
{
    return tokenText[static_cast<int>(type)];
}

String Token::toString()
{
    return format("{:6} {:20} '{}'\n", line, typeStr(), text);
}

TokenType Token::getTokenTypeFromStr(const String &text)
{
    auto it = keywords.find(text);
    if (it == keywords.end()) {
        return TokenType::IDENTIFIER;
    }
    return it->second;
}

Map<String, TokenType> Token::keywords = {
    {"and", TokenType::AND},           /**< "and" 关键字对应的标记类型。 */
    {"as", TokenType::AS},             /**< "as" 关键字对应的标记类型。 */
    {"break", TokenType::BREAK},       /**< "break" 关键字对应的标记类型。 */
    {"catch", TokenType::CATCH},       /**< "catch" 关键字对应的标记类型。 */
    {"class", TokenType::CLASS},       /**< "class" 关键字对应的标记类型。 */
    {"continue", TokenType::CONTINUE}, /**< "continue" 关键字对应的标记类型。 */
    {"else", TokenType::ELSE},         /**< "else" 关键字对应的标记类型。 */
    {"false", TokenType::FALSE},       /**< "false" 关键字对应的标记类型。 */
    {"for", TokenType::FOR},           /**< "for" 关键字对应的标记类型。 */
    {"fun", TokenType::FUN},           /**< "function" 关键字对应的标记类型。 */
    {"if", TokenType::IF},             /**< "if" 关键字对应的标记类型。 */
    {"in", TokenType::IN},             /**< "if" 关键字对应的标记类型。 */
    {"import", TokenType::IMPORT},     /**< "import" 关键字对应的标记类型。 */
    {"nil", TokenType::NIL},           /**< "nil" 关键字对应的标记类型。 */
    {"not", TokenType::NOT},           /**< "not" 关键字对应的标记类型。 */
    {"or", TokenType::OR},             /**< "or" 关键字对应的标记类型。 */
    {"print", TokenType::PRINT},       /**< "print" 关键字对应的标记类型。 */
    {"return", TokenType::RETURN},     /**< "return" 关键字对应的标记类型。 */
    {"super", TokenType::SUPER},       /**< "super" 关键字对应的标记类型。 */
    {"this", TokenType::THIS},         /**< "this" 关键字对应的标记类型。 */
    {"throw", TokenType::THROW},       /**< "throw" 关键字对应的标记类型。 */
    {"true", TokenType::TRUE},         /**< "true" 关键字对应的标记类型。 */
    {"try", TokenType::TRY},           /**< "try" 关键字对应的标记类型。 */
    {"var", TokenType::VAR},           /**< "var" 关键字对应的标记类型。 */
    {"while", TokenType::WHILE},       /**< "while" 关键字对应的标记类型。 */
};

const char *Token::tokenText[] = {
    // Single-character tokens. 单字符词法
    "TOKEN_LEFT_PAREN",    ///< 左括号 (，用于表示代码中的分组或函数调用。
    "TOKEN_RIGHT_PAREN",   ///< 右括号 )，用于表示代码中的分组或函数调用。
    "TOKEN_LEFT_BRACE",    ///< 左花括号 {，用于表示代码中的代码块的开始。
    "TOKEN_RIGHT_BRACE",   ///< 右花括号 }，用于表示代码中的代码块的结束。
    "TOKEN_LEFT_BRACKET",  ///< 左方括号 [，用于表示数组下标访问或其他用途。
    "TOKEN_RIGHT_BRACKET", ///< 右方括号 ]，用于表示数组下标访问或其他用途。
    "TOKEN_COMMA",         ///< 逗号 ,，用于分隔函数参数或数组元素等。
    "TOKEN_DOT",           ///< 点 .，用于访问对象的属性或方法。
    "TOKEN_COLON",         ///< 冒号 :, 用于类继承和哈希表初始化
    "TOKEN_MINUS",         ///< 减号 -，用于表示减法操作。
    "TOKEN_PLUS",          ///< 加号 +，用于表示加法操作。
    "TOKEN_SEMICOLON",     ///< 分号 ;，用于表示语句的结束。
    "TOKEN_SLASH",         ///< 斜杠 /，用于表示除法操作。
    "TOKEN_STAR",          ///< 星号 *，用于表示乘法操作。
    "TOKEN_MOD",           ///< 百分号 %，用于表示取模操作。
    "ELLIPSIS",            ///< 省略号 ...，用于表示不定参数个数。
    "TOKEN_INCREMENT",     ///< 双加号 ++，用于表示自增操作。
    "TOKEN_DECREMENT",     ///< 双减号 --，用于表示自减操作。

    // One or two character tokens. 一或两字符词法
    "TOKEN_NOT_EQUAL",     ///< 感叹号等号 !=，用于表示不等于比较操作。
    "TOKEN_EQUAL",         ///< 单个等号 =，用于赋值操作。
    "TOKEN_EQUAL_EQUAL",   ///< 双等号 ==，用于相等比较操作。
    "TOKEN_GREATER",       ///< 大于号 >，用于大于比较操作。
    "TOKEN_GREATER_EQUAL", ///< 大于等于号 >=，用于大于等于比较操作。
    "TOKEN_LESS",          ///< 小于号 <，用于小于比较操作。
    "TOKEN_LESS_EQUAL",    ///< 小于等于号 <=，用于小于等于比较操作。
    "PLUS_EQUAL",    ///< 加等于号 +=，用于加法操作
    "MINUS_EQUAL",    ///< 减等于号 -=，用于减法操作
    "STAR_EQUAL",    ///< 乘等于号 *=，用于乘法操作
    "SLASH_EQUAL",    ///< 除等于号 /=，用于除法操作
    "PERCENT_EQUAL",    ///< 模等于号 %=，用于取模操作

    // Literals. 字面量
    "TOKEN_IDENTIFIER", ///< 标识符，用于表示变量名、函数名等
    "TOKEN_STRING",     ///< 字符串字面量，表示一串文本。
    "TOKEN_NUMBER",     ///< 数字字面量，表示数字。

    // Keywords. 关键字||三个逻辑运算也是符号
    "TOKEN_AND",      ///< and &&
    "TOKEN_AS",       ///< as
    "TOKEN_BREAK",    ///< break
    "TOKEN_CATCH",    ///< catch
    "TOKEN_CLASS",    ///< class
    "TOKEN_CONTINUE", ///< continue
    "TOKEN_ELSE",     ///< else
    "TOKEN_FALSE",    ///< false
    "TOKEN_FOR",      ///< for
    "TOKEN_FUN",      ///< fun
    "TOKEN_IF",       ///< if
    "TOKEN_IN",       ///< in
    "TOKEN_IMPORT",   ///< import
    "TOKEN_NIL",      ///< nil
    "TOKEN_NOT",      ///< not !
    "TOKEN_OR",       ///< or ||
    "TOKEN_PRINT",    ///< print
    "TOKEN_RETURN",   ///< return
    "TOKEN_SUPER",    ///< super
    "TOKEN_THIS",     ///< this
    "TOKEN_THROW",    ///< throw
    "TOKEN_TRUE",     ///< true
    "TOKEN_TRY",      ///< try
    "TOKEN_VAR",      ///< var
    "TOKEN_WHILE",    ///< while

    "TOKEN_ERROR",
    "TOKEN_EOF" ///< EOF,表示已经到达代码文件的末尾。
};

} // namespace aria
