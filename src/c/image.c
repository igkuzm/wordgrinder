/**
 * File              : image.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.01.2024
 * Last Modified Date: 16.01.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "globals.h"
#include "images/stb_image.h"
#include "images/image2ascii.h"
#include "images/image2rtf.h"
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

/* Parse image. */
static int parseimage_cb_cb(
		void *userdata, int len, const char *row)
{
	lua_State* L = (lua_State*) userdata;
	/* pos 3 contains the callback function */
	lua_pushvalue(L, 3);
	lua_pushlstring(L, row, len);
	lua_call(L, 1, 0);

	return 0;
}

static int parseimage_cb(lua_State* L)
{
	/* pos 1 contains filepath */
	size_t size;
	const char* filepath = luaL_checklstring(L, 1, &size);
	
	/* pos 2 contains number of cols */
	int cols = forceinteger(L, 2);

	image2ascii(filepath, cols, 0,
		 	(void*)L, parseimage_cb_cb);

	return 0;
}

/* Image to RTF. */
static void imagetortf_cb_cb(
		void *userdata, const char *rtf)
{
	lua_State* L = (lua_State*) userdata;
	/* pos 2 contains the callback function */
	lua_pushvalue(L, 2);
	lua_pushlstring(L, rtf, strlen(rtf));
	lua_call(L, 1, 0);
}

static int imagetortf_cb(lua_State* L)
{
	/* pos 1 contains filepath */
	size_t size;
	const char* filepath = luaL_checklstring(L, 1, &size);
	
	image2rtf(filepath, (void*)L,
		 	imagetortf_cb_cb);

	return 0;
}

void image_init(void)
{
	const static luaL_Reg funcs[] =
	{
		{ "getimagesize", getimagesize_cb },
		{ "parseimage",   parseimage_cb },
		{ "imagetortf",   imagetortf_cb },
		{ NULL,           NULL }
	};

	lua_getglobal(L, "wg");
	luaL_setfuncs(L, funcs, 0);
}
