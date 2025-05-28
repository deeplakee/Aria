#ifndef ASTNODE_H
#define ASTNODE_H

#include "chunk/code.h"
#include "common.h"
#include "token.h"

namespace aria {
class Chunk;
class FunctionContext;

struct ASTNode
{
    virtual ~ASTNode() = default;

    virtual void generateByteCode(FunctionContext *currentFunction) = 0;

    int line = -1;
    static Map<TokenType, opCode> tokenToOpCodeInBinary;
};

struct ProgramNode : public ASTNode
{
    List<UniquePtr<ASTNode>> declarations;

    ProgramNode(List<UniquePtr<ASTNode>> declarations, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct FunDeclNode : public ASTNode
{
    String name;
    List<String> params;
    UniquePtr<ASTNode> body;
    bool acceptsVarargs;
    int endLine;

    FunDeclNode(
        String name,
        List<String> params,
        UniquePtr<ASTNode> body,
        bool acceptsVarargs,
        int endLine,
        int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ClassDeclNode : public ASTNode
{
    String name;
    String superName;
    List<UniquePtr<ASTNode>> methods;
    int endLine;

    ClassDeclNode(
        String name, String superName, List<UniquePtr<ASTNode>> methods, int endLine, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct VarDeclNode : public ASTNode
{
    List<String> nameList;
    List<UniquePtr<ASTNode>> exprList;

    VarDeclNode(List<String> name, List<UniquePtr<ASTNode>> expr, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct BlockNode : public ASTNode
{
    List<UniquePtr<ASTNode>> declarations;
    int endLine;

    BlockNode(List<UniquePtr<ASTNode>> declarations, int endLine, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct PrintStmtNode : public ASTNode
{
    UniquePtr<ASTNode> expr;

    PrintStmtNode(UniquePtr<ASTNode> e, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ImportStmtNode : public ASTNode
{
    String inputStr;
    String moduleName;

    ImportStmtNode(String _importedName, String _moduleName, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct IfStmtNode : public ASTNode
{
    UniquePtr<ASTNode> condition;
    UniquePtr<ASTNode> body;
    UniquePtr<ASTNode> elseBody;

    IfStmtNode(
        UniquePtr<ASTNode> condition,
        UniquePtr<ASTNode> body,
        UniquePtr<ASTNode> elseBody,
        int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct WhileStmtNode : public ASTNode
{
    UniquePtr<ASTNode> condition;
    UniquePtr<ASTNode> body;
    int endLine;

    WhileStmtNode(UniquePtr<ASTNode> condition, UniquePtr<ASTNode> body, int endLine, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ForStmtNode : public ASTNode
{
    UniquePtr<ASTNode> varInit;
    UniquePtr<ASTNode> condition;
    UniquePtr<ASTNode> increment;
    UniquePtr<ASTNode> body;
    int endLine;

    ForStmtNode(
        UniquePtr<ASTNode> varInit,
        UniquePtr<ASTNode> condition,
        UniquePtr<ASTNode> increment,
        UniquePtr<ASTNode> body,
        int endLine,
        int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ForInStmtNode : public ASTNode
{
    String loopVarName;
    UniquePtr<ASTNode> iterExpr;
    UniquePtr<ASTNode> body;
    int endLine;

    ForInStmtNode(
        String iterName,
        UniquePtr<ASTNode> iterExpr,
        UniquePtr<ASTNode> body,
        int endLine,
        int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct TryCatchStmtNode : public ASTNode
{
    UniquePtr<ASTNode> tryBody;
    UniquePtr<ASTNode> catchBody;
    String exceptionName;
    int catchLine;
    int endLine;

    TryCatchStmtNode(
        UniquePtr<ASTNode> tryBody,
        UniquePtr<ASTNode> catchBody,
        String exceptionName,
        int catchLine,
        int endLine,
        int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ThrowStmtNode : public ASTNode
{
    UniquePtr<ASTNode> e;

    ThrowStmtNode(UniquePtr<ASTNode> e, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct BreakStmtNode : public ASTNode
{
    explicit BreakStmtNode(int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ContinueStmtNode : public ASTNode
{
    explicit ContinueStmtNode(int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ReturnStmtNode : public ASTNode
{
    UniquePtr<ASTNode> expr;

    ReturnStmtNode(UniquePtr<ASTNode> expr, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ExprStmtNode : public ASTNode
{
    UniquePtr<ASTNode> expr;

    ExprStmtNode(UniquePtr<ASTNode> e, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct AssignNode : public ASTNode
{
    UniquePtr<ASTNode> left;
    UniquePtr<ASTNode> expr;

    AssignNode(UniquePtr<ASTNode> l, UniquePtr<ASTNode> r, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct BinaryOpNode : public ASTNode
{
    UniquePtr<ASTNode> left;
    UniquePtr<ASTNode> right;
    TokenType op;

    BinaryOpNode(std::unique_ptr<ASTNode> l, TokenType o, std::unique_ptr<ASTNode> r, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct UnaryOpNode : public ASTNode
{
    std::unique_ptr<ASTNode> operand;
    TokenType op;

    UnaryOpNode(TokenType o, std::unique_ptr<ASTNode> e, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct CallNode : public ASTNode
{
    UniquePtr<ASTNode> callee;
    List<UniquePtr<ASTNode>> args;

    CallNode(UniquePtr<ASTNode> callee, List<UniquePtr<ASTNode>> args, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct InvokeMethodNode : public ASTNode
{
    UniquePtr<ASTNode> instance;
    String methodName;
    List<UniquePtr<ASTNode>> args;

    InvokeMethodNode(
        UniquePtr<ASTNode> instance, String methodName, List<UniquePtr<ASTNode>> args, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct LoadSuperMethodNode : public ASTNode
{
    String methodName;

    LoadSuperMethodNode(String methodName, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct LoadPropertyNode : public ASTNode
{
    UniquePtr<ASTNode> instance;
    String propertyName;

    LoadPropertyNode(UniquePtr<ASTNode> instance, String propertyName, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct StorePropertyNode : public ASTNode
{
    UniquePtr<ASTNode> instance;
    String propertyName;

    StorePropertyNode(UniquePtr<ASTNode> instance, String propertyName, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct LoadSubscrNode : public ASTNode
{
    UniquePtr<ASTNode> instance;
    UniquePtr<ASTNode> index;

    LoadSubscrNode(UniquePtr<ASTNode> instance, UniquePtr<ASTNode> index, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct StoreSubscrNode : public ASTNode
{
    UniquePtr<ASTNode> instance;
    UniquePtr<ASTNode> index;

    StoreSubscrNode(UniquePtr<ASTNode> instance, UniquePtr<ASTNode> index, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct LoadVarNode : public ASTNode
{
    String varName;

    LoadVarNode(String varName, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct StoreVarNode : public ASTNode
{
    String varName;

    StoreVarNode(String varName, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct NumberNode : public ASTNode
{
    double value;

    explicit NumberNode(double v, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct TrueNode : public ASTNode
{
    explicit TrueNode(int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct FalseNode : public ASTNode
{
    explicit FalseNode(int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct NilNode : public ASTNode
{
    explicit NilNode(int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct StringNode : public ASTNode
{
    String text;

    StringNode(String str, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ListNode : public ASTNode
{
    List<UniquePtr<ASTNode>> exprs;

    ListNode(List<UniquePtr<ASTNode>> exprs, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct MapNode : public ASTNode
{
    List<UniquePtr<ASTNode>> kvPairs;

    MapNode(List<UniquePtr<ASTNode>> kvPairs, int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

struct ErrorNode : public ASTNode
{
    explicit ErrorNode(int nodeLine);

    void generateByteCode(FunctionContext *currentFunction) override;
};

} // namespace aria

#endif //ASTNODE_H