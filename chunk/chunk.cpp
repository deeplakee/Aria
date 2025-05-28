#include "chunk.h"
#include "compile/compilingException.h"
#include "disassembler.h"
#include "memory/gc.h"
#include "value/valueHashTable.h"

namespace aria {
Chunk::Chunk(GC *_gc)
    : gc{_gc}
    , count{0}
    , capacity{0}
    , code{nullptr}
    , constantPool{_gc}
    , sharedGlobalVarTable{false}
    , asModule{false}
    , globalVarTable{new ValueHashTable{_gc}}
{}

Chunk::Chunk(GC *_gc, ValueHashTable *_globalVarTable)
    : gc{_gc}
    , count{0}
    , capacity{0}
    , code{nullptr}
    , constantPool{_gc}
    , sharedGlobalVarTable{true}
    , asModule{false}
    , globalVarTable{_globalVarTable}
{}

Chunk::~Chunk()
{
    gc->free_array<uint8_t>(code, capacity);
    if (!sharedGlobalVarTable && !asModule) {
        delete globalVarTable;
    }
}

void Chunk::writeCode(uint8_t code_to_write, int line)
{
    if (capacity < count + 1) {
        int oldCapacity = capacity;
        capacity = GC::grow_capacity(oldCapacity);
        code = gc->grow_array<uint8_t>(code, oldCapacity, capacity);
    }

    code[count] = code_to_write;
    lines.insert(line);
    count++;
}

void Chunk::writeCode_L(uint16_t code_to_write, int line)
{
    writeCode(static_cast<uint8_t>(code_to_write & 0xFF), line);
    writeCode(static_cast<uint8_t>(code_to_write >> 8), line);
}

void Chunk::writeTwoCode(uint8_t code1, uint8_t code2, int line)
{
    writeCode(code1, line);
    writeCode(code2, line);
}

int Chunk::writeCode(opCode op, int line)
{
    writeCode(static_cast<uint8_t>(op), line);
    return count - 1;
}

void Chunk::loadConstant(Value value, int line)
{
    gc->cache(value);
    constantPool.write(value);
    gc->releaseCache(1);
    int constantIndex = constantPool.size() - 1;
    if (constantIndex < UINT16_MAX) {
        writeCode(opCode::LOAD_CONST, line);
        writeCode(static_cast<uint8_t>(constantIndex & 0xFF), line);
        writeCode(static_cast<uint8_t>(constantIndex >> 8), line);
        return;
    }
    throw CompilingException{"Too many constants in one chunk.", line};
}

uint16_t Chunk::writeConstant(Value value, int line)
{
    gc->cache(value);
    constantPool.write(value);
    gc->releaseCache(1);
    int constantIndex = constantPool.size() - 1;
    if (constantIndex < UINT16_MAX) {
        writeCode(static_cast<uint8_t>(constantIndex & 0xFF), line);
        writeCode(static_cast<uint8_t>(constantIndex >> 8), line);
        return static_cast<uint16_t>(constantIndex);
    }
    throw CompilingException{"Too many constants in one chunk.", line};
}

int Chunk::writeJumpBwd(opCode jumpOp, int line)
{
    writeCode(jumpOp, line);
    writeCode(0xFF, line);
    writeCode(0xFF, line);
    return count - 2;
}

void Chunk::writeJumpFwd(int start, int line)
{
    const unsigned int offset = count + 3 - start;
    if (offset > UINT16_MAX) {
        throw CompilingException{"Too much code to jump over.", line};
    }

    writeCode(opCode::JUMP_FWD, line);
    writeCode(static_cast<uint8_t>(offset & 0xFF), line);
    writeCode(static_cast<uint8_t>(offset >> 8), line);
}

void Chunk::backPatch(int src)
{
    const unsigned int offset = count - (src + 2);
    if (offset > UINT16_MAX) {
        throw CompilingException{"Too much code to jump over.", lines[src - 1]};
    }
    code[src] = static_cast<uint8_t>(offset & 0xFF);
    code[src + 1] = static_cast<uint8_t>(offset >> 8);
}

void Chunk::backPatch(int des, int src)
{
    unsigned int offset = 0;
    if (des < src) {
        code[src - 1] = static_cast<uint8_t>(opCode::JUMP_FWD);
        offset = src + 2 - des;
    } else {
        code[src - 1] = static_cast<uint8_t>(opCode::JUMP_BWD);
        offset = des - (src + 2);
    }
    if (offset > UINT16_MAX) {
        throw CompilingException{"Too much code to jump over.", lines[des - 1]};
    }
    code[src] = static_cast<uint8_t>(offset & 0xFF);
    code[src + 1] = static_cast<uint8_t>(offset >> 8);
}

void Chunk::rewriteCode(opCode op, int index)
{
    code[index] = static_cast<uint8_t>(op);
}

void Chunk::rewriteCode(uint8_t code_to_write, int index)
{
    code[index] = code_to_write;
}

void Chunk::genPopInstructions(int popCount, int line)
{
    while (popCount > 0) {
        if (popCount == 1) {
            writeCode(opCode::POP, line);
            break;
        }
        if (popCount < UINT8_MAX) {
            writeTwoCode(static_cast<uint8_t>(opCode::POP_N), static_cast<uint8_t>(popCount), line);
            break;
        }
        writeTwoCode(static_cast<uint8_t>(opCode::POP_N), 255, line);
        popCount -= UINT8_MAX;
    }
}

void Chunk::disassemble(StringView name)
{
    Disassembler::disassembleChunk(*this, name.data());
}
} // namespace aria
