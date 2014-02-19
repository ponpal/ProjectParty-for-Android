#include <lua.hpp>

#ifdef __cplusplus
extern "C" {
#endif
#include <string.h>
#include <jni.h>
#include <android/log.h>

jint
Java_projectparty_ppandroid_MainActivity_cmain( JNIEnv*  env,
                                      jobject  obj)
{

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	char buff[256] = "x=5\ny=3";
	int error = luaL_loadbuffer(L, buff, strlen(buff), "line")||
			lua_pcall(L,0,0,0);
	lua_getglobal(L, "x");
	lua_getglobal(L, "y");

	if (!lua_isnumber(L, -2))
		__android_log_write(ANDROID_LOG_INFO,
				"LUALOG", "No number...");

	if (!lua_isnumber(L, -1))
		__android_log_write(ANDROID_LOG_INFO,
				"LUALOG", "No number...");

	return 5;
}

#ifdef __cplusplus
}
#endif
