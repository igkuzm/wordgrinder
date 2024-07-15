/**
 * File              : doc.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.01.2024
 * Last Modified Date: 15.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "globals.h"
#include "libdoc/include/libdoc.h"
#include "str.h"

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
	str_init(&t->str);
}

static void par_get_style(struct undoc_t *t, PAP *p, char style[32])
{
	strcpy(style, "P");
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

static void flushstyle(struct undoc_t *t, int STY, bool val){
	lua_pushvalue(t->L, 3);
	lua_pushnumber(t->L, STY);
	lua_pushboolean(t->L, val);
	lua_call(t->L, 2, 0);
}

int main_document(void *d, ldp_t *p, int ch){
	struct undoc_t *t = d;

	char c = ch;
	
/* Following symbols below 32 are allowed inside paragraph:
0x000D - return - marks an end of paragraph
0x000B - hard return
0x0002 - footnote mark
0x0007 - table separator (converted to tabmode)
0x0009 - Horizontal tab ( printed as is)
0x000B - hard return
0x000C - page break
0x001E - IS2 for some reason means short defis in Word.
0x001F - soft hyphen in Word
0x0013 - start embedded hyperlink
0x0014 - separate hyperlink URL from text
0x0015 - end embedded hyperlink
*/

	switch (ch) {
		case 0x0D: case 0x07:
			flushparagraph(t, &p->pap);
			return 0;
		
		case 0x1E: c= '-' ; break;
		case 0x09: c= '\t'; break;
		case 0x13: c= ' ' ; break;
		case 0x15: c= ' ' ; break;
		case 0x0C: c= ch  ; break;
		case 0x1F: c= 0xAD; break;
		case 0x0B: c= 0x0A; break;
		case 0x08: 
		case 0x01: c=' '  ; break;
		
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
	str_init(&t.str);

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
