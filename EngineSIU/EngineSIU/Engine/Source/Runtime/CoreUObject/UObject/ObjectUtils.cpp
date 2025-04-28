#include "ObjectUtils.h"
#include "UObjectArray.h"


bool IsValid(const UObject* Test)
{
    return Test && GUObjectArray.GetObjectItemArrayUnsafe().Contains(const_cast<UObject*>(Test));
}
