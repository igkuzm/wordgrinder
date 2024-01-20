/**
 * File              : rtf.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.01.2024
 * Last Modified Date: 20.01.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "globals.h"
#include "rtf/rtfreadr.h"
#include "images/stb_image.h"
#include "images/stb_image_write.h"
#include <lua.h>

struct unrtf_s {
	lua_State* L;
	struct str str;
	bool fBold;
	bool fUnderline;
	bool fItalic;
	bool fTable;
};

void flush_str(struct unrtf_s *s){
	lua_pushvalue(s->L, 18);
	lua_pushlstring(L, s->str.str, s->str.len);
	lua_call(L, 1, 0);
	str_init(&s->str, BUFSIZ);
}

int par_cb    (void *udata, PAP *p){
	struct unrtf_s *s = udata;

	return 0;
}

int row_cb    (void *udata, TRP *p){
	struct unrtf_s *s = udata;

	return 0;
}

int cell_cb   (void *udata, TCP *p){
	struct unrtf_s *s = udata;

	return 0;
}

int char_cb   (void *udata, int ch, CHP *p){
	struct unrtf_s *s = udata;
	if (s->fBold != p->fBold){
		s->fBold = p->fBold;
		// start bold
		lua_pushvalue(s->L, 4);
		lua_call(s->L, 0, 0);
		// flush str
		flush_str(s);
		return 0;
	}

	char c = ch;
	str_append(&s->str, &c, 1);
	return 0;
}

int pict_cb   (void *udata, PICT *p){
	struct unrtf_s *s = udata;

	return 0;
}

static int unrtf_cb(lua_State* L)
{
	size_t size;
	const char* filename = luaL_checklstring(L, 1, &size);

	FILE *fp = fopen(filename, "r");
	if (!fp)
		return 1;
	
	rprop_t p;
	rnotify_t n;
	memset(&(n), 0, sizeof(rnotify_t));

	struct unrtf_s s;
	memset(&(s), 0, sizeof(struct unrtf_s));
	s.L = L;
	str_init(&s.str, BUFSIZ);

	n.udata = L;
	n.par_cb = par_cb;
	n.row_cb = row_cb;
	n.char_cb = char_cb;
	n.pict_cb = pict_cb;
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
