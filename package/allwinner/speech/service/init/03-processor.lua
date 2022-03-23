require('luabin')
aiosResPath = aiosconf and aiosconf.resPath or 'res'
luabin.path = aiosResPath .. '/aios.lub;' .. aiosResPath .. '/sdsres/sds.lub'

local projectname

if not aiosconf then
	aiosconf = {
		appKey    = "146182097285958e",
	    secretKey = "42794791306de0ac9fbf1edb90b0febc",
	    provision = "test/res/aios-1.0.0-146182097285958e.provision",
	    serial    = "test/res/aios-aihome.serial",
	    userId    = "soundbox",
	    ailog_level = 3,
	}
end


aiosconf.asrtype = 'asrcloud'



local processor = require 'aios.processor'
processor.run()
