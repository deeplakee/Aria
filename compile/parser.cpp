#include "parser.h"

namespace aria {
Parser::Parser(List<Token> _tokens)
    : current{0}
    , tokens{std::move(_tokens)}
    , hadError{false}
    , panicMode{false}
{}

Parser::~Parser() = default;

UniquePtr<ASTNode> Parser::parse()
{
    return parseProgram();
}

bool Parser::hasError() const
{
    return hadError;
}

void Parser::reportParseError(Token &errorToken, StringView msg)
{
    hadError = true;
    panicMode = true;
    cerr << format("[line {}] Error", errorToken.getLine());
    cerr << format(" at '{}':", errorToken.getText());
    cerr << msg << '\n';
}

bool Parser::match(TokenType t)
{
    if (tokens[current].getType() == t) {
        ++current;
        return true;
    }
    return false;
}

bool Parser::check(TokenType t) const
{
    if (tokens[current].getType() == t) {
        return true;
    }
    return false;
}

bool Parser::consume(TokenType t, StringView msg)
{
    if (match(t))
        return true;
    reportParseError(tokens[current], msg);
    return false;
}

void Parser::synchronize()
{
    panicMode = false;
    if (tokens[current - 1].getType() == TokenType::SEMICOLON) {
        return;
    }
    auto tokentype = tokens[current].getType();
    while (tokentype != TokenType::CODE_EOF) {
        switch (tokentype) {
        case TokenType::SEMICOLON:
            current++;
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::BREAK:
        case TokenType::CONTINUE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        default:
            current++;
            tokentype = tokens[current].getType();
        }
    }
}

// program        → declaration*
UniquePtr<ASTNode> Parser::parseProgram()
{
    int line = tokens[current].getLine();
    List<UniquePtr<ASTNode>> declarations;
    while (!check(TokenType::CODE_EOF)) {
        declarations.emplace_back(parseDeclaration());
    }
    return std::make_unique<ProgramNode>(std::move(declarations), line);
}

// declaration    → funDecl | classDecl | varDecl | statement
UniquePtr<ASTNode> Parser::parseDeclaration()
{
    UniquePtr<ASTNode> declaration = nullptr;
    if (match(TokenType::VAR)) {
        declaration = parseVarDecl();
    } else if (match(TokenType::FUN)) {
        declaration = parseFunDecl();
    } else if (match(TokenType::CLASS)) {
        declaration = parseClassDecl();
    } else {
        declaration = parseStatement();
    }
    if (panicMode) {
        synchronize();
    }
    return declaration;
}

// funDecl        → "fun" IDENTIFIER "(" parameters? ")" block
UniquePtr<ASTNode> Parser::parseFunDecl()
{
    int line = tokens[current].getLine(); // line of function name
    String name = tokens[current++].getText();
    List<String> params;
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    bool acceptsVarargs = false;
    if (!check(TokenType::RIGHT_PAREN)) {
        int arity = 0;
        do {
            if (arity >= 255) {
                reportParseError(tokens[current], "Can't have more than 255 parameters.");
            }
            if (match(TokenType::ELLIPSIS)) {
                acceptsVarargs = true;
                consume(TokenType::IDENTIFIER, "Expect parameter name.");
                arity++;
                params.emplace_back(tokens[current - 1].getText());
                break;
            }
            if (!consume(TokenType::IDENTIFIER, "Expect parameter name.")) {
                break;
            }
            arity++;
            params.emplace_back(tokens[current - 1].getText());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    UniquePtr<ASTNode> body = parseBlock();
    int endLine = tokens[current - 1].getLine();

    return std::make_unique<FunDeclNode>(
        std::move(name), std::move(params), std::move(body), acceptsVarargs, endLine, line);
}

// classDecl      → "class" IDENTIFIER "{" "}"
UniquePtr<ASTNode> Parser::parseClassDecl()
{
    int line = tokens[current - 1].getLine(); // line of token "class"
    if (!consume(TokenType::IDENTIFIER, "Expect class name")) {
        return nullptr;
    }
    String name = tokens[current - 1].getText();
    String superName;

    if (match(TokenType::COLON)) {
        if (!consume(TokenType::IDENTIFIER, "Expect class name")) {
            return nullptr;
        }
        superName = tokens[current - 1].getText();
        if (superName == name) {
            reportParseError(tokens[current - 1], "A class can't inherit from itself.");
        }
    }

    consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

    List<UniquePtr<ASTNode>> methods;

    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::CODE_EOF)) {
        if (!check(TokenType::IDENTIFIER)) {
            reportParseError(tokens[current], "Expect method name.");
        }
        methods.emplace_back(parseFunDecl());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    int endLine = tokens[current - 1].getLine();

    return std::make_unique<ClassDeclNode>(
        std::move(name), std::move(superName), std::move(methods), endLine, line);
}

// varDecl        → "var" IDENTIFIER ( "=" expression )? ( "," IDENTIFIER ( "=" expression )? )* ";"
UniquePtr<ASTNode> Parser::parseVarDecl()
{
    int line = tokens[current - 1].getLine(); // line of token "var"
    List<String> names;
    List<UniquePtr<ASTNode>> exprs;

    for (;;) {
        // read variable name
        if (!consume(TokenType::IDENTIFIER, "Expect a variable name")) {
            return nullptr;
        }
        names.emplace_back(tokens[current - 1].getText());

        // read initializer
        int currentLine = tokens[current - 1].getLine();
        if (match(TokenType::EQUAL)) {
            exprs.emplace_back(parseExpression());
        } else {
            exprs.emplace_back(std::make_unique<NilNode>(currentLine));
        }

        // read next variable declare or ';'
        if (match(TokenType::COMMA)) {
            // nothing to do
        } else if (match(TokenType::SEMICOLON)) {
            break;
        } else {
            reportParseError(tokens[current], "Expected ',' or ';' here");
            break;
        }
    }

    return std::make_unique<VarDeclNode>(std::move(names), std::move(exprs), line);
}

// statement      → printStmt | ifStmt | whileStmt | forStmt | breakStmt | continueStmt | returnStmt | block | exprStmt
UniquePtr<ASTNode> Parser::parseStatement()
{
    if (match(TokenType::PRINT)) {
        return parsePrintStmt();
    }
    if (match(TokenType::IMPORT)) {
        return parseImportStmt();
    }
    if (match(TokenType::IF)) {
        return parseIfStmt();
    }
    if (match(TokenType::WHILE)) {
        return parseWhileStmt();
    }
    if (match(TokenType::FOR)) {
        return parseForStmt();
    }
    if (match(TokenType::TRY)) {
        return parseTryCatchStmt();
    }
    if (match(TokenType::THROW)) {
        return parseThrowStmt();
    }
    if (match(TokenType::BREAK)) {
        return parseBreakStmt();
    }
    if (match(TokenType::CONTINUE)) {
        return parseContinueStmt();
    }
    if (match(TokenType::RETURN)) {
        return parseReturnStmt();
    }
    if (match(TokenType::LEFT_BRACE)) {
        return parseBlock();
    }
    if (match(TokenType::RIGHT_BRACE)) {
        reportParseError(tokens[current], "unexpected token at the begin of statement");
        current++;
        return nullptr;
    }
    return parseExprStmt();
}

// printStmt      → "print" expression ";"
UniquePtr<ASTNode> Parser::parsePrintStmt()
{
    int line = tokens[current - 1].getLine();
    UniquePtr<ASTNode> expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';'");
    return std::make_unique<PrintStmtNode>(std::move(expr), line);
}

// importStmt     → "import" IDENTIFIER ( "as" IDENTIFIER )? ";"
//                | "import" STRING "as" IDENTIFIER ";" UniquePtr<ASTNode> Parser::parseImportStmt()
UniquePtr<ASTNode> Parser::parseImportStmt()
{
    int line = tokens[current - 1].getLine();
    String input;
    if (match(TokenType::IDENTIFIER)) {
        input = tokens[current - 1].getText();
        String moduleName = input;
        if (match(TokenType::AS)) {
            consume(TokenType::IDENTIFIER, "Expect a new module name.");
            moduleName = tokens[current - 1].getText();
        }
        consume(TokenType::SEMICOLON, "Expected ';' after module name.");
        return std::make_unique<ImportStmtNode>(input, moduleName, line);
    }
    if (match(TokenType::STRING)) {
        input = tokens[current - 1].getText();
        String moduleName;
        if (match(TokenType::AS)) {
            consume(TokenType::IDENTIFIER, "Expect a new module name.");
            moduleName = tokens[current - 1].getText();
        } else {
            reportParseError(tokens[current], "Expected a module name for imported module.");
        }
        consume(TokenType::SEMICOLON, "Expected ';' after module name.");
        return std::make_unique<ImportStmtNode>(input, moduleName, line);
    }
    reportParseError(tokens[current], "Expected identifier or string as module name.");
    return std::make_unique<ErrorNode>(line);
}

// ifStmt         → "if" "(" expression ")" statement ( "else" statement )?
UniquePtr<ASTNode> Parser::parseIfStmt()
{
    int line = tokens[current - 1].getLine();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    UniquePtr<ASTNode> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    UniquePtr<ASTNode> body = parseStatement();
    UniquePtr<ASTNode> elseBody = nullptr;
    if (match(TokenType::ELSE)) {
        elseBody = parseStatement();
    }
    return std::make_unique<IfStmtNode>(
        std::move(condition), std::move(body), std::move(elseBody), line);
}

// whileStmt      → "while" "(" expression ")" statement
UniquePtr<ASTNode> Parser::parseWhileStmt()
{
    int line = tokens[current - 1].getLine();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    UniquePtr<ASTNode> condition = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    UniquePtr<ASTNode> body = parseStatement();
    int endLine = tokens[current - 1].getLine();
    return std::make_unique<WhileStmtNode>(std::move(condition), std::move(body), endLine, line);
}

// forStmt        → "for" "(" ( varDecl | exprStmt | ";" ) expression? ";" expression? ")" statement
UniquePtr<ASTNode> Parser::parseForStmt()
{
    int line = tokens[current - 1].getLine();
    UniquePtr<ASTNode> varInit = nullptr;
    UniquePtr<ASTNode> condition = nullptr;
    UniquePtr<ASTNode> increment = nullptr;

    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    if (tokens[current].getType() == TokenType::VAR
        && tokens[current + 1].getType() == TokenType::IDENTIFIER
        && tokens[current + 2].getType() == TokenType::IN) {
        return parseForInStmt();
    }

    if (match(TokenType::SEMICOLON)) {
        // No initializer.
    } else if (match(TokenType::VAR)) {
        varInit = parseVarDecl();
    } else {
        varInit = parseExprStmt();
    }

    if (!match(TokenType::SEMICOLON)) {
        condition = parseExpression();
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
    }

    if (!match(TokenType::RIGHT_PAREN)) {
        increment = parseExpression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");
    }

    UniquePtr<ASTNode> body = parseStatement();
    int endLine = tokens[current - 1].getLine();
    return std::make_unique<ForStmtNode>(
        std::move(varInit),
        std::move(condition),
        std::move(increment),
        std::move(body),
        endLine,
        line);
}

// forInStmt        → "for" "(" "var" IDENTIFIER "in" expression ")" statement
UniquePtr<ASTNode> Parser::parseForInStmt()
{
    // now we reach token "var"
    int line = tokens[current - 2].getLine(); // line of "for"

    String iterName = tokens[current + 1].getText();
    current += 3;
    UniquePtr<ASTNode> expr = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for expression.");
    UniquePtr<ASTNode> body = parseStatement();
    int endLine = tokens[current - 1].getLine();
    return std::make_unique<ForInStmtNode>(iterName, std::move(expr), std::move(body), endLine, line);
}

// tryCatchStmt   → "try" block "catch" "(" IDENTIFIER ")" block
UniquePtr<ASTNode> Parser::parseTryCatchStmt()
{
    int line = tokens[current - 1].getLine();
    consume(TokenType::LEFT_BRACE, "Expect '{' after 'try'.");
    UniquePtr<ASTNode> tryBody = parseBlock();
    consume(TokenType::CATCH, "Expect 'catch' after try body.");
    int catchLine = tokens[current - 1].getLine();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'catch'.");
    consume(TokenType::IDENTIFIER, "Expect a exception name after '('.");
    String exceptionName = tokens[current - 1].getText();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after exception name.");
    consume(TokenType::LEFT_BRACE, "Expect '{' after ')'.");
    UniquePtr<ASTNode> catchBody = parseBlock();
    int endLine = tokens[current - 1].getLine();

    return std::make_unique<TryCatchStmtNode>(
        std::move(tryBody), std::move(catchBody), std::move(exceptionName), catchLine, endLine, line);
}

// throwStmt      → "throw" expression ";"
UniquePtr<ASTNode> Parser::parseThrowStmt()
{
    int line = tokens[current - 1].getLine();
    UniquePtr<ASTNode> e = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ThrowStmtNode>(std::move(e), line);
}

// breakStmt      → "break"";"
UniquePtr<ASTNode> Parser::parseBreakStmt()
{
    int line = tokens[current - 1].getLine();
    consume(TokenType::SEMICOLON, "Expected ';' after 'break'.");
    return std::make_unique<BreakStmtNode>(line);
}

// continueStmt   → "continue"";"
UniquePtr<ASTNode> Parser::parseContinueStmt()
{
    int line = tokens[current - 1].getLine();
    consume(TokenType::SEMICOLON, "Expected ';' after 'continue'.");
    return std::make_unique<ContinueStmtNode>(line);
}

// returnStmt     → "return" expression? ";"
UniquePtr<ASTNode> Parser::parseReturnStmt()
{
    int line = tokens[current - 1].getLine(); // line of token "return"
    UniquePtr<ASTNode> expr = nullptr;
    if (match(TokenType::SEMICOLON)) {
        expr = std::make_unique<NilNode>(line);
        return std::make_unique<ReturnStmtNode>(std::move(expr), line);
    }
    expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmtNode>(std::move(expr), line);
}

// exprStmt       → expression ";"
UniquePtr<ASTNode> Parser::parseExprStmt()
{
    int line = tokens[current].getLine();
    UniquePtr<ASTNode> expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';'");

    return std::make_unique<ExprStmtNode>(std::move(expr), line);
}

// block          → "{" declaration* "}"
UniquePtr<ASTNode> Parser::parseBlock()
{
    int line = tokens[current - 1].getLine(); // line of token "{"
    List<UniquePtr<ASTNode>> declarations;
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::CODE_EOF)) {
        declarations.emplace_back(parseDeclaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    int endLine = tokens[current - 1].getLine();

    return std::make_unique<BlockNode>(std::move(declarations), endLine, line);
}

// expression     → assignment
UniquePtr<ASTNode> Parser::parseExpression()
{
    return parseAssignment();
}

// assignment     → lvalue ( "=" | "+=" | "-=" | "*=" | "/=" | "%=" ) assignment | logic_or
UniquePtr<ASTNode> Parser::parseAssignment()
{
    int line = tokens[current].getLine();
    const int current_backup = current;
    UniquePtr<ASTNode> lvalue = parseLvalue();

    if (match(TokenType::PLUS_EQUAL) || match(TokenType::MINUS_EQUAL)
        || match(TokenType::STAR_EQUAL) || match(TokenType::SLASH_EQUAL)
        || match(TokenType::PERCENT_EQUAL) || match(TokenType::PLUS_PLUS)
        || match(TokenType::MINUS_MINUS)) {
        ASTNode *rawPtr = lvalue.get();
        ErrorNode *errorNode = dynamic_cast<ErrorNode *>(rawPtr);
        if (errorNode != nullptr) {
            reportParseError(tokens[current - 1], "Invalid assignment target");
        }

        line = tokens[current - 1].getLine();
        TokenType op = tokens[current - 1].getType();

        const int new_current = current;
        current = current_backup;
        UniquePtr<ASTNode> exprLeft = parseRvalue();
        UniquePtr<ASTNode> exprRight = nullptr;
        current = new_current;
        if (op == TokenType::PLUS_PLUS || op == TokenType::MINUS_MINUS) {
            exprRight = std::make_unique<NumberNode>(1, line);
        } else {
            exprRight = parseAssignment();
        }
        UniquePtr<ASTNode> expr
            = std::make_unique<BinaryOpNode>(std::move(exprLeft), op, std::move(exprRight), line);
        return std::make_unique<AssignNode>(std::move(lvalue), std::move(expr), line);
    }

    if (match(TokenType::EQUAL)) {
        ASTNode *rawPtr = lvalue.get();
        ErrorNode *errorNode = dynamic_cast<ErrorNode *>(rawPtr);
        if (errorNode != nullptr) {
            reportParseError(tokens[current - 1], "Invalid assignment target");
        }
        return std::make_unique<AssignNode>(std::move(lvalue), parseAssignment(), line);
    }
    current = current_backup;
    return parseLogicOr();
}

// logic_or       → logic_and ( ( "or" | "||" ) logic_and )*
UniquePtr<ASTNode> Parser::parseLogicOr()
{
    UniquePtr<ASTNode> left = parseLogicAnd();
    UniquePtr<ASTNode> right = nullptr;
    for (;;) {
        TokenType opType = tokens[current].getType();
        int line = tokens[current].getLine();
        if (opType == TokenType::OR) {
            current++;
            right = parseLogicAnd();
            left = std::make_unique<BinaryOpNode>(std::move(left), opType, std::move(right), line);
        } else {
            break;
        }
    }
    return left;
}

// logic_and      → equality ( ( "and" | "&&" ) equality )*
UniquePtr<ASTNode> Parser::parseLogicAnd()
{
    UniquePtr<ASTNode> left = parseEquality();
    UniquePtr<ASTNode> right = nullptr;
    for (;;) {
        TokenType opType = tokens[current].getType();
        int line = tokens[current].getLine();
        if (opType == TokenType::AND) {
            current++;
            right = parseEquality();
            left = std::make_unique<BinaryOpNode>(std::move(left), opType, std::move(right), line);
        } else {
            break;
        }
    }
    return left;
}

// lvalue         → IDENTIFIER
//                | lproperty
//                | lthis
UniquePtr<ASTNode> Parser::parseLvalue()
{
    Token &firstToken = tokens[current];
    if (match(TokenType::IDENTIFIER)) {
        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)
            || check(TokenType::LEFT_PAREN)) {
            UniquePtr<ASTNode> instance
                = std::make_unique<LoadVarNode>(firstToken.getText(), firstToken.getLine());
            return parseLProperty(std::move(instance));
        }
        return std::make_unique<StoreVarNode>(firstToken.getText(), firstToken.getLine());
    }
    if (match(TokenType::THIS)) {
        return parseLThis();
    }
    return nullptr;
}

// lproperty      → IDENTIFIER ( "." invokeMethod | "." IDENTIFIER | "[" expression "]" | args )+
UniquePtr<ASTNode> Parser::parseLProperty(UniquePtr<ASTNode> instance)
{
    if (match(TokenType::DOT)) {
        consume(TokenType::IDENTIFIER, "Expect a property name after '.'.");
        String name = tokens[current - 1].getText();
        int line = tokens[current - 2].getLine();
        UniquePtr<ASTNode> loadProperty = std::move(instance);

        // invokeMethod
        if (match(TokenType::LEFT_PAREN)) {
            loadProperty = parseInvokeMethod(std::move(loadProperty), name);
            if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)
                || check(TokenType::LEFT_PAREN)) {
                return parseLProperty(std::move(loadProperty));
            }
            return std::make_unique<ErrorNode>(tokens[current - 1].getLine());
        }

        // lproperty
        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)
            || check(TokenType::LEFT_PAREN)) {
            loadProperty
                = std::make_unique<LoadPropertyNode>(std::move(loadProperty), std::move(name), line);
            return parseLProperty(std::move(loadProperty));
        }
        // reach end
        return std::make_unique<StorePropertyNode>(std::move(loadProperty), std::move(name), line);
    }

    if (match(TokenType::LEFT_BRACKET)) {
        int line = tokens[current - 1].getLine();
        UniquePtr<ASTNode> index = parseExpression();
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)
            || check(TokenType::LEFT_PAREN)) {
            UniquePtr<ASTNode> indexAccess
                = std::make_unique<LoadSubscrNode>(std::move(instance), std::move(index), line);
            return parseLProperty(std::move(indexAccess));
        }
        return std::make_unique<StoreSubscrNode>(std::move(instance), std::move(index), line);
    }

    if (match(TokenType::LEFT_PAREN)) {
        UniquePtr<ASTNode> call = parseCall(std::move(instance));
        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)) {
            return parseLProperty(std::move(call));
        }
        return std::make_unique<ErrorNode>(tokens[current - 1].getLine());
    }

    // check '[' and '.' and '(' before calling this function ,so the following code will not be run
    reportParseError(tokens[current], "error in parsing lproperty");
    return nullptr;
}

// lthis          → "this" "." IDENTIFIER
//                | "this" "." lproperty
UniquePtr<ASTNode> Parser::parseLThis()
{
    UniquePtr<ASTNode> loadThis
        = std::make_unique<LoadVarNode>("this", tokens[current - 1].getLine());

    // consume(TokenType::DOT, "Expect '.' after 'this'.");
    if (!match(TokenType::DOT)) {
        return std::make_unique<ErrorNode>(tokens[current].getLine());
    }

    if (!match(TokenType::IDENTIFIER)) {
        reportParseError(tokens[current], "Expect a property name after '.'.");
        return nullptr;
    }
    String property = tokens[current - 1].getText();
    int line = tokens[current - 2].getLine();

    if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET) || check(TokenType::LEFT_PAREN)) {
        UniquePtr<ASTNode> loadProperty
            = std::make_unique<LoadPropertyNode>(std::move(loadThis), std::move(property), line);
        return parseLProperty(std::move(loadProperty));
    }
    return std::make_unique<StorePropertyNode>(std::move(loadThis), std::move(property), line);
}

// equality       → comparison ( ( "!=" | "==" ) comparison )*
UniquePtr<ASTNode> Parser::parseEquality()
{
    UniquePtr<ASTNode> left = parseComparison();
    UniquePtr<ASTNode> right = nullptr;
    for (;;) {
        TokenType opType = tokens[current].getType();
        int line = tokens[current].getLine();
        if (opType == TokenType::NOT_EQUAL || opType == TokenType::EQUAL_EQUAL) {
            current++;
            right = parseComparison();
            left = std::make_unique<BinaryOpNode>(std::move(left), opType, std::move(right), line);
        } else {
            break;
        }
    }
    return left;
}

// comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )*
UniquePtr<ASTNode> Parser::parseComparison()
{
    UniquePtr<ASTNode> left = parseTerm();
    UniquePtr<ASTNode> right = nullptr;
    for (;;) {
        TokenType opType = tokens[current].getType();
        int line = tokens[current].getLine();
        if (opType == TokenType::GREATER || opType == TokenType::GREATER_EQUAL
            || opType == TokenType::LESS || opType == TokenType::LESS_EQUAL) {
            current++;
            right = parseTerm();
            left = std::make_unique<BinaryOpNode>(std::move(left), opType, std::move(right), line);
        } else {
            break;
        }
    }
    return left;
}

// term           → factor ( ( "-" | "+" ) factor )*
UniquePtr<ASTNode> Parser::parseTerm()
{
    UniquePtr<ASTNode> left = parseFactor();
    UniquePtr<ASTNode> right = nullptr;
    for (;;) {
        TokenType opType = tokens[current].getType();
        int line = tokens[current].getLine();
        if (opType == TokenType::PLUS || opType == TokenType::MINUS) {
            current++;
            right = parseFactor();
            left = std::make_unique<BinaryOpNode>(std::move(left), opType, std::move(right), line);
        } else {
            break;
        }
    }
    return left;
}

// factor         → unary ( ( "/" | "*" | "%" ) unary )*
UniquePtr<ASTNode> Parser::parseFactor()
{
    UniquePtr<ASTNode> left = parseUnary();
    UniquePtr<ASTNode> right = nullptr;
    for (;;) {
        TokenType opType = tokens[current].getType();
        int line = tokens[current].getLine();
        if (opType == TokenType::STAR || opType == TokenType::SLASH
            || opType == TokenType::PERCENT) {
            current++;
            right = parseUnary();
            left = std::make_unique<BinaryOpNode>(std::move(left), opType, std::move(right), line);
        } else {
            break;
        }
    }
    return left;
}

// unary          → ( "-" | "!" | "not" ) unary | value
UniquePtr<ASTNode> Parser::parseUnary()
{
    TokenType opType = tokens[current].getType();
    int line = tokens[current].getLine();
    if (opType == TokenType::NOT || opType == TokenType::MINUS) {
        current++;
        return std::make_unique<UnaryOpNode>(opType, std::move(parseUnary()), line);
    }

    return parseValue();
}

// if (match(TokenType::PLUS_PLUS) || match(TokenType::MINUS_MINUS)) {
//     return std::make_unique<GetSetNode>(
//         tokens[current - 1].getType(),
//         varName,
//         tokens[current - 1].getLine());
// }

// value          → primary | rvalue
UniquePtr<ASTNode> Parser::parseValue()
{
    if (check(TokenType::IDENTIFIER) || check(TokenType::THIS) || check(TokenType::SUPER)) {
        return parseRvalue();
    }
    return parsePrimary();
}

// rvalue         → IDENTIFIER
//                | rproperty
//                | rthis
//                | super
UniquePtr<ASTNode> Parser::parseRvalue()
{
    if (match(TokenType::THIS)) {
        return parseRThis();
    }
    if (match(TokenType::SUPER)) {
        return parseSuper();
    }
    int line = tokens[current].getLine();
    String varName = tokens[current].getText();
    UniquePtr<ASTNode> loadVarNode = std::make_unique<LoadVarNode>(varName, line);
    current++;
    if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET) || check(TokenType::LEFT_PAREN)) {
        return parseRProperty(std::move(loadVarNode));
    }
    return loadVarNode;
}

// args           → ( "(" ( expression ("," expression )* )? ")" ) +
UniquePtr<ASTNode> Parser::parseCall(UniquePtr<ASTNode> callee)
{
    int count = 0;
    List<UniquePtr<ASTNode>> args;
    do {
        if (check(TokenType::RIGHT_PAREN)) {
            break;
        }
        if (count >= UINT8_MAX) {
            reportParseError(tokens[current], "Can't have more than 255 arguments.");
        }
        args.emplace_back(parseExpression());
        count++;
    } while (match(TokenType::COMMA));
    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    callee = std::make_unique<CallNode>(
        std::move(callee), std::move(args), tokens[current - 1].getLine());
    if (match(TokenType::LEFT_PAREN)) {
        return parseCall(std::move(callee));
    }
    return callee;
}

// rproperty      → IDENTIFIER ( "." callMethod | "." IDENTIFIER | "[" expression "]" | args )+
UniquePtr<ASTNode> Parser::parseRProperty(UniquePtr<ASTNode> loadInstance)
{
    if (match(TokenType::DOT)) {
        consume(TokenType::IDENTIFIER, "Expect a property name after '.'.");
        String property = tokens[current - 1].getText();
        UniquePtr<ASTNode> loadProperty = nullptr;

        if (match(TokenType::LEFT_PAREN)) {
            loadProperty = parseInvokeMethod(std::move(loadInstance), property);
        } else {
            loadProperty = std::make_unique<LoadPropertyNode>(
                std::move(loadInstance), property, tokens[current - 2].getLine());
        }

        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)
            || check(TokenType::LEFT_PAREN)) {
            return parseRProperty(std::move(loadProperty));
        }
        return loadProperty;
    }

    if (match(TokenType::LEFT_BRACKET)) {
        int line = tokens[current - 1].getLine();
        UniquePtr<ASTNode> index = parseExpression();
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
        UniquePtr<ASTNode> indexAccess
            = std::make_unique<LoadSubscrNode>(std::move(loadInstance), std::move(index), line);
        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)
            || check(TokenType::LEFT_PAREN)) {
            return parseRProperty(std::move(indexAccess));
        }
        return indexAccess;
    }

    if (match(TokenType::LEFT_PAREN)) {
        UniquePtr<ASTNode> loadProperty = parseCall(std::move(loadInstance));
        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)
            || check(TokenType::LEFT_PAREN)) {
            return parseRProperty(std::move(loadProperty));
        }
        return loadProperty;
    }

    // check '[' and '.' and '(' before calling this function ,so the following code will not be run
    reportParseError(tokens[current], "error in parsing rproperty");
    return nullptr;
}

// callMethod     → IDENTIFIER args+
UniquePtr<ASTNode> Parser::parseInvokeMethod(UniquePtr<ASTNode> instance, String methodName)
{
    int line = tokens[current].getLine();
    int count = 0;
    List<UniquePtr<ASTNode>> args;
    do {
        if (check(TokenType::RIGHT_PAREN)) {
            break;
        }
        if (count >= 255) {
            reportParseError(tokens[current], "Can't have more than 255 arguments.");
        }
        args.emplace_back(parseExpression());
        count++;
    } while (match(TokenType::COMMA));
    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    UniquePtr<ASTNode> callee = std::make_unique<InvokeMethodNode>(
        std::move(instance), std::move(methodName), std::move(args), line);
    if (match(TokenType::LEFT_PAREN)) {
        return parseCall(std::move(callee));
    }
    return callee;
}

// rthis          → "this" "." IDENTIFIER
//                | "this" "." rproperty
//                | "this" "." call
UniquePtr<ASTNode> Parser::parseRThis()
{
    UniquePtr<ASTNode> loadThis
        = std::make_unique<LoadVarNode>("this", tokens[current - 1].getLine());

    // consume(TokenType::DOT, "Expect '.' after 'this'.");
    if (!match(TokenType::DOT)) {
        return loadThis;
    }

    consume(TokenType::IDENTIFIER, "Expect a property name after '.'.");
    String property = tokens[current - 1].getText();
    int line = tokens[current - 2].getLine();
    if (match(TokenType::LEFT_PAREN)) {
        return parseInvokeMethod(std::move(loadThis), property);
    }
    UniquePtr<ASTNode> loadProperty
        = std::make_unique<LoadPropertyNode>(std::move(loadThis), std::move(property), line);
    if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)) {
        return parseRProperty(std::move(loadProperty));
    }
    return loadProperty;
}

// super          → "super" "." IDENTIFIER args?
UniquePtr<ASTNode> Parser::parseSuper()
{
    consume(TokenType::DOT, "Expect '.' after 'super'.");
    consume(TokenType::IDENTIFIER, "Expect superclass method name after '.'.");
    String methodName = tokens[current - 1].getText();
    int line = tokens[current - 1].getLine();
    UniquePtr<ASTNode> loadSuperMethod = std::make_unique<LoadSuperMethodNode>(methodName, line);
    if (match(TokenType::LEFT_PAREN)) {
        return parseCall(std::move(loadSuperMethod));
    }
    return loadSuperMethod;
}

// primary        → NUMBER | "nil" | "false" | "true" | parenExpr | visitable
UniquePtr<ASTNode> Parser::parsePrimary()
{
    TokenType currentTokenType = tokens[current].getType();
    int line = tokens[current].getLine();
    switch (currentTokenType) {
    case TokenType::NUMBER: {
        double num = std::stod(tokens[current].getText());
        current++;
        return std::make_unique<NumberNode>(num, line);
    }
    case TokenType::TRUE: {
        current++;
        return std::make_unique<TrueNode>(line);
    }
    case TokenType::FALSE: {
        current++;
        return std::make_unique<FalseNode>(line);
    }
    case TokenType::STRING:
    case TokenType::LEFT_BRACKET:
    case TokenType::LEFT_BRACE:
        return parseVisitable();
    case TokenType::LEFT_PAREN: {
        current++;
        return parseParenExpr();
    }
    case TokenType::NIL:
        current++;
    default:
        return std::make_unique<NilNode>(line);
    }
}

// parenExpr    → "(" expression ")"
UniquePtr<ASTNode> Parser::parseParenExpr()
{
    auto expr = parseExpression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    return expr;
}

// listExpr     → "[" ( expression ("," expression )* )? "]"
UniquePtr<ASTNode> Parser::parseList()
{
    int count = 0;
    int line = tokens[current].getLine();
    List<UniquePtr<ASTNode>> list;
    do {
        if (check(TokenType::RIGHT_BRACKET)) {
            break;
        }
        if (count >= UINT16_MAX) {
            reportParseError(
                tokens[current], "Can't have more than 65535 elements in a list initialization.");
        }
        list.emplace_back(parseExpression());
        count++;
    } while (match(TokenType::COMMA));
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after elements.");
    return std::make_unique<ListNode>(std::move(list), line);
}

// mapExpr      → "{" ( expression ("," expression )* )? "}"
UniquePtr<ASTNode> Parser::parseMap()
{
    int count = 0;
    List<UniquePtr<ASTNode>> list;
    do {
        if (check(TokenType::RIGHT_BRACE)) {
            break;
        }
        if (count >= UINT8_MAX * 4) {
            reportParseError(
                tokens[current],
                "Can't have more than 1024 key-value pairs in a map initialization.");
        }
        list.emplace_back(parseExpression());
        consume(TokenType::COLON, "Expect ':' after key.");
        list.emplace_back(parseExpression());
        count++;
    } while (match(TokenType::COMMA));
    consume(TokenType::RIGHT_BRACE, "Expect '}' after key-value pairs.");
    return std::make_unique<MapNode>(std::move(list), count);
}

UniquePtr<ASTNode> Parser::parseString()
{
    String str = tokens[current - 1].getText();
    int line = tokens[current - 1].getLine();
    return std::make_unique<StringNode>(str, line);
}

// visitable    → STRING
//              | listExpr
//              | mapExpr
//              | ( STRING | listExpr | mapExpr ) ( "." callMethod | "." IDENTIFIER | "[" expression "]" )+
UniquePtr<ASTNode> Parser::parseVisitable()
{
    UniquePtr<ASTNode> loadInstance = nullptr;

    if (match(TokenType::LEFT_BRACE)) {
        // map
        loadInstance = parseMap();
    } else if (match(TokenType::LEFT_BRACKET)) {
        // list
        loadInstance = parseList();
    } else if (match(TokenType::STRING)) {
        // string
        loadInstance = parseString();
    } else {
        // check '[' and '{' and literal string before calling this function ,so the following code will not be run
        reportParseError(tokens[current], "error in parsing visitable");
        return nullptr;
    }

    if (match(TokenType::DOT)) {
        consume(TokenType::IDENTIFIER, "Expect a property name after '.'.");
        String property = tokens[current - 1].getText();
        UniquePtr<ASTNode> loadProperty = nullptr;
        if (match(TokenType::LEFT_PAREN)) {
            loadProperty = parseInvokeMethod(std::move(loadInstance), property);
        } else {
            loadProperty = std::make_unique<LoadPropertyNode>(
                std::move(loadInstance), property, tokens[current - 2].getLine());
        }
        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)) {
            return parseRProperty(std::move(loadProperty));
        }
        return loadProperty;
    }

    if (match(TokenType::LEFT_BRACKET)) {
        int line = tokens[current - 1].getLine();
        UniquePtr<ASTNode> index = parseExpression();
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
        UniquePtr<ASTNode> indexAccess
            = std::make_unique<LoadSubscrNode>(std::move(loadInstance), std::move(index), line);
        if (check(TokenType::DOT) || check(TokenType::LEFT_BRACKET)) {
            return parseRProperty(std::move(indexAccess));
        }
        return indexAccess;
    }
    return loadInstance;
}
} // namespace aria