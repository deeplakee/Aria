#include "callFrame.h"

namespace aria {
CallFrame::CallFrame()
    : function{nullptr}
    , ip{nullptr}
    , slots{nullptr}
{}

CallFrame::CallFrame(ObjFunction *_function, uint8_t *_ip,Value* _slots)
    : function{_function}
    , ip{_ip}
    , slots{_slots}
{}

CallFrame::~CallFrame() = default;


} // namespace aria
