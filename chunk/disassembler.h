#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "chunk.h"

namespace aria {
class Disassembler
{
public:
    static void disassembleChunk(const Chunk &chunk, const char *name);

    static int disassembleInstruction(const Chunk &chunk, int offset);

    static int simpleInstruction(const char *name, int offset);

    static int constantInstruction(const Chunk &chunk, const char *name, int offset);

    static int jumpInstruction(const Chunk &chunk, const char *name, int offset, int sign);

    static int tryInstruction(const Chunk &chunk, const char *name, int offset);

    static int twoBytesInstruction(const Chunk &chunk, const char *name, int offset);

    static int threeBytesInstruction(const Chunk &chunk, const char *name, int offset);

    static int fourBytesInstruction(const Chunk &chunk, const char *name, int offset);

    static int fiveBytesInstruction(const Chunk &chunk, const char *name, int offset);

    static int closureInstruction(const Chunk &chunk, int offset);
};
}

#endif //DISASSEMBLER_H