#include "astNode.h"

#include <utility>

#include "chunk/chunk.h"

namespace aria {
Map<TokenType, opCode> ASTNode::tokenToOpCodeInBinary = {
    {TokenType::PLUS, opCode::ADD},
    {TokenType::MINUS, opCode::SUBTRACT},
    {TokenType::STAR, opCode::MULTIPLY},
    {TokenType::SLASH, opCode::DIVIDE},
    {TokenType::PERCENT, opCode::MOD},
    {TokenType::PLUS_EQUAL, opCode::ADD},
    {TokenType::MINUS_EQUAL, opCode::SUBTRACT},
    {TokenType::STAR_EQUAL, opCode::MULTIPLY},
    {TokenType::SLASH_EQUAL, opCode::DIVIDE},
    {TokenType::PERCENT_EQUAL, opCode::MOD},
    {TokenType::EQUAL_EQUAL, opCode::EQUAL},
    {TokenType::NOT_EQUAL, opCode::NOT_EQUAL},
    {TokenType::GREATER, opCode::GREATER},
    {TokenType::GREATER_EQUAL, opCode::GREATER_EQUAL},
    {TokenType::LESS, opCode::LESS},
    {TokenType::LESS_EQUAL, opCode::LESS_EQUAL},
    {TokenType::PLUS_PLUS, opCode::ADD},
    {TokenType::MINUS_MINUS, opCode::SUBTRACT},
};

ProgramNode::ProgramNode(List<UniquePtr<ASTNode>> declarations, int nodeLine)
    : declarations{std::move(declarations)}
{
    line = nodeLine;
}

FunDeclNode::FunDeclNode(
    String name,
    List<String> params,
    UniquePtr<ASTNode> body,
    bool acceptsVarargs,
    int endLine,
    int nodeLine)
    : name{std::move(name)}
    , params{std::move(params)}
    , body{std::move(body)}
    , acceptsVarargs{acceptsVarargs}
    , endLine{endLine}
{
    line = nodeLine;
}

ClassDeclNode::ClassDeclNode(
    String name, String superName, List<UniquePtr<ASTNode>> methods, int endLine, int nodeLine)
    : name{std::move(name)}
    , superName{std::move(superName)}
    , methods{std::move(methods)}
    , endLine{endLine}
{
    line = nodeLine;
}

VarDeclNode::VarDeclNode(List<String> name, List<UniquePtr<ASTNode>> expr, int nodeLine)
    : nameList{std::move(name)}
    , exprList{std::move(expr)}
{
    line = nodeLine;
}

BlockNode::BlockNode(List<UniquePtr<ASTNode>> declarations, int endLine, int nodeLine)
    : declarations{std::move(declarations)}
    , endLine{endLine}
{
    line = nodeLine;
}

PrintStmtNode::PrintStmtNode(UniquePtr<ASTNode> e, int nodeLine)
    : expr{std::move(e)}
{
    line = nodeLine;
}

ImportStmtNode::ImportStmtNode(String _importedName, String _moduleName, int nodeLine)
    : inputStr{std::move(_importedName)}
    , moduleName{std::move(_moduleName)}
{
    line = nodeLine;
}

IfStmtNode::IfStmtNode(
    UniquePtr<ASTNode> condition, UniquePtr<ASTNode> body, UniquePtr<ASTNode> elseBody, int nodeLine)
    : condition{std::move(condition)}
    , body{std::move(body)}
    , elseBody{std::move(elseBody)}
{
    line = nodeLine;
}

WhileStmtNode::WhileStmtNode(
    UniquePtr<ASTNode> condition, UniquePtr<ASTNode> body, int endLine, int nodeLine)
    : condition{std::move(condition)}
    , body{std::move(body)}
    , endLine{endLine}
{
    line = nodeLine;
}

ForStmtNode::ForStmtNode(
    UniquePtr<ASTNode> varInit,
    UniquePtr<ASTNode> condition,
    UniquePtr<ASTNode> increment,
    UniquePtr<ASTNode> body,
    int endLine,
    int nodeLine)
    : varInit{std::move(varInit)}
    , condition{std::move(condition)}
    , increment{std::move(increment)}
    , body{std::move(body)}
    , endLine{endLine}
{
    line = nodeLine;
}

ForInStmtNode::ForInStmtNode(
    String iterName, UniquePtr<ASTNode> iterExpr, UniquePtr<ASTNode> body, int endLine, int nodeLine)
    : loopVarName{std::move(iterName)}
    , iterExpr{std::move(iterExpr)}
    , body{std::move(body)}
    , endLine{endLine}
{
    line = nodeLine;
}

TryCatchStmtNode::TryCatchStmtNode(
    UniquePtr<ASTNode> tryBody,
    UniquePtr<ASTNode> catchBody,
    String exceptionName,
    int catchLine,
    int endLine,
    int nodeLine)
    : tryBody{std::move(tryBody)}
    , catchBody{std::move(catchBody)}
    , exceptionName{std::move(exceptionName)}
    , catchLine{catchLine}
    , endLine{endLine}
{
    line = nodeLine;
}

ThrowStmtNode::ThrowStmtNode(UniquePtr<ASTNode> e, int nodeLine)
    : e{std::move(e)}
{
    line = nodeLine;
}

BreakStmtNode::BreakStmtNode(int nodeLine)
{
    line = nodeLine;
}

ContinueStmtNode::ContinueStmtNode(int nodeLine)
{
    line = nodeLine;
}

ReturnStmtNode::ReturnStmtNode(UniquePtr<ASTNode> expr, int nodeLine)
    : expr{std::move(expr)}
{
    line = nodeLine;
}

ExprStmtNode::ExprStmtNode(UniquePtr<ASTNode> e, int nodeLine)
    : expr{std::move(e)}
{
    line = nodeLine;
}

AssignNode::AssignNode(UniquePtr<ASTNode> l, UniquePtr<ASTNode> r, int nodeLine)
    : left{std::move(l)}
    , expr{std::move(r)}
{
    line = nodeLine;
}

BinaryOpNode::BinaryOpNode(
    std::unique_ptr<ASTNode> l, TokenType o, std::unique_ptr<ASTNode> r, int nodeLine)
    : left{std::move(l)}
    , right{std::move(r)}
    , op{o}
{
    line = nodeLine;
}

UnaryOpNode::UnaryOpNode(TokenType o, std::unique_ptr<ASTNode> e, int nodeLine)
    : operand{std::move(e)}
    , op{o}
{
    line = nodeLine;
}

CallNode::CallNode(UniquePtr<ASTNode> callee, List<UniquePtr<ASTNode>> args, int nodeLine)
    : callee{std::move(callee)}
    , args{std::move(args)}
{
    line = nodeLine;
}

InvokeMethodNode::InvokeMethodNode(
    UniquePtr<ASTNode> instance, String methodName, List<UniquePtr<ASTNode>> args, int nodeLine)
    : instance{std::move(instance)}
    , methodName{std::move(methodName)}
    , args{std::move(args)}
{
    line = nodeLine;
}

LoadSuperMethodNode::LoadSuperMethodNode(String methodName, int nodeLine)
    : methodName{std::move(methodName)}
{
    line = nodeLine;
}

LoadPropertyNode::LoadPropertyNode(UniquePtr<ASTNode> instance, String propertyName, int nodeLine)
    : instance{std::move(instance)}
    , propertyName{std::move(propertyName)}
{
    line = nodeLine;
}

StorePropertyNode::StorePropertyNode(UniquePtr<ASTNode> instance, String propertyName, int nodeLine)
    : instance{std::move(instance)}
    , propertyName{std::move(propertyName)}
{
    line = nodeLine;
}

LoadSubscrNode::LoadSubscrNode(UniquePtr<ASTNode> instance, UniquePtr<ASTNode> index, int nodeLine)
    : instance{std::move(instance)}
    , index{std::move(index)}
{
    line = nodeLine;
}

StoreSubscrNode::StoreSubscrNode(UniquePtr<ASTNode> instance, UniquePtr<ASTNode> index, int nodeLine)
    : instance{std::move(instance)}
    , index{std::move(index)}
{
    line = nodeLine;
}

LoadVarNode::LoadVarNode(String varName, int nodeLine)
    : varName{std::move(varName)}
{
    line = nodeLine;
}

StoreVarNode::StoreVarNode(String varName, int nodeLine)
    : varName{std::move(varName)}
{
    line = nodeLine;
}

NumberNode::NumberNode(double v, int nodeLine)
    : value{v}
{
    line = nodeLine;
}

TrueNode::TrueNode(int nodeLine)
{
    line = nodeLine;
}

FalseNode::FalseNode(int nodeLine)
{
    line = nodeLine;
}

NilNode::NilNode(int nodeLine)
{
    line = nodeLine;
}

StringNode::StringNode(String str, int nodeLine)
    : text{std::move(str)}
{
    line = nodeLine;
}

ListNode::ListNode(List<UniquePtr<ASTNode>> exprs, int nodeLine)
    : exprs{std::move(exprs)}
{
    line = nodeLine;
}

MapNode::MapNode(List<UniquePtr<ASTNode>> kvPairs, int nodeLine)
    : kvPairs{std::move(kvPairs)}
{
    line = nodeLine;
}

ErrorNode::ErrorNode(int nodeLine)
{
    line = nodeLine;
}

} // namespace aria