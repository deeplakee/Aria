#include <utility>

#include "chunk/chunk.h"
#include "compiler.h"
#include "compilingException.h"
#include "object/objString.h"
#include "parser.h"
#include "scanner.h"

namespace aria {
Compiler::Compiler(GC *gc)
    : mainCode{FunctionType::SCRIPT, gc}
{}

Compiler::Compiler(GC *gc, ValueHashTable *globalVarTable)
    : mainCode{FunctionType::SCRIPT, gc, globalVarTable}
{}

Compiler::~Compiler() = default;

ObjFunction *Compiler::compile(String source)
{
    auto scanner = std::make_unique<Scanner>(std::move(source));

    List<Token> tokens = scanner->scan();
    if (scanner->hadError()) {
        return nullptr;
    }

    auto parser = std::make_unique<Parser>(std::move(tokens));

    UniquePtr<ASTNode> ast = parser->parse();
    if (parser->hasError()) {
        return nullptr;
    }

    try {
        ast->generateByteCode(&mainCode);
    } catch (const CompilingException &e) {
        cerr << e.what() << endl;
        return nullptr;
    }

    ObjFunction *function = mainCode.function;

#ifdef DEBUG_PRINT_COMPILED_CODE
    mainCode.getChunk()->disassemble(function->toString());
#endif

    return function;
}
ObjFunction *Compiler::compile(String source, String moduleName)
{
    auto scanner = std::make_unique<Scanner>(std::move(source));

    List<Token> tokens = scanner->scan();
    if (scanner->hadError()) {
        return nullptr;
    }

    auto parser = std::make_unique<Parser>(std::move(tokens));

    UniquePtr<ASTNode> ast = parser->parse();
    if (parser->hasError()) {
        return nullptr;
    }

    try {
        ast->generateByteCode(&mainCode);
    } catch (const CompilingException &e) {
        cerr << e.what() << endl;
        return nullptr;
    }

    ObjFunction *function = mainCode.function;
    function->name = newObjString(moduleName, mainCode.gc);

#ifdef DEBUG_PRINT_COMPILED_CODE
    mainCode.getChunk()->disassemble(moduleName);
#endif

    return function;
}
} // namespace aria