#ifndef CHUNK_H
#define CHUNK_H

#include "code.h"
#include "common.h"
#include "util/rleList.h"
#include "value/valueArray.h"

namespace aria {
class GC;
class ValueHashTable;

class Chunk
{
public:
    GC *gc;

    int count;
    int capacity;
    uint8_t *code;
    RLEList<int> lines;

    ValueArray constantPool;

    bool sharedGlobalVarTable;
    bool asModule;
    ValueHashTable *globalVarTable;

    // no sharedGlobalVarTable
    explicit Chunk(GC *_gc);

    // sharedGlobalVarTable
    Chunk(GC *_gc, ValueHashTable *_globalVarTable);

    ~Chunk();

    uint8_t operator[](int offset) const {
        return code[offset];
    }

    void writeCode(uint8_t code, int line);

    void writeCode_L(uint16_t code, int line);

    void writeTwoCode(uint8_t code1, uint8_t code2, int line);

    // write an operation
    int writeCode(opCode op, int line);

    // write loading value
    void loadConstant(Value value, int line);

    // write 16bits index of constant and return the 16bits index
    uint16_t writeConstant(Value value, int line);

    /**
     * write a jump backward instruction, return the index of offset which is
     * needed to be backpatched
     * @param jumpOp Jump op to be used
     * @param line the line of source code of the op
     * @return the index after jump op,the first byte of offset
     */
    int writeJumpBwd(opCode jumpOp, int line);

    /**
     * write a jump forward instruction
     * @param start destination of jump
     * @param line the line of source code of the op
     */
    void writeJumpFwd(int start, int line);

    /**
     * jump from src to current
     * @note jump backward
     * @param src source,the index after jump op,the first byte of offset
     */
    void backPatch(int src);

    /**
     * jump from src to des
     * @param des destination,the index of the op to be executed after the jump
     * @param src source,the index after jump op,the first byte of offset
     */
    void backPatch(int des, int src);

    void rewriteCode(opCode op, int index);

    void rewriteCode(uint8_t code_to_write, int index);

    void genPopInstructions(int popCount, int line);

    void disassemble(StringView name);
};
} // namespace aria

#endif // CHUNK_H
