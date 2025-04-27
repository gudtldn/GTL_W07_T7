#pragma once
#include <type_traits>
#include <xutility>

#include "HAL/PlatformType.h"


template <typename T>
concept TIsCharType =
    std::_Is_character<T>::value
    || std::is_same_v<T, ANSICHAR>
    || std::is_same_v<T, WIDECHAR>;
