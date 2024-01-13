/**
 * File              : unrtf.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.01.2024
 * Last Modified Date: 14.01.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "globals.h"
#include "unrtf.h"
#include "images/stb_image.h"
#include "images/stb_image_write.h"
#include <lua.h>

static int paragraph_start(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 2);
	lua_call(L, 0, 0);
	return 0;
} 

static int paragraph_end(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 3);
	lua_call(L, 0, 0);
	return 0;
} 

static int bold_start(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 4);
	lua_call(L, 0, 0);
	return 0;
} 

static int bold_end(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 5);
	lua_call(L, 0, 0);
	return 0;
} 

static int italic_start(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 6);
	lua_call(L, 0, 0);
	return 0;
} 

static int italic_end(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 7);
	lua_call(L, 0, 0);
	return 0;
} 

static int underline_start(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 8);
	lua_call(L, 0, 0);
	return 0;
} 

static int underline_end(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 9);
	lua_call(L, 0, 0);
	return 0;
} 

static int table_start(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 10);
	lua_call(L, 0, 0);
	return 0;
} 

static int table_end(void *u){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 11);
	lua_call(L, 0, 0);
	return 0;
} 

static int tablerow_width(void *u, int i, int w)
{
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 12);
	lua_pushinteger(L, i);
	lua_pushinteger(L, w);
	lua_call(L, 2, 0);
	return 0;
}	

static int tablerow_start(void *u, int n){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 13);
	lua_pushinteger(L, n);
	lua_call(L, 1, 0);
	return 0;
} 

static int tablerow_end(void *u, int n){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 14);
	lua_pushinteger(L, n);
	lua_call(L, 1, 0);
	return 0;
} 

static int tablecell_start(void *u, int n){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 15);
	lua_pushinteger(L, n);
	lua_call(L, 1, 0);
	return 0;
} 

static int tablecell_end(void *u, int n){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 16);
	lua_pushinteger(L, n);
	lua_call(L, 1, 0);
	return 0;
} 

static int style(void *u, const char *s){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 17);
	lua_pushlstring(L, s, strlen(s));
	lua_call(L, 1, 0);
	return 0;
}	
	
static int text(void *u, const char *s, int len){
	lua_State* L = (lua_State*)u;
	lua_pushvalue(L, 18);
	lua_pushlstring(L, s, len);
	lua_call(L, 1, 0);
	return 0;
}

static int image(void *u, const unsigned char *s, size_t len){
	// get image data
	int x, y, c;
  stbi_uc *image = 
		stbi_load_from_memory(s, len,
			 	&x, &y, 
				&c, 0);

	if (!image)
		return 0;
		
	lua_State* L = (lua_State*)u;
	lua_call(L, 0, 1);
	
	size_t size;
	const char* filename = 
		luaL_checklstring(L, -1, &size);

	if (filename){
		stbi_write_jpg(filename, x, y,
			 	c, image, 90);
		stbi_image_free(image);
	}

	return 0;
}

static int unrtf_cb(lua_State* L)
{
	size_t size;
	const char* filename = luaL_checklstring(L, 1, &size);

	unrtf_parse(
			filename, 
			(void *)L, 
			paragraph_start, 
			paragraph_end, 
			bold_start, 
			bold_end, 
			italic_start, 
			italic_end, 
			underline_start, 
			underline_end, 
			table_start, 
			table_end, 
			tablerow_width, 
			tablerow_start, 
			tablerow_end, 
			tablecell_start, 
			tablecell_end, 
			style, 
			text,
			image);
	return 0;
}

void unrtf_init(void)
{
	const static luaL_Reg funcs[] =
	{
		{ "unrtf",                 unrtf_cb },
		{ NULL,                    NULL }
	};

	lua_getglobal(L, "wg");
	luaL_setfuncs(L, funcs, 0);
}
