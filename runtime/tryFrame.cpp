#include "tryFrame.h"

namespace aria {

TryFrame::TryFrame()
    : ip{nullptr}
    , stackSize{-1}
    , callFrame{nullptr}
    , callFrameCount{-1}
{}
TryFrame::TryFrame(uint8_t *_ip, int _stackSize, CallFrame *_callFrame, int _callFrameCount)
    : ip{_ip}
    , stackSize{_stackSize}
    , callFrame{_callFrame}
    , callFrameCount{_callFrameCount}
{}

TryFrame::~TryFrame() = default;

void TryFrame::init(uint8_t *_ip, int _stackSize, CallFrame *_callFrame, int _callFrameCount)
{
    ip = _ip;
    stackSize = _stackSize;
    callFrame = _callFrame;
    callFrameCount = _callFrameCount;
}

} // namespace aria
