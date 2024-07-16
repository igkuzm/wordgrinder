/**
 * File              : doc.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.01.2024
 * Last Modified Date: 16.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "globals.h"
#include "libdoc/include/libdoc.h"
#include "libdoc/include/libdoc/str.h"
#include <stdio.h>

static int main_document(void *, ldp_t*, int);
static int footnotes(void *, ldp_t*, int);
static int headers(void *, ldp_t*, int);

struct undoc_t {
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
};

int footnotes(void *d, ldp_t *p, int ch){

	return 0;
}
int headers(void *d, ldp_t *p, int ch){

	return 0;
}

static void flushstring(struct undoc_t *t)
{
	lua_pushvalue(t->L, 6);
	lua_pushlstring(t->L, t->str.str, t->str.len);
	lua_call(t->L, 1, 0);
	free(t->str.str);
	str_init(&t->str, BUFSIZ);
}

static void par_get_style(struct undoc_t *t, PAP *p, char style[32])
{
	strcpy(style, "P");

	if (p->just == justC)
		strcpy(style, "CENTER");

	if (p->just == justR)
		strcpy(style, "RIGHT");
	
	if (p->just == justL)
		strcpy(style, "LEFT");

}

static void flushparagraph(struct undoc_t *t, PAP *p)
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

static void flushrow(struct undoc_t *t, TRP *rp){
	fprintf(stderr, "FLUSHROW\n");
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

static void flushcell(struct undoc_t *t, TCP *tcp, TRP *trp){
	fprintf(stderr, "FLUSHCELL\n");
	if (
			tcp->bordB ||
			tcp->bordL ||
			tcp->bordR ||
			tcp->bordT
		 )
		t->bordered = true;

	if (trp->ncellx)
		t->ncells = trp->ncellx;

	int len = 0;
	if (t->cell < trp->ncellx)
		len = trp->cellx[t->cell];

	t->cell++;

	flushstring(t);

	lua_pushvalue(t->L, 5);
	lua_pushnumber(t->L, t->ncells);
	lua_pushnumber(t->L, len);
	lua_call(t->L, 2, 0);
}


static void flushstyle(struct undoc_t *t, int STY, bool val){
	lua_pushvalue(t->L, 3);
	lua_pushnumber(t->L, STY);
	lua_pushboolean(t->L, val);
	lua_call(t->L, 2, 0);
}

int main_document(void *d, ldp_t *p, int ch){
	struct undoc_t *t = d;

	char c = ch;
	
	switch (ch) {
		case LINEBREAK: 
			flushparagraph(t, &p->pap);
			return 0;
		case PARAGRAPH_MARK:
			if (p->pap.ITTP)
				flushrow(t, &p->trp);
			else if (p->pap.ITC)
				flushcell(t, &p->tcp, &p->trp);
			else
				flushparagraph(t, &p->pap);
			return 0;
		case CELL_MARK:
			if (p->pap.TTP)
				flushrow(t, &p->trp);
			else
				flushcell(t, &p->tcp, &p->trp);
			return 0;
		
		case HORIZONTALTAB:      c= '\t' ; break;
		case HYPERLINK_START:    c= ' '  ; break;
		case HYPERLINK_SEPARATE: c= ' '  ; break;
		case HYPERLINK_END:      c= ' '  ; break;
		case PAGEBREAK:          c= ch   ; break;
		case SOFT_HYPEN:
		case HYPHEN:             c= '-'  ; break;
		case INLINE_PICTURE:
		case FLOATING_PICTURE:   c= ' '  ; break;
		
		default: break;	
	}

	if (ch > 256)
		return 0;

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
	str_appendf(&t->str, "%c", c);
	return 0;
}

static int undoc_cb(lua_State* L)
{
	size_t size;
	const char* filename = luaL_checklstring(L, 1, &size);

	struct undoc_t t;
	memset(&(t), 0, sizeof(struct undoc_t));
	t.L = L;
	str_init(&t.str, BUFSIZ);

	int ret = doc_parse(
			filename, 
			&t, 
			main_document,
			footnotes,
			headers);

	return ret;
}

void undoc_init(void)
{
	const static luaL_Reg funcs[] =
	{
		{ "undoc",                undoc_cb },
		{ NULL,                   NULL }
	};

	lua_getglobal(L, "wg");
	luaL_setfuncs(L, funcs, 0);
}