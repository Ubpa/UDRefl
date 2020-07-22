#include "UDRefl_lua.h"
#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa;
using namespace std;

int hello(lua_State* L) {
	cout << "[cpp] : hello world!" << endl;
	lua_pushstring(L, "[lua] : hello world!");
	return 1;
}

namespace UDRefl_lua {
	struct MetaName {
		static constexpr const char TypeInfo[] = "UDRefl_lua.TypeInfo";
		static constexpr const char Object[] = "UDRefl_lua.Object";
	};
	namespace TypeInfoMngr {
		int GetTypeInfo(lua_State* L) {
			auto id = static_cast<size_t>(lua_tointeger(L, 1));
			auto typeinfo = UDRefl::TypeInfoMngr::Instance().GetTypeInfo(id);
			cout << typeinfo << endl;
			auto typeinfo_lua = lua_newuserdata(L, sizeof(UDRefl::TypeInfo*));
			memcpy(typeinfo_lua, &typeinfo, sizeof(UDRefl::TypeInfo*));
			luaL_getmetatable(L, MetaName::TypeInfo);
			lua_setmetatable(L, -2);
			return 1;
		}
	}
	namespace TypeInfo {
		int New(lua_State* L) {
			auto pptypeinfo = (UDRefl::TypeInfo**)luaL_checkudata(L, 1, MetaName::TypeInfo);
			auto obj = (*pptypeinfo)->New();
			auto obj_lua = lua_newuserdata(L, sizeof(UDRefl::Object));
			memcpy(obj_lua, &obj, sizeof(UDRefl::Object));
			luaL_getmetatable(L, MetaName::Object);
			lua_setmetatable(L, -2);
			return 1;
		}

		int Delete(lua_State* L) {
			auto pptypeinfo = (UDRefl::TypeInfo**)luaL_checkudata(L, 1, MetaName::TypeInfo);
			auto obj = (UDRefl::Object*)luaL_checkudata(L, 2, MetaName::Object);
			(*pptypeinfo)->Delete(*obj);
			return 0;
		}
	}
}

void Register_TypeInfoMngr(lua_State* L) {
	lua_newtable(L); // UDRefl_lua.TypeInfoMngr
	const struct luaL_Reg reg_TypeInfoMngr[] = {
		{"GetTypeInfo", UDRefl_lua::TypeInfoMngr::GetTypeInfo},
		{NULL, NULL}
	};
	luaL_setfuncs(L, reg_TypeInfoMngr, 0);
	lua_setfield(L, -2, "TypeInfoMngr");
}

void Register_TypeInfo(lua_State* L) {
	lua_newtable(L); // UDRefl_lua.TypeInfo
	const struct luaL_Reg reg_TypeInfo[] = {
		{"New", UDRefl_lua::TypeInfo::New},
		{"Delete", UDRefl_lua::TypeInfo::Delete},
		{NULL, NULL}
	};
	luaL_setfuncs(L, reg_TypeInfo, 0);
	lua_setfield(L, -2, "TypeInfo");
}

int luaopen_UDRefl_luad(lua_State* L) {
	luaL_newmetatable(L, UDRefl_lua::MetaName::TypeInfo);
	luaL_newmetatable(L, UDRefl_lua::MetaName::Object);
	lua_pop(L, 2);

	const struct luaL_Reg reg_UDRefl_lua[] = {
		{"hello", hello},
		{NULL, NULL}
	};
	luaL_newlib(L, reg_UDRefl_lua);
	Register_TypeInfoMngr(L);
	Register_TypeInfo(L);
	return 1;
}
