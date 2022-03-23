require 'lasa'
local json = require 'test.dkjson'

local sock = lasa.socket.tcp()
local ok, err = sock:connect('127.0.0.1', 50002)
if err then
	print('client connect error: ', err)
	return
end

print('client connect success')

local function keep_alive()
	lasa.timer.at(3, function()
		local js = {}
		js.msgtype = 'publish'
		js.state_name = "keepalive"
		js.msgid = lasa.uuid()
		js.params = {state = 'alive'}
		local msg = json.encode(js)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		sock:send(msg)
		keep_alive()
	end)
end

local function playback_states_playing()
	lasa.timer.at(10, function()
		local js = {}
		js.msgtype = 'publish'
		js.state_name = "playback_states"
		js.msgid = lasa.uuid()
		js.params = {state = 'playing'}
		local msg = json.encode(js)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		sock:send(msg)
		print("======================== playback_states_playing", msg)
	end)
end

local function playback_states_paused()
	lasa.timer.at(6, function()
		local js = {}
		js.msgtype = 'publish'
		js.state_name = "playback_states"
		js.msgid = lasa.uuid()
		js.params = {state = 'paused'}
		local msg = json.encode(js)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		sock:send(msg)
		print("======================== playback_states_paused", msg)
	end)
end

local function playback_states_stop()
	lasa.timer.at(6, function()
		local js = {}
		js.msgtype = 'publish'
		js.state_name = "playback_states"
		js.msgid = lasa.uuid()
		js.params = {state = 'stop'}
		local msg = json.encode(js)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		sock:send(msg)
		print("======================== playback_states_paused", msg)
	end)
end

local function turn_off()
	lasa.timer.at(6, function()
		local js = {}
		js.msgtype = 'request'
		js.method = "turnoff"
		js.msgid = lasa.uuid()
		local msg = json.encode(js)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		sock:send(msg)
		print("======================== turn off", msg)
	end)
end

local function turn_on()
	lasa.timer.at(3, function()
		local js = {}
		js.msgtype = 'request'
		js.method = "turnon"
		js.msgid = lasa.uuid()
		local msg = json.encode(js)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		local bytes,err = sock:send(msg)
--		print("send status:",bytes,err)
		print("======================== turn on", msg)
	end)
end

local function get_music_list()
	lasa.timer.at(3, function()
		local js = {}
		js.msgtype = 'request'
		js.method = "getmusiclist"
		js.msgid = lasa.uuid()
		local msg = json.encode(js)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		sock:send(msg)
		print("======================== getmusiclist", msg)
	end)
end

local function re_get_music_list()
	lasa.timer.at(6, function()
		local js = {}
		js.msgtype = 'request'
		js.method = "getmusiclist"
		js.msgid = lasa.uuid()
		local msg = json.encode(js)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		sock:send(msg)
		print("======================== getmusiclist", msg)
	end)
end

--keep_alive()

--turn_off()
turn_on()
--get_music_list()
--re_get_music_list()
--playback_states_playing()
--playback_states_paused()
--playback_states_stop()

while true do
	local d, e, p = sock:receive('*l')
	if e then
		print('client receive error: ', e)
		break
	end
	
--	print('=====================>', d)
	
	local d, e, p = sock:receive('*l')
	if e then
		print('client receive error: ', e)
		break
	end
	
--	print('=====================>', d)
	
	local num = tonumber(d)
	local d, e, p = sock:receive(num)
	if e then
		print('client receive error: ', e)
		break
	end
	
--	print('=====================>', d)
	
	local js = json.decode(d)
	if not js then print('client receive not json') break end
	
	if js.msgtype == 'request' then
		print('=================playback client===========================')
		print("                 "..js.method)
--		print(d)
		print('===========================================================')
	
	
		local msg = {}
		msg.msgtype = 'response'
		msg.msgid = js.msgid
		msg.params = {}
		msg.params.state = 'success'
		local msg = json.encode(msg)
		msg = 'A\r\n'..#msg..'\r\n'..msg
		sock:send(msg)
--		print('send response :', js.msgid)
--		print(msg)
--		print("\n\n")
	elseif js.msgtype == 'response' then
		print(d)
	end
	
end

sock:close()

