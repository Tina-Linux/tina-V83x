require('luabin')
aiosResPath = aiosconf and aiosconf.resPath or 'res'
luabin.path = aiosResPath .. '/aios.lub'

if not aiosconf then
	aiosconf = {
		srctype='aec.pcm'
	}
end

local recordernode = require 'aios.node.recorder'
recordernode.run()

