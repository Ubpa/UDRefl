print("load test.lua")
print("load UDRefl_lua.dll")
local UDRefl_lua = require("UDRefl_lua")
print(UDRefl_lua.hello())
print("done")
print("you can use UDRefl_lua")
return UDRefl_lua
