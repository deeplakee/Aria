#include "iterator.h"

#include "objList.h"
#include "objMap.h"
#include "objString.h"

namespace aria {

ListIterator::ListIterator(ObjList *list)
    : obj{list}
    , nextIndex{0}
{}
ListIterator::~ListIterator() = default;

void ListIterator::blacken(GC *gc)
{
    obj->blacken(gc);
}

String ListIterator::typeString()
{
    return valueTypeString(obj_val(obj));
}
size_t ListIterator::getSize()
{
    return sizeof(ListIterator);
}

bool ListIterator::hasNext()
{
    return obj->list->size() > nextIndex;
}

Value ListIterator::next(GC *gc)
{
    if (!hasNext()) {
        return nil_val;
    }
    return (*obj->list)[nextIndex++];
}

MapIterator::MapIterator(ObjMap *map)
    : obj{map}
    , nextIndex{-1}
{
    nextIndex = obj->map->getNextIndex(nextIndex);
}

MapIterator::~MapIterator() = default;

void MapIterator::blacken(GC *gc)
{
    obj->blacken(gc);
}

String MapIterator::typeString()
{
    return valueTypeString(obj_val(obj));
}

size_t MapIterator::getSize()
{
    return sizeof(MapIterator);
}

bool MapIterator::hasNext()
{
    return nextIndex != -2;
}

Value MapIterator::next(GC *gc)
{
    Value value = obj->map->getByIndex(nextIndex, gc);
    nextIndex = obj->map->getNextIndex(nextIndex);
    return value;
}

StringIterator::StringIterator(ObjString *str)
    : obj{str}
    , nextIndex{0}
{}
StringIterator::~StringIterator() = default;

void StringIterator::blacken(GC *gc)
{
    obj->blacken(gc);
}

String StringIterator::typeString()
{
    return valueTypeString(obj_val(obj));
}

size_t StringIterator::getSize()
{
    return sizeof(StringIterator);
}

bool StringIterator::hasNext()
{
    return obj->length > nextIndex;
}

Value StringIterator::next(GC *gc)
{
    if (!hasNext()) {
        return nil_val;
    }
    return obj_val(newObjString(obj->C_str_ref()[nextIndex++], gc));
}

} // namespace aria
