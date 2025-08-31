#ifndef OBJMODULE_H
#define OBJMODULE_H

#include "object.h"

namespace aria {
class ObjFunction;
class ValueHashTable;
class ObjModule : public Obj
{
public:
    ObjModule() = delete;
    ObjModule(ObjFunction *_module, GC *_gc);
    ~ObjModule() override;

    bool getAttribute(ObjString *name, Value &value) override;
    String toString() override;
    String toRawString() override;
    void blacken(GC *gc) override;

    ObjString *name;
    ValueHashTable *module;
};

inline bool is_objModule(Value value)
{
    return isObjType(value, objType::MODULE);
}

inline ObjModule *as_objModule(Value value)
{
    return dynamic_cast<ObjModule *>(as_obj(value));
}

ObjModule *newObjModule(ObjFunction *module, GC *gc);

} // namespace aria

#endif //OBJMODULE_H
