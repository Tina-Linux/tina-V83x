require('luabin')
aiosResPath = aiosconf and aiosconf.resPath or 'res'
luabin.path = aiosResPath .. '/aios.lub'

if not aiosconf then
	aiosconf = {
		appKey    = "146182097285958e",
	    secretKey = "42794791306de0ac9fbf1edb90b0febc",
	    provision = "test/res/aios-1.0.0-146182097285958e.provision",
--	    cloudServer = "ws://s-test.api.aispeech.com:10000"
--		cloudServer = "ws://172.16.10.119:80"
		--cloudServer = "ws://112.80.39.95:8009"
		cloudServer = "ws://58.210.96.236:8888"
	}
end

local ttsnode = require 'aios.node.tts'
ttsnode.run()
