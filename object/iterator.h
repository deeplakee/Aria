#ifndef ITERATOR_H
#define ITERATOR_H

#include "common.h"
#include "value/value.h"

namespace aria {
class ObjList;
class ObjMap;
class ObjString;
class Iterator
{
public:
    Iterator() = default;
    virtual ~Iterator() = default;

    virtual void blacken(GC *gc) = 0;
    virtual String typeString() = 0;
    virtual size_t getSize() { return sizeof(Iterator); }
    virtual bool hasNext() { return false; }
    virtual Value next(GC *gc) = 0;
};

class ListIterator : public Iterator
{
public:
    ListIterator() = delete;
    explicit ListIterator(ObjList *list);
    ~ListIterator() override;

    void blacken(GC *gc) override;
    String typeString() override;
    size_t getSize() override;
    bool hasNext() override;
    Value next(GC *gc) override;

    ObjList *obj;
    int nextIndex;
};

class MapIterator : public Iterator
{
public:
    MapIterator() = delete;
    explicit MapIterator(ObjMap *map);
    ~MapIterator() override;

    void blacken(GC *gc) override;
    String typeString() override;
    size_t getSize() override;
    bool hasNext() override;
    Value next(GC *gc) override;

    ObjMap *obj;
    // -1: begin
    // -2: reach the end
    // >0: index of next value
    int nextIndex;
};

class StringIterator : public Iterator
{
public:
    StringIterator() = delete;
    explicit StringIterator(ObjString *str);
    ~StringIterator() override;

    void blacken(GC *gc) override;
    String typeString() override;
    size_t getSize() override;
    bool hasNext() override;
    Value next(GC *gc) override;

    ObjString *obj;
    int nextIndex;
};

} // namespace aria

#endif //ITERATOR_H
