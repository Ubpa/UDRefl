#pragma once

#if defined(_WIN32) && defined(UBPA_UDREFL_BUILD_AS_SHARED)
#  if defined(UBPA_UDREFL_EXPORT)
#    define UDREFL_DESC __declspec(dllexport)
#  else
#    define UDREFL_DESC __declspec(dllimport)
#  endif
#else
#  define UDREFL_DESC 
#endif
