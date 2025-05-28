#ifndef PARSER_H
#define PARSER_H

#include "astNode.h"
#include "token.h"

namespace aria {
class Parser
{
public:
    explicit Parser(List<Token> _tokens);

    ~Parser();

    UniquePtr<ASTNode> parse();

    bool hasError() const;

private:
    int current;
    List<Token> tokens;
    bool hadError;
    bool panicMode;

    void reportParseError(Token &errorToken, StringView msg);

    bool match(TokenType t);

    bool check(TokenType t) const;

    bool consume(TokenType t, StringView msg);

    void synchronize();

    UniquePtr<ASTNode> parseProgram();

    UniquePtr<ASTNode> parseDeclaration();

    UniquePtr<ASTNode> parseFunDecl();

    UniquePtr<ASTNode> parseClassDecl();

    UniquePtr<ASTNode> parseVarDecl();

    UniquePtr<ASTNode> parseStatement();

    UniquePtr<ASTNode> parsePrintStmt();

    UniquePtr<ASTNode> parseImportStmt();

    UniquePtr<ASTNode> parseIfStmt();

    UniquePtr<ASTNode> parseWhileStmt();

    UniquePtr<ASTNode> parseForStmt();

    UniquePtr<ASTNode> parseForInStmt();

    UniquePtr<ASTNode> parseTryCatchStmt();

    UniquePtr<ASTNode> parseThrowStmt();

    UniquePtr<ASTNode> parseBreakStmt();

    UniquePtr<ASTNode> parseContinueStmt();

    UniquePtr<ASTNode> parseReturnStmt();

    UniquePtr<ASTNode> parseExprStmt();

    UniquePtr<ASTNode> parseBlock();

    UniquePtr<ASTNode> parseExpression();

    UniquePtr<ASTNode> parseAssignment();

    UniquePtr<ASTNode> parseLogicOr();

    UniquePtr<ASTNode> parseLogicAnd();

    UniquePtr<ASTNode> parseLvalue();

    UniquePtr<ASTNode> parseLProperty(UniquePtr<ASTNode> instance);

    UniquePtr<ASTNode> parseLThis();

    UniquePtr<ASTNode> parseEquality();

    UniquePtr<ASTNode> parseComparison();

    UniquePtr<ASTNode> parseTerm();

    UniquePtr<ASTNode> parseFactor();

    UniquePtr<ASTNode> parseUnary();

    UniquePtr<ASTNode> parseValue();

    UniquePtr<ASTNode> parseRvalue();

    UniquePtr<ASTNode> parseCall(UniquePtr<ASTNode> callee);

    UniquePtr<ASTNode> parseRProperty(UniquePtr<ASTNode> loadInstance);

    UniquePtr<ASTNode> parseInvokeMethod(UniquePtr<ASTNode> instance, String methodName);

    UniquePtr<ASTNode> parseRThis();

    UniquePtr<ASTNode> parseSuper();

    UniquePtr<ASTNode> parsePrimary();

    UniquePtr<ASTNode> parseParenExpr();

    UniquePtr<ASTNode> parseList();

    UniquePtr<ASTNode> parseMap();

    UniquePtr<ASTNode> parseString();

    UniquePtr<ASTNode> parseVisitable();
};
} // namespace aria

#endif //PARSER_H
