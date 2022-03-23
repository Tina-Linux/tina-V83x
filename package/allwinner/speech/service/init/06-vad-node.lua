require('luabin')
aiosResPath = aiosconf and aiosconf.resPath or 'res'
luabin.path = aiosResPath .. '/aios.lub'


local vadnode = require 'aios.node.vad'
vadnode.run()
