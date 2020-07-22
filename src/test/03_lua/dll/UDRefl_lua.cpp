#include "UDRefl_lua.h"
#include <iostream>

int hello(lua_State* L) {
	std::cout << "[cpp] : hello world!" << std::endl;
	lua_pushstring(L, "[lua] : hello world!");
	return 1;
}

int luaopen_UDRefl_lua(lua_State* L) {
	const struct luaL_Reg reg_UDRefl_lua[] = {
		{"hello", hello},
		{NULL, NULL}
	};
	luaL_newlib(L, reg_UDRefl_lua);
	return 1;
}
