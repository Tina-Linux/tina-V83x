require('luabin')
aiosResPath = aiosconf and aiosconf.resPath or 'res'
luabin.path = aiosResPath .. '/aios.lub'

local wakeupnode = require 'aios.node.wakeup'
wakeupnode.run()
