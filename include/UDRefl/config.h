#pragma once

#include <cstddef>

namespace Ubpa::UDRefl {
    static constexpr std::size_t MaxArgNum = 64;
    static_assert(MaxArgNum <= 256);
} // namespace Ubpa::UDRefl
