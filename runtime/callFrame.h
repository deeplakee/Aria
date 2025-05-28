#ifndef CALLFRAME_H
#define CALLFRAME_H

#include "common.h"
#include "value/value.h"

namespace aria {
class ObjFunction;

class CallFrame {
public:
    CallFrame();

    CallFrame(ObjFunction* _function,uint8_t *_ip,Value* _slots);

    ~CallFrame();

    void init(ObjFunction *_function, uint8_t *_ip, Value *_slots)
    {
        this->function = _function;
        this->ip = _ip;
        this->slots = _slots;
    }
    void copy(CallFrame *other)
    {
        this->function = other->function;
        this->ip = other->ip;
        this->slots = other->slots;
    }

    ObjFunction *function;
    uint8_t *ip;
    Value* slots;

};
}

#endif //CALLFRAME_H
