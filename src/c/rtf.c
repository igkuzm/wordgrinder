/**
 * File              : rtf.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.01.2024
 * Last Modified Date: 05.08.2024
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
	int  cell;
	int  ncells;
	bool bordered;
	STYLE styles[32];
	int nstyles;
	bool flushPageProp;
};

int command_cb(
		void *udata, const char *s, int param, char fParam){
	return 0;
}

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

int info_cb(void *d, tINFO t, const char *s)
{

	return 0;
}

void flushrow(struct unrtf_t *t, TRP *rp){
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

	t->cell = 0;
}

void flushcell(struct unrtf_t *t, prop_t *p){
	if (
			p->tcp.bordB ||
			p->tcp.bordL ||
			p->tcp.bordR ||
			p->tcp.bordT
		 )
		t->bordered = true;

	if (p->trp.ncellx)
		t->ncells = p->trp.ncellx;

	int len = 0;
	if (t->cell < p->trp.ncellx)
		len = p->trp.cellx[t->cell];

	t->cell++;

	flushstring(t);

	lua_pushvalue(t->L, 5);
	lua_pushnumber(t->L, t->ncells);
	lua_pushnumber(t->L, len);
	lua_call(t->L, 2, 0);
}

void par_get_style(struct unrtf_t *t, PAP *p, char style[32])
{
	// handle with paragraph style
	strcpy(style, "P");
	
	if (p->just == justF)
		strcpy(style, "BOTH");

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

void flushparagraph(struct unrtf_t *t, PAP *p)
{
	// skip if in table
	if (p->fIntbl == 1)
		return;

	flushstring(t);
	
	char style[32];
	par_get_style(t, p, style);

	// add paragraph
	lua_pushvalue(t->L, 2);
	lua_pushstring(t->L, style);
	lua_call(t->L, 1, 0);
}

int pict_cb(void *d, prop_t *p, PICT *pict)
{
	struct unrtf_t *t = d;
	flushstring(t);
	
	char style[32];
	par_get_style(t, &p->pap, style);
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

static void flushpageprop(struct unrtf_t *t, prop_t *p){
	lua_pushvalue(t->L, 8);
	lua_pushnumber(t->L, p->dop.xaPage);
	lua_pushnumber(t->L, p->dop.yaPage);
	lua_pushnumber(t->L, p->dop.xaLeft);
	lua_pushnumber(t->L, p->dop.xaRight);
	lua_pushnumber(t->L, p->dop.yaTop);
	lua_pushnumber(t->L, p->dop.yaBottom);
	lua_call(t->L, 6, 0);
}

int char_cb(void *d, STREAM s, prop_t *p, int ch)
{
	struct unrtf_t *t = d;
	if (!t->flushPageProp){
		if (p->dop.xaPage){
			flushpageprop(t, p);
			t->flushPageProp = true;
		}
	}

	if (s != sMain)
		return 0;

	if (ch > 256) {
		switch (ch) {
			case PAR:
				flushparagraph(t, &p->pap);
				break;
			case ROW:
				flushrow(t, &p->trp);
				break;
			case CELL:
				flushcell(t, p);
				break;
			
			default:
				break;	
		}

		return 0;
	}
	
	if (
			t->fBold != p->chp.fBold           ||
			t->fUnderline != p->chp.fUnderline ||
			t->fItalic != p->chp.fItalic       
			)
	{
		flushstring(t);
			
		int STY; 
		
		if (t->fBold != p->chp.fBold){
			t->fBold = p->chp.fBold;
			STY = DPY_BOLD;
			if (p->chp.fBold == 1)
				flushstyle(t, STY, true);
			else
				flushstyle(t, STY, false);
		}
		
		if (t->fUnderline != p->chp.fUnderline){
			t->fUnderline = p->chp.fUnderline;
			STY = DPY_UNDERLINE;
			if (p->chp.fUnderline == 1)
				flushstyle(t, STY, true);
			else 
				flushstyle(t, STY, false);
		}
		
		if (t->fItalic != p->chp.fItalic){
			t->fItalic = p->chp.fItalic;
			STY = DPY_ITALIC;
			if (p->chp.fItalic == 1)
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

	prop_t p;
	rnotify_t n;
	memset(&(n), 0, sizeof(rnotify_t));

	struct unrtf_t t;
	memset(&(t), 0, sizeof(struct unrtf_t));
	t.L = L;
	str_init(&t.str, BUFSIZ);

	n.udata = &t;
	n.char_cb = char_cb;
	n.style_cb = style_cb;
	//n.command_cb = command_cb;
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
