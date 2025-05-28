#include "disassembler.h"

#include "object/objFunction.h"

namespace aria {
void Disassembler::disassembleChunk(const Chunk &chunk, const char *name)
{
    print("  ====== {:^10} ======\n", name);

    for (int offset = 0; offset < chunk.count;) {
        offset = disassembleInstruction(chunk, offset);
    }
    print("  ====== {:^10} ======\n", "chunk end");
}

int Disassembler::disassembleInstruction(const Chunk &chunk, int offset)
{
    print("{:06} ", offset);

    if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1]) {
        cout << "     | ";
    } else {
        print("{:6} ", chunk.lines[offset]);
    }

    switch (auto instruction = static_cast<opCode>(chunk[offset])) {
    case opCode::LOAD_CONST:
        return constantInstruction(chunk, "LOAD_CONST", offset);
    case opCode::LOAD_NIL:
        return simpleInstruction("LOAD_NIL", offset);
    case opCode::LOAD_TRUE:
        return simpleInstruction("LOAD_TRUE", offset);
    case opCode::LOAD_FALSE:
        return simpleInstruction("LOAD_FALSE", offset);
    case opCode::POP:
        return simpleInstruction("POP", offset);
    case opCode::POP_N:
        return twoBytesInstruction(chunk, "POP_N", offset);
    case opCode::LOAD_LOCAL:
        return threeBytesInstruction(chunk, "LOAD_LOCAL", offset);
    case opCode::STORE_LOCAL:
        return threeBytesInstruction(chunk, "STORE_LOCAL", offset);
    case opCode::LOAD_UPVALUE:
        return threeBytesInstruction(chunk, "LOAD_UPVALUE", offset);
    case opCode::STORE_UPVALUE:
        return threeBytesInstruction(chunk, "STORE_UPVALUE", offset);
    case opCode::CLOSE_UPVALUE:
        return simpleInstruction("CLOSE_UPVALUE", offset);
    case opCode::DEF_GLOBAL:
        return constantInstruction(chunk, "DEF_GLOBAL", offset);
    case opCode::LOAD_GLOBAL:
        return constantInstruction(chunk, "LOAD_GLOBAL", offset);
    case opCode::STORE_GLOBAL:
        return constantInstruction(chunk, "STORE_GLOBAL", offset);
    case opCode::LOAD_PROPERTY:
        return constantInstruction(chunk, "LOAD_PROPERTY", offset);
    case opCode::STORE_PROPERTY:
        return constantInstruction(chunk, "STORE_PROPERTY", offset);
    case opCode::LOAD_SUBSCR:
        return simpleInstruction("LOAD_SUBSCR", offset);
    case opCode::STORE_SUBSCR:
        return simpleInstruction("STORE_SUBSCR", offset);
    case opCode::EQUAL:
        return simpleInstruction("EQUAL", offset);
    case opCode::NOT_EQUAL:
        return simpleInstruction("NOT_EQUAL", offset);
    case opCode::GREATER:
        return simpleInstruction("GREATER", offset);
    case opCode::GREATER_EQUAL:
        return simpleInstruction("GREATER_EQUAL", offset);
    case opCode::LESS:
        return simpleInstruction("LESS", offset);
    case opCode::LESS_EQUAL:
        return simpleInstruction("LESS_EQUAL", offset);
    case opCode::ADD:
        return simpleInstruction("ADD", offset);
    case opCode::SUBTRACT:
        return simpleInstruction("SUBTRACT", offset);
    case opCode::MULTIPLY:
        return simpleInstruction("MULTIPLY", offset);
    case opCode::DIVIDE:
        return simpleInstruction("DIVIDE", offset);
    case opCode::MOD:
        return simpleInstruction("MOD", offset);
    case opCode::NOT:
        return simpleInstruction("NOT", offset);
    case opCode::NEGATE:
        return simpleInstruction("NEGATE", offset);
    case opCode::INC:
        return simpleInstruction("INC", offset);
    case opCode::DEC:
        return simpleInstruction("DEC", offset);
    case opCode::PRINT:
        return simpleInstruction("PRINT", offset);
    case opCode::NOP:
        return simpleInstruction("NOP", offset);
    case opCode::JUMP_FWD:
        return jumpInstruction(chunk, "JUMP_FWD", offset, -1);
    case opCode::JUMP_BWD:
        return jumpInstruction(chunk, "JUMP_BWD", offset, 1);
    case opCode::JUMP_TRUE:
        return jumpInstruction(chunk, "JUMP_TRUE", offset, 1);
    case opCode::JUMP_TRUE_NOPOP:
        return jumpInstruction(chunk, "JUMP_TRUE_NOPOP", offset, 1);
    case opCode::JUMP_FALSE:
        return jumpInstruction(chunk, "JUMP_FALSE", offset, 1);
    case opCode::JUMP_FALSE_NOPOP:
        return jumpInstruction(chunk, "JUMP_FALSE_NOPOP", offset, 1);
    case opCode::CALL:
        return twoBytesInstruction(chunk, "CALL", offset);
    case opCode::CLOSURE:
        return closureInstruction(chunk, offset);
    case opCode::MAKE_CLASS:
        return constantInstruction(chunk, "MAKE_CLASS", offset);
    case opCode::INHERIT:
        return simpleInstruction("INHERIT", offset);
    case opCode::MAKE_METHOD:
        return constantInstruction(chunk, "MAKE_METHOD", offset);
    case opCode::MAKE_INIT_METHOD:
        return simpleInstruction("MAKE_INIT_METHOD", offset);
    case opCode::INVOKE_METHOD:
        return fourBytesInstruction(chunk, "INVOKE_METHOD", offset);
    case opCode::LOAD_SUPER_METHOD:
        return constantInstruction(chunk, "LOAD_SUPER_METHOD", offset);
    case opCode::MAKE_LIST:
        return threeBytesInstruction(chunk, "MAKE_LIST", offset);
    case opCode::MAKE_MAP:
        return threeBytesInstruction(chunk, "MAKE_MAP", offset);
    case opCode::IMPORT:
        return fiveBytesInstruction(chunk, "IMPORT", offset);
    case opCode::GET_ITER:
        return simpleInstruction("GET_ITER", offset);
    case opCode::ITER_HAS_NEXT:
        return simpleInstruction("ITER_HAS_NEXT", offset);
    case opCode::ITER_GET_NEXT:
        return simpleInstruction("ITER_GET_NEXT", offset);
    case opCode::BEGIN_TRY:
        return jumpInstruction(chunk, "BEGIN_TRY", offset, 1);
    case opCode::END_TRY:
        return simpleInstruction("END_TRY", offset);
    case opCode::THROW:
        return simpleInstruction("THROW", offset);
    case opCode::RETURN:
        return simpleInstruction("RETURN", offset);
    default:
        print("Unknown opcode {:02x}\n", static_cast<uint8_t>(instruction));
        return offset + 1;
    }
}

static uint16_t getU16data(int offset, const uint8_t *code)
{
    uint8_t byte1 = code[offset];
    uint8_t byte2 = code[offset + 1];
    return static_cast<uint16_t>(byte1) | static_cast<uint16_t>(byte2 << 8);
}

int Disassembler::simpleInstruction(const char *name, const int offset)
{
    cout << name << endl;
    return offset + 1;
}

int Disassembler::constantInstruction(const Chunk &chunk, const char *name, int offset)
{
    int slot = getU16data(offset + 1, chunk.code);
    String constantName = valueString(chunk.constantPool[slot]);
    print("{:<18} {:6} {}\n", name, slot, constantName);
    return offset + 3;
}

int Disassembler::jumpInstruction(const Chunk &chunk, const char *name, int offset, const int sign)
{
    uint16_t jump = getU16data(offset + 1, chunk.code);
    print("{:<18} {:6} -> {}\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

int Disassembler::tryInstruction(const Chunk &chunk, const char *name, int offset)
{
    uint16_t jump = getU16data(offset + 1, chunk.code);
    print("{:<18} {:6} -> {}\n", name, offset, offset + 1 + jump);
    return offset + 3;
}

int Disassembler::twoBytesInstruction(const Chunk &chunk, const char *name, const int offset)
{
    const uint8_t n = chunk[offset + 1];
    print("{:<18} {:6}\n", name, n);
    return offset + 2;
}

int Disassembler::threeBytesInstruction(const Chunk &chunk, const char *name, const int offset)
{
    uint16_t slot = getU16data(offset + 1, chunk.code);
    print("{:<18} {:6}\n", name, slot);
    return offset + 3;
}

int Disassembler::fourBytesInstruction(const Chunk &chunk, const char *name, const int offset)
{
    uint16_t slot = getU16data(offset + 1, chunk.code);
    uint8_t argCount = chunk[offset + 3];
    String methodName = valueString(chunk.constantPool[slot]);
    print("{:<18} {:6} {} ( {} args)\n", name, slot, methodName, argCount);
    return offset + 4;
}

int Disassembler::fiveBytesInstruction(const Chunk &chunk, const char *name, const int offset)
{
    uint16_t slot1 = getU16data(offset + 1, chunk.code);
    uint16_t slot2 = getU16data(offset + 3, chunk.code);
    String importedName = valueString(chunk.constantPool[slot1]);
    String moduleName = valueString(chunk.constantPool[slot2]);
    print("{:<18} {:6} {} as {} {}\n", name, slot1, importedName, slot2, moduleName);
    return offset + 5;
}

int Disassembler::closureInstruction(const Chunk &chunk, int offset)
{
    int _offset = offset;
    uint16_t funIndex = getU16data(_offset + 1, chunk.code);
    _offset += 3;
    ObjFunction *function = as_objFunction(chunk.constantPool[funIndex]);
    print("{:<18} {:6} {}\n", "OP_CLOSURE", funIndex, function->toString());
    for (int j = 0; j < function->upvalueCount; j++) {
        int isLocal = chunk[_offset++];
        String varType = isLocal ? "local" : "upvalue";
        uint16_t index = getU16data(_offset, chunk.code);
        _offset += 2;
        print("{:06}      |{:>25}{:<7} {}\n", _offset - 3, "", varType, index);
    }
    return _offset;
}
} // namespace aria