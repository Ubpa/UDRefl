#pragma once

#if (defined(WIN32) || defined(_WIN32)) && defined(UBPA_UDREFL_SHARED)
#ifdef UCMAKE_EXPORT_UDRefl_core
#define UDRefl_core_API __declspec(dllexport)
#else
#define UDRefl_core_API __declspec(dllimport)
#endif
#else
#define UDRefl_core_API
#endif // (defined(WIN32) || defined(_WIN32)) && !defined(UBPA_UDREFL_SHARED)

#include <cstddef>

// use it in "Basic.h"
#ifndef UBPA_UDREFL_INCLUDE_ALL_STD_NAME
// #define UBPA_UDREFL_INCLUDE_ALL_STD_NAME
#endif // UBPA_UDREFL_INCLUDE_ALL_STD_NAME

namespace Ubpa::UDRefl {
    static constexpr std::size_t MaxArgNum = 64;
    static_assert(MaxArgNum <= 256 - 2);
} // namespace Ubpa::UDRefl
