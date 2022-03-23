#if defined(LOCAL_ASR)&&defined(LOCAL_ASR_AC)

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif


void json2hex(char *data,struct localasr_proc *proc)
{
	lua_State *L = luaL_newstate();
    char *ret;
	char buf[30];
	int hexlen=0;
	size_t len;
	 struct mcrc_speech_user *user;

	luaL_openlibs(L);

	//if(luaL_loadfile(L, "/oem/lua/T_0000_AC_26.lua")||lua_pcall(L, 0,0,0)){
	if(luaL_loadfile(L, "/mnt/app/lua/T_0000_AC_26.lua")||lua_pcall(L, 0,0,0)){
        MS_TRACE("error %s\n", lua_tostring(L,-1));
        return -1;
    }
	lua_getglobal(L,"jsonToData");
	//lua_pushlightuserdata(L,data);
	lua_pushstring(L,data);
	lua_pcall(L, 1,1,0);

	ret = lua_tolstring(L,-1,&len);

//	MS_TRACE("str result=%s,%d\r\n",ret,len);
	//AscToHex(ret,buf,&hexlen);

	        cJSON* root = cJSON_CreateObject();
	        cJSON* cmd = cJSON_CreateObject();



			//cJSON_AddItemToObject(root,"cmd",cmd);

		//	if(proc->cmd.cmdtype == LOCAL_CMD_OPEN_LIGHT)
		//		{
		//	cJSON_AddStringToObject(cmd,"cmd","aa0fb1000000000300022202ffff0118");
		//		}
		//	else if(proc->cmd.cmdtype == LOCAL_CMD_CLOSE_LIGHT)
		//		{
			//	cJSON_AddStringToObject(cmd,"cmd","aa0fb1000000000300022202ffff0019");
			//	}
			//else{
				cJSON_AddStringToObject(cmd, "cmd",ret);
			//	}
			cJSON_AddItemToObject(root,"cmd",cmd);
			char *out = cJSON_Print(root);
         //   debug(LOG_LEVEL_DEBUG,"M_OUT_HEX_CMD is %s,%d\n",out,proc->cmd.cmdtype);

            int msui_ret = MSUI_CB_RET_SUCCESS;
            struct speech_usrdata msui_data;
            msui_data.event = MSUI_EV_OUT_CMD;
            strcpy(msui_data.out_res, out);
			user =proc->user;
            msui_ret = user->_handler(&msui_data);

          //  MS_TRACE("[msui_process] get user handler return %d\n", msui_ret);
            free(out);
		    cJSON_Delete(root);
		    lua_close(L);
			local_timer_init();
			local_timer_start();
            MS_TRACE("cmdtype=%d\r\n",proc->cmd.cmdtype);

}
int AscToHex(char *str,unsigned char **hex,int *hexlen)
{
    unsigned char odd;
    unsigned char even;
	char *ptr=str;
	unsigned char *hexPtr=hex;
	int i =0;

	int len = strlen(str);

	if(len%2){
		MS_TRACE("is not even\r\n");
		return -1;
	}
	while(i < len){
		odd = *(ptr+i);
		even = *(ptr+i+1);
		//MS_TRACE("odd=%x,%x,%x\r\n",odd,even,hexPtr);

		if((odd>=0x30)&&(odd<=0x39))
		odd -= 0x30;
		else if((odd>=0x41)&&(odd<=0x46))// upper letter
		odd -= 0x37;
		else if((odd>=0x61)&&(odd<=0x66))//Lowercase letter
		odd -= 0x57;
		else odd = 0xff;

		if((even>=0x30)&&(even<=0x39))
		even -= 0x30;
		else if((even>=0x41)&&(even<=0x46))// upper letter
		even -= 0x37;
		else if((even>=0x61)&&(even<=0x66))//Lowercase letter
		even -= 0x57;
		else even = 0xff;

               (*hexPtr) = (unsigned char)(((odd<<4)&0xf0)|(even&0x0f));
MS_TRACE("hex=%x,",*hexPtr);
		hexPtr++;
		(*hexlen)++;
		i +=2;

	}
	return 0;

}
#endif
