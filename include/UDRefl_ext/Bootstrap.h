#pragma once

#if (defined(WIN32) || defined(_WIN32)) && defined(UBPA_UDREFL_SHARED)
#ifdef UCMAKE_EXPORT_UDRefl_ext_Bootstrap
#define UDRefl_ext_Bootstrap_API __declspec(dllexport)
#else
#define UDRefl_ext_Bootstrap_API __declspec(dllimport)
#endif
#else
#define UDRefl_ext_Bootstrap_API extern
#endif // (defined(WIN32) || defined(_WIN32)) && defined(UBPA_UDREFL_SHARED)

#ifdef __cplusplus
extern "C" {
#endif

UDRefl_ext_Bootstrap_API void Ubpa_UDRefl_ext_Bootstrap();

#ifdef __cplusplus
}
#endif
