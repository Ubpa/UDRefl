#include <lua.hpp>

int main() {
	char buff[256];
	int error;
	lua_State* L = luaL_newstate(); /* opens Lua */
	luaL_openlibs(L); /* opens the standard libraries */

	// 1. load UDRefl_lua
	error = luaL_loadstring(L, "UDRefl_lua = require(\"lua/test\")") || lua_pcall(L, 0, 0, 0);
	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1); /* pop error message from the stack */
	}

	// 2. run CLI
	while (fgets(buff, sizeof(buff), stdin) != NULL) {
		error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
		if (error) {
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1); /* pop error message from the stack */
		}
	}
	lua_close(L);
	return 0;
}
