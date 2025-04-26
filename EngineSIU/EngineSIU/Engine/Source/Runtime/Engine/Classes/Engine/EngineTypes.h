#pragma once
#include "Core/HAL/PlatformType.h"

namespace EEndPlayReason
{
enum Type : uint8
{
    /** 명시적인 삭제가 일어났을 때, Destroy()등 */
    Destroyed,
    /** World가 바뀌었을 때 */
    WorldTransition,
    /** 프로그램을 종료했을 때 */
    Quit,
};

inline const char* ToString(Type e)
{
    switch (e)
    {
    case Destroyed: return "Destroyed";
    case WorldTransition: return "WorldTransition";
    case Quit: return "Quit";
    default: return "unknown";
    }
}
}
