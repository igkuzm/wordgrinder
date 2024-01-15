/**
 * File              : image.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.01.2024
 * Last Modified Date: 15.01.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "globals.h"
#include "images/stb_image.h"
#include <lua.h>

static int getimagesize_cb(lua_State* L)
{
	size_t size;
	const char* filename = luaL_checklstring(L, 1, &size);

	int x, y, c;
	if (stbi_info(filename, &x, &y, &c)){
		lua_pushvalue(L, 2);
		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		lua_pushnumber(L, c);
		lua_call(L, 3, 0);
	
		lua_pushvalue(L, -1);
		lua_pushboolean(L, true);
		return 1;
	}

	return 0;
}

void image_init(void)
{
	const static luaL_Reg funcs[] =
	{
		{ "getimagesize",           getimagesize_cb },
		{ NULL,                    NULL }
	};

	lua_getglobal(L, "wg");
	luaL_setfuncs(L, funcs, 0);
}
