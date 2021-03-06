#pragma once

#include <cstddef>

// use it in "Basic.h"
#ifndef UBPA_UDREFL_INCLUDE_ALL_STD_NAME
// #define UBPA_UDREFL_INCLUDE_ALL_STD_NAME
#endif // UBPA_UDREFL_INCLUDE_ALL_STD_NAME

namespace Ubpa::UDRefl {
    static constexpr std::size_t MaxArgNum = 64;
    static_assert(MaxArgNum <= 256 - 2);
} // namespace Ubpa::UDRefl
