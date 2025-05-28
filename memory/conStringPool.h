#ifndef CONSTRINGPOOL_H
#define CONSTRINGPOOL_H

#include "common.h"

namespace aria {
class GC;
class ObjString;

using ObjStringPtr = ObjString*;

#define TABLE_MAX_LOAD 0.75

class ConStringPool
{
public:
    explicit ConStringPool(GC *_gc);

    ~ConStringPool();

    bool insert(ObjString *obj);

    ObjStringPtr get(const char *chars, size_t length, uint32_t hash);

    void removeWhite();

private:
    int count;
    int capacity;
    ObjStringPtr *table;
    GC *gc;

    static ObjStringPtr *findDest(ObjStringPtr  *f_table, ObjStringPtr key, int f_capacity);
    void adjustCapacity(int newCapacity);
};

} // namespace aria

#endif //CONSTRINGPOOL_H
