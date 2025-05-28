#ifndef VALUE_H
#define VALUE_H

#include "util/nan.h"

namespace aria {
class GC;
class ValueStack;
using Value = nan_t;

String valueString(Value value);

String valueString(Value value, ValueStack *outer);

String rawValueString(Value value);

String rawValueString(Value value, ValueStack *outer);

String valueTypeString(Value value);

bool valuesEqual(Value a, Value b);

bool valuesSame(Value a, Value b);

uint32_t valueHash(Value value);

void markValue(Value value, GC *gc);

/**
 *
 * @return true->false
 * @return false->true
 * @return nil->true
 * @return others->false
 */
bool isFalsey(Value value);
} // namespace aria

#endif // VALUE_H
