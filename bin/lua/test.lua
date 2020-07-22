print("load test.lua")
print("load UDRefl_lua.dll")
local UDRefl_lua = require("UDRefl_luad")
print(UDRefl_lua.hello())
print("done")
print("you can use UDRefl_lua")

print("[lua] pointer: ", UDRefl_lua.TypeInfoMngr.GetTypeInfo(0))
local meta = getmetatable(UDRefl_lua.TypeInfoMngr.GetTypeInfo(0))
print("TypeInfo metatable name: " .. meta.__name)

local type = UDRefl_lua.TypeInfoMngr.GetTypeInfo(0)
local point = UDRefl_lua.TypeInfo.New(type)
UDRefl_lua.TypeInfo.Delete(point)

return UDRefl_lua
