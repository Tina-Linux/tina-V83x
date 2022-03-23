require('luabin')
luabin.path = 'res/aios.lub'
local busclient = require('aios.bus.client')
local bc = busclient.new()
if bc then
    bc:call('/bus/exit')
    bc:delete()
end