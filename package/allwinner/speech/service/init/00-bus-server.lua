require('luabin')
aiosResPath = aiosconf and aiosconf.resPath or 'res'
luabin.path = aiosResPath .. '/aios.lub'


local busserver = require('aios.bus.server')
busserver.run()
