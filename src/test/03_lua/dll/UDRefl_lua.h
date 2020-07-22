#pragma once

#include <lua.hpp>

#ifdef UBPA_EXPORT_UDREFL_LUA
#  define UDREFL_LUA_DESC __declspec(dllexport)
#else
#  define UDREFL_LUA_DESC __declspec(dllimport)
#endif

extern "C" UDREFL_LUA_DESC int luaopen_UDRefl_luad(lua_State * L);
