#ifndef TRYFRAME_H
#define TRYFRAME_H

#include "common.h"
#include "value/value.h"

namespace aria {
class CallFrame;

class TryFrame
{
public:
    TryFrame();
    TryFrame(uint8_t *_ip, int _stackSize,CallFrame *_callFrame, int _callFrameCount);
    ~TryFrame();

    void init(uint8_t *_ip,  int _stackSize,CallFrame *_callFrame, int _callFrameCount);

    uint8_t *ip;
    int stackSize;
    CallFrame *callFrame;
    int callFrameCount;
};

} // namespace aria

#endif //TRYFRAME_H
