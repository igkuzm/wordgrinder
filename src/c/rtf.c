/**
 * File              : rtf.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.01.2024
 * Last Modified Date: 20.01.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "globals.h"
#include "rtf/rtfreadr.h"
#include "rtf/rtftype.h"
#include "rtf/str.h"
#include "images/stb_image.h"
#include "images/stb_image_write.h"
#include <lua.h>
#include <stdio.h>

struct unrtf_t {
	struct str str;
	lua_State *L;
	char fBold;
	char fUnderline;
	char fItalic;
	char incell;
	int  ncells;
	bool bordered;
	STYLE styles[32];
	int nstyles;
};

void flushstring(struct unrtf_t *t)
{
	lua_pushvalue(t->L, 6);
	lua_pushlstring(t->L, t->str.str, t->str.len);
	lua_call(t->L, 1, 0);
	str_init(&t->str, BUFSIZ);
}

int style_cb(void *d, STYLE *s)
{
	struct unrtf_t *t = d;
	t->styles[t->nstyles++] = *s;

	return 0;
}

int info_cb(void *d, INFO_T t, const char *s)
{

	return 0;
}

int row_cb(void *d, TRP *rp){
	struct unrtf_t *t = d;

	if (
			rp->bordB ||
			rp->bordH ||
			rp->bordL ||
			rp->bordR ||
			rp->bordT ||
			rp->bordV
		 )
		t->bordered = true;

	lua_pushvalue(t->L, 4);
	lua_pushboolean(t->L, t->bordered);
	lua_call(t->L, 1, 0);

	t->incell = false;
	return 0;
}

int cell_cb(void *d, TRP *rp, TCP *cp){
	struct unrtf_t *t = d;
	t->incell = true;

	if (
			cp->bordB ||
			cp->bordL ||
			cp->bordR ||
			cp->bordT
		 )
		t->bordered = true;

	if (rp->ncellx)
		t->ncells = rp->ncellx;

	flushstring(t);

	lua_pushvalue(t->L, 5);
	lua_pushnumber(t->L, t->ncells);
	lua_call(t->L, 1, 0);
	return 0;
}

void par_get_style(struct unrtf_t *t, PAP *p, char style[32])
{
	// handle with paragraph style
	strcpy(style, "P");
	if (p->just == justC)
		strcpy(style, "CENTER");

	if (p->just == justR)
		strcpy(style, "RIGHT");
	
	if (p->just == justL)
		strcpy(style, "LEFT");

	// get from styles
	int i;
	for (i = 0; i < t->nstyles; ++i) {
		STYLE s = t->styles[i];
		if (
				p->s == s.s &&
				(
					strcmp(s.name, "P")  == 0 ||	
					strcmp(s.name, "H1") == 0 ||
					strcmp(s.name, "H2") == 0 ||
					strcmp(s.name, "H3") == 0 ||
					strcmp(s.name, "H4") == 0 ||
					strcmp(s.name, "Q")  == 0 ||
					strcmp(s.name, "LB") == 0 ||
					strcmp(s.name, "LN") == 0 ||
					strcmp(s.name, "L")  == 0 ||
					strcmp(s.name, "V")  == 0 ||
					strcmp(s.name, "PRE")== 0 ||
					strcmp(s.name, "RAW")== 0
				)
			)
		strncpy(style, s.name, 31);
	}
}

int par_cb(void *d, PAP *p)
{
	struct unrtf_t *t = d;
	if (t->incell)
		return 0;

	flushstring(t);
	
	char style[32];
	par_get_style(t, p, style);

	// add paragraph
	lua_pushvalue(t->L, 2);
	lua_pushstring(t->L, style);
	lua_call(t->L, 1, 0);
	return 0;
}

int pict_cb(void *d, PICT *pict, PAP *p)
{
	struct unrtf_t *t = d;
	flushstring(t);
	
	char style[32];
	par_get_style(t, p, style);
	style[31] = 0;
	
	// get image data
	int x, y, c;
  stbi_uc *image = 
		stbi_load_from_memory(pict->data, pict->len,
			 	&x, &y, 
				&c, 0);

	if (!image)
		return 0;
	
		
	lua_pushvalue(t->L, 7);
	lua_pushstring(t->L, style);
	lua_call(t->L, 1, 1);
	
	size_t size;
	const char* filename = 
		luaL_checklstring(t->L, -1, &size);

	if (filename){
		stbi_write_jpg(filename, x, y,
			 	c, image, 90);
		stbi_image_free(image);
	}

	return 0;
}

void flushstyle(struct unrtf_t *t, int STY, bool val){
	lua_pushvalue(t->L, 3);
	lua_pushnumber(t->L, STY);
	lua_pushboolean(t->L, val);
	lua_call(t->L, 2, 0);
}

int char_cb(void *d, int ch, CHP *p)
{
	struct unrtf_t *t = d;
	
	if (
			t->fBold != p->fBold           ||
		  t->fUnderline != p->fUnderline ||
		  t->fItalic != p->fItalic       
			)
	{
		flushstring(t);
			
		int STY; 
		
		if (t->fBold != p->fBold){
			t->fBold = p->fBold;
			STY = DPY_BOLD;
			if (p->fBold == 1)
				flushstyle(t, STY, true);
			else
				flushstyle(t, STY, false);
		}
		
		if (t->fUnderline != p->fUnderline){
			t->fUnderline = p->fUnderline;
			STY = DPY_UNDERLINE;
			if (p->fUnderline == 1)
				flushstyle(t, STY, true);
			else 
				flushstyle(t, STY, false);
		}
		
		if (t->fItalic != p->fItalic){
			t->fItalic = p->fItalic;
			STY = DPY_ITALIC;
			if (p->fItalic == 1)
				flushstyle(t, STY, true);
			else
				flushstyle(t, STY, false);
		}
	}

	// add char to buffer
	char c = ch;
	str_append(&t->str, &c, 1);
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

	struct unrtf_t t;
	memset(&(t), 0, sizeof(struct unrtf_t));
	t.L = L;
	str_init(&t.str, BUFSIZ);

	n.udata = &t;
	n.char_cb = char_cb;
	n.par_cb = par_cb;
	n.row_cb = row_cb;
	n.cell_cb = cell_cb;
	n.style_cb = style_cb;
	n.pict_cb = pict_cb;
	//n.info_cb = info_cb;
	//n.date_cb = date_cb;

	int ec = ecRtfParse(fp, &p, &n);
	fclose(fp);

	return ec;
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
