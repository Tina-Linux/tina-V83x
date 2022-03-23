local JSON = require "cjson"
function add(v1,v2)
    -- add positive numbers
    -- return 0 if any of the numbers are 0
    -- error if any of the two numbers are negative
    if v1 < 0 or v2 < 0 then
        error('Can only add positive or null numbers, received '..v1..' and '..v2)
    end
    if v1 == 0 or v2 == 0 then
        return 0
    end
    return v1+v2
end
function splitStrByChar(str,sepChar)
    local splitList = {}
    local pattern = '[^'..sepChar..']+'
    string.gsub(str, pattern, function(w) table.insert(splitList, w) end )
    return splitList
end

--nluDuration format:after|3000|sec
function getTimePartH_M_S_FromDuration(nluDuration)
    local splitList = splitStrByChar(nluDuration,'|')
    local durationSeconds = splitList[2]
    local currentOsTime = os.time()
    local hour = os.date("%H",currentOsTime + durationSeconds)
    local minute = os.date("%M",currentOsTime + durationSeconds)
    local second = os.date("%S",currentOsTime + durationSeconds)
    return hour,minute,second
end
function getDurationSecondValue(nluDuration)
    if(nluDuration==nil) then return nil end
    return splitStrByChar(nluDuration,'|')[2]
end
--nluTime format: at|08:00:00|pm
function getTimePartH_M_S(nluTime)
    local nluTimeLocal = nluTime
    local timeTable = {}
    local splitList = splitStrByChar(nluTimeLocal,'|')
    nluTimeLocal = splitList[2]
    local amPM = splitList[3]
    splitList = splitStrByChar(nluTimeLocal,':')
    timeTable["hour"] = splitList[1]
    if(amPM == 'pm' and tonumber(timeTable["hour"]) < 12) then
        timeTable["hour"] =  timeTable["hour"]+ 12
    end
    timeTable["min"] = splitList[2]
    timeTable["sec"] = splitList[3]
    return timeTable["hour"],timeTable["min"],timeTable["sec"]
end


function getDurationH_M_S(nluDate,nluTime)

    local nluTimeLocal = nluTime
    local timeTable = {}
    local splitList
    if(nluDate~=nil) then
        splitList = splitStrByChar(nluDate,'-')
        timeTable["year"] = splitList[1]
        timeTable["month"] = splitList[2]
        timeTable["day"] = splitList[3]
    else
        timeTable["year"] = os.date("%Y")
        timeTable["month"] = os.date("%m")
        timeTable["day"] = os.date("%d")
    end
    splitList = splitStrByChar(nluTimeLocal,'|')
    nluTimeLocal = splitList[2]
    local amPM = splitList[3]
    splitList = splitStrByChar(nluTimeLocal,':')
    timeTable["hour"] = splitList[1]
    if(amPM == 'pm' and tonumber(timeTable["hour"]) < 12) then
        timeTable["hour"] =  timeTable["hour"]+ 12
    end

    timeTable["min"] = splitList[2]
    timeTable["sec"] = splitList[3]
    local osTime = os.time(timeTable)
    local currentOsTime = os.time()
    local diffTime = os.difftime(osTime,currentOsTime)
	if(diffTime>0) then
		local diffHour,frag = math.modf(diffTime/(60*60))
		local diffMinute,frag = math.modf((diffTime/60)%60)
		local diffSecond,frag = math.modf(diffTime%60)
		return diffHour,diffMinute,diffSecond
	else
		return 0,0,0
	end

end

function removeElementByKey(tbl,key)
    --新建一个临时的table
    local tmp ={}

    --把每个key做一个下标，保存到临时的table中，转换成{1=a,2=c,3=b}
    --组成一个有顺序的table，才能在while循环准备时使用#table
    for i in pairs(tbl) do
        table.insert(tmp,i)
    end

    local newTbl = {}
    --使用while循环剔除不需要的元素
    local i = 1
    while i <= #tmp do
        local val = tmp [i]
        if val == key then
            --如果是需要剔除则remove
            table.remove(tmp,i)
        else
            --如果不是剔除，放入新的tabl中
            newTbl[val] = tbl[val]
            i = i + 1
        end
    end
    return newTbl
end
function getKVTable(t, keyName, valueName)
    local kvTable = {}
    for i = 1, #t do
        key = t[i][keyName]
        if(t[i][valueName]~=nil and type(t[i][valueName]) ~= "userdata") then
            val = t[i][valueName]
       else
            val = "null"
        end

        kvTable[key] = val
    end
    return kvTable
end

--说明：从nlu输出的intent 尚不明确，需结合品类再进一步判断
-- PI(parseIntent) 返回结果:
--deviceStartPI 开机,deviceStopPI 关机, deviceStartFunctionPI 启动功能/模式,deviceStopFunctionPI 停止功能/模式，
--appointmentPI 预约启动功能/模式,cancelAppointmentPI 取消预约,deviceVolumeControlPI 增加或减少参数,deviceSetFunctionPI设置参数
--deviceStatePI 查询状态
deviceStatePI = "deviceStatePI"
deviceStartPI = "deviceStartPI"
deviceStopPI = "deviceStopPI"
devicePausePI = "devicePausePI"
deviceStartFunctionPI = "deviceStartFunctionPI"
deviceStopFunctionPI = "deviceStopFunctionPI"
appointmentPI = "appointmentPI"
cancelAppointmentPI = "cancelAppointmentPI"
cancelAppointmentStartPI = "cancelAppointmentStartPI"
cancelAppointmentStopPI = "cancelAppointmentStopPI"
deviceVolumeControlPI = "deviceVolumeControlPI"
deviceSetFunctionPI = "deviceSetFunctionPI"
deviceResumePI = "deviceResumePI"
function parseIntent(nluIntent,slotItems)
    --取消预约
    if(nluIntent["deviceVerb"] == "cancelappointment" ) then
        print(cancelAppointmentPI)
        return cancelAppointmentPI
    end
    if(nluIntent["deviceVerb"] == "cancelappointmentstart" ) then
        print(cancelAppointmentStartPI)
        return cancelAppointmentStartPI
    end
    if(nluIntent["deviceVerb"] == "cancelappointmentstop" ) then
        print(cancelAppointmentStopPI)
        return cancelAppointmentStopPI
    end

    --查询状态
    if(nluIntent["intentType"]=="deviceState") then
        print(deviceStatePI)
		return deviceStatePI
	end

	--预约启停功能模式或开关机
    if(nluIntent["intentType"] == "deviceSetFunction" or nluIntent["intentType"] == "deviceStart" or nluIntent["intentType"] == "deviceStop") then
        if(nluIntent["deviceVerb"] == "appointment"
                or(slotItems["time"]~=nil )
                or (slotItems["duration"]~=nil and string.find(slotItems["duration"],"after")==1)
        ) then
            print(appointmentPI)
            return appointmentPI
        end
    end
   --开机
    if((nluIntent["intentType"] == "deviceStart" or nluIntent["intentType"] == "deviceOpen" ) and slotItems["deviceMode"]==nil and  slotItems["deviceAspect"]==nil) then
        print(deviceStartPI)
        return deviceStartPI
    end
    --关机
    if((nluIntent["intentType"] == "deviceStop" or nluIntent["intentType"] == "deviceClose") and slotItems["deviceMode"]==nil and  slotItems["deviceAspect"]==nil) then
        print(deviceStopPI)
        return deviceStopPI
    end
    --暂停
    if(nluIntent["intentType"] == "devicePause") then
        print(devicePausePI)
        return devicePausePI
    end
    if(nluIntent["intentType"] == "deviceResume") then
        print(deviceResumePI)
        return deviceResumePI
    end

    --启动功能或模式
    if(((nluIntent["intentType"] == "deviceStart" or nluIntent["intentType"] == "deviceOpen" )and (slotItems["deviceMode"]~=nil or  slotItems["deviceAspect"]~=nil))
       or(nluIntent["intentType"] == "deviceSetFunction" and slotItems["deviceMode"]~=nil)) then
        print(deviceStartFunctionPI)
        return deviceStartFunctionPI
    end
    --停止功能或模式
    if((nluIntent["intentType"] == "deviceStop" or nluIntent["intentType"] == "deviceClose") and (slotItems["deviceMode"]~=nil or  slotItems["deviceAspect"]~=nil)) then
        print(deviceStopFunctionPI)
        return deviceStopFunctionPI
    end

    --设置风力等参数
    if(nluIntent["intentType"]=="deviceSetFunction") then
        print(deviceSetFunctionPI)
        return    deviceSetFunctionPI
    end
    --增加或减小风力等参数
    if(nluIntent["intentType"]=="deviceVolumeControl") then
        print(deviceVolumeControlPI)
        return deviceVolumeControlPI
    end


end
--getDurationH_M_S(nil,'at|19:00:00|')
--h,m,s = getTimePartH_M_S_FromDuration('after|3600|sec')
--print(h,m,s)
