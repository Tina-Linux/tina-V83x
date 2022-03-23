#if defined(LOCAL_ASR)&&defined(LOCAL_ASR_AC)


#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <math.h>
#include "cJSON.h"
#include <pthread.h>
#include <unistd.h>
#include "duilite.h"
#include "local_asr.h"
#include "debug.h"
#include "mcrc_speech_user_intf.h"
#include "ms_common.h"

char * data2json(char *data,unsigned char cmdtype)
{
   lua_State *L = luaL_newstate();
    char *ret;
	char buf[30];
	int hexlen=0;
	size_t len;


	luaL_openlibs(L);

	//if(luaL_loadfile(L, "/oem/lua/T_0000_AC_26.lua")||lua_pcall(L, 0,0,0)){
	if(luaL_loadfile(L, "/mnt/app/lua/T_0000_AC_26.lua")||lua_pcall(L, 0,0,0)){
        MS_TRACE("error %s\n", lua_tostring(L,-1));
        return -1;
    }
	lua_getglobal(L,"dataToJson");
	//lua_pushlightuserdata(L,data);
	lua_pushstring(L,data);
	lua_pcall(L, 1,1,0);

	ret = lua_tolstring(L,-1,&len);

	MS_TRACE("str hextojson:result=%s,%d\r\n",ret,len);


    result_parse(ret,cmdtype);

    lua_close(L);



	return ret;

}

char * data03json(char *data)
{
   lua_State *L = luaL_newstate();
    char *ret;
	char buf[30];
	int hexlen=0;
	size_t len;


	luaL_openlibs(L);

	//if(luaL_loadfile(L, "/oem/lua/T_0000_AC_26.lua")||lua_pcall(L, 0,0,0)){
	if(luaL_loadfile(L, "/mnt/app/lua/T_0000_AC_26.lua")||lua_pcall(L, 0,0,0)){
        MS_TRACE("error %s\n", lua_tostring(L,-1));
        return -1;
    }
	lua_getglobal(L,"dataToJson");
	//lua_pushlightuserdata(L,data);
	lua_pushstring(L,data);
	lua_pcall(L, 1,1,0);

	ret = lua_tolstring(L,-1,&len);

//	MS_TRACE("str hextojson:result=%s,%d\r\n",ret,len);


    result_03parse(ret);

	lua_close(L);



	return ret;

}
#endif
