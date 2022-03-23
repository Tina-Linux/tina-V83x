require('luabin')
aiosResPath = aiosconf and aiosconf.resPath or 'res'
luabin.path = aiosResPath .. '/aios.lub'

local playernode = require 'aios.node.player'
playernode.run()
