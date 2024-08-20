/**
 * File              : pdf.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.07.2022
 * Last Modified Date: 21.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "HPDF/hpdf_objects.h"
#include "HPDF/hpdf_types.h"
#include "globals.h"
#include <libgen.h>
#include <lua.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "HPDF/hpdf.h"

#define STR(...) \
({char _s[BUFSIZ]; snprintf(_s, BUFSIZ-1, __VA_ARGS__); _s[BUFSIZ-1]=0; _s;})

const char *argv0;

void
error_handler (HPDF_STATUS   error_no,
               HPDF_STATUS   detail_no,
               void         *user_data)
{
	
		char *err = STR("HPDF ERROR: error_no: 0x%04X, "
			"detail_no: %u\n", 
			(HPDF_UINT)error_no, (HPDF_UINT)detail_no);
		
		lua_getglobal(L, "pdf_error_handler");
		lua_pushstring(L, err);
		lua_call(L, 1, 0);
}

static char * _getpath(const char *exe, char buf[256]) {
	if (!exe || !exe[0] || !buf)
		return NULL;

	char selfpath[128];
	if (readlink("/proc/self/exe", selfpath, 127) < 0)
		return NULL;

	sprintf(buf, "%s/../share/wordgrinder", 
			dirname(selfpath));

	return buf;
}

int linux_get_fonts_path_cb(lua_State* L)
{
	char buf[256] = {0};
	if (!_getpath(argv0, buf))
		return -1;

	lua_pushstring(L, buf);
	lua_call(L, 1, 0);
	
	return 0;
}

int macos_get_fonts_path_cb(lua_State* L)
{
	char buf[256] = {0};
	char *e = strdup(argv0);
  sprintf(buf, "%s%s", 
			dirname(e), "/../Resources");
	free(e);

	lua_pushstring(L, buf);
	lua_call(L, 1, 0);
	
	return 0;
}


HPDF_Doc  pdf;

HPDF_Font font; 
HPDF_Font sans; 
HPDF_Font sans_bold; 
HPDF_Font sans_italic; 
HPDF_Font sans_bold_italic; 
HPDF_Font mono; 
HPDF_Font mono_bold; 
HPDF_Font mono_italic; 
HPDF_Font mono_bold_italic; 

HPDF_Page page;

HPDF_PageSizes pagesize;
HPDF_PageDirection pagedirection;

HPDF_Point p;
HPDF_Point rp;  // row point
HPDF_Point cp;  // cell point

int ncell;      // cell number
int cellw;      // cell width
int rowh;       // rowheight

float py;
float px;

float left, right, top, bottom;

float fs;

float lw; // line width
float ph; // page height

float indent;
float space;

bool firstWordInLine;

bool underline;

int pdf_new_cb(lua_State *L)
{
  pdf = HPDF_New (
			error_handler,
			L                
	);
	if (!pdf) {
		lua_getglobal(L, "pdf_error_handler");
		lua_pushstring(L, "error: cannot create PdfDoc object");
		lua_call(L, 1, 0);
		return -1;	
	}	

	/* set compression mode */
	HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

  /* add UTF-8 support */
	HPDF_UseUTFEncodings(pdf);
	HPDF_SetCurrentEncoder(pdf,"UTF-8");

	underline = false;
	ncell = 0;
	rowh  = 0;
	cellw = 0;

	return 0;
}

typedef enum {
	FONTERR,
	FONTSANS,          
	FONTSANSBOLD,      
	FONTSANSITALIC,    
	FONTSANSBOLDITALIC,
	FONTMONO,          
	FONTMONOBOLD,      
	FONTMONOITALIC,    
	FONTMONOBOLDITALIC,
} FONTTYPE;

/* arg1 - font filename 
 * arg2 - font type
 */
int pdf_load_font_cb(lua_State* L)
{
	const char* file_name = 
		luaL_checkstring(L, 1);
	if (!file_name)
		return 1;
	
	FONTTYPE type = forceinteger(L, 2);
	if (type == FONTERR)
		return -1;
	
	/* init font */
	HPDF_Font f = 
		HPDF_GetFont (pdf, 
				HPDF_LoadTTFontFromFile(
					pdf, 
					file_name, 
					HPDF_TRUE), 
				"UTF-8");

	switch (type) {
		case FONTSANS:
			sans = f;
			break;
		case FONTSANSBOLD:
			sans_bold = f;
			break;
		case FONTSANSITALIC:
			sans_italic = f;
			break;
		case FONTSANSBOLDITALIC:
			sans_bold_italic = f;
			break;
		case FONTMONO:
			mono = f;
			break;
		case FONTMONOBOLD:
			mono_bold = f;
			break;
		case FONTMONOITALIC:
			mono_italic = f;
			break;
		case FONTMONOBOLDITALIC:
			mono_bold_italic = f;
			break;

		default:
			return -1;;
	}

	return 0;
}

int pdf_set_font_cb(lua_State* L)
{
	FONTTYPE type = forceinteger(L, 1);
	if (type == FONTERR)
		return -1;
	
	switch (type) {
		case FONTSANS:
			font = sans;
			break;
		case FONTSANSBOLD:
			font = sans_bold;
			break;
		case FONTSANSITALIC:
			font = sans_italic;
			break;
		case FONTSANSBOLDITALIC:
			font = sans_bold_italic;
			break;
		case FONTMONO:
			font = mono;
			break;
		case FONTMONOBOLD:
			font = mono_bold;
			break;
		case FONTMONOITALIC:
			font = mono_italic;
			break;
		case FONTMONOBOLDITALIC:
			font = mono_bold_italic;
			break;

		default:
			return -1;;
	}

	int _fs = forceinteger(L, 2);
	if (_fs < 1)
		return -1;
	fs = _fs;
	
	HPDF_Page_SetFontAndSize(page, font, fs);
	return 0;
}

int pdf_set_underline_cb(lua_State* L)
{
	underline = lua_toboolean(L, 1);
	return 0;
}

struct page_size_t {
	char *format;
	HPDF_PageSizes size;	
};

static const struct page_size_t page_sizes[] =
{
	"A4",HPDF_PAGE_SIZE_A4,
 	"A5",HPDF_PAGE_SIZE_A5, 
	"letter", HPDF_PAGE_SIZE_LETTER,
	NULL
};

HPDF_PageSizes pdf_page_size_from_format(const char *format)
{
	struct page_size_t *p = (struct page_size_t *)page_sizes;
	while (p->size)
		if (strcmp(format, p->format) == 0)
			return p->size;	
	return HPDF_PAGE_SIZE_A4;
}

/* add page 
 * $arg1 - page number
 * $arg2 - page size
 * $arg3 - is Landscape layout
 * $arg4 - margin left
 * $arg5 - margin right
 * $arg6 - margin top
 * $arg7 - margin bottom
 */
int pdf_add_page_cb(lua_State* L)
{
	int npage = forceinteger(L, 1);
	if (!npage)
		return -1;

	const char *psz = 
		luaL_checkstring(L, 2);
	if (!psz)
		return -1;

	bool fLandscape = forceinteger(L, 3);
	
	left   = forcedouble(L, 4);
	right  = forcedouble(L, 5);
	top    = forcedouble(L, 6);
	bottom = forcedouble(L, 7);
	if (left < 0 || right < 0 || top < 0 || bottom < 0)
		return -1;

	page = HPDF_AddPage(pdf);
	pagesize = pdf_page_size_from_format(psz);
	pagedirection =	fLandscape?HPDF_PAGE_LANDSCAPE:HPDF_PAGE_PORTRAIT;

	HPDF_Page_SetSize(
			page,
			pagesize,	
			pagedirection);
	
	py = HPDF_Page_GetHeight(page) - top * 72 / 2.5;
	px = left * 72 / 2.5;

	HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);
	HPDF_Page_SetTextLeading(page, 20);

	p.x = px;
	p.y = py;
	
	ph = HPDF_Page_GetHeight(page) 
				- top * 72 / 2.5
				- bottom * 72 / 2.5;
		
	return 0;
}

int pdf_write_text_cb(lua_State* L)
{
	const char *text = 
		luaL_checkstring(L, 1);
	if (!text)
		return -1;

	if (firstWordInLine) {
		firstWordInLine = false;
		if (text[0] == ' ')
			return 0;
	}
	
	HPDF_Page_BeginText(page);
	
	HPDF_REAL w = 
		HPDF_Page_TextWidth(page, text);

	HPDF_Page_TextOut(page, p.x, p.y, text);
  
	HPDF_Page_EndText(page);

	if (underline){ // draw line
		HPDF_Page_SetLineWidth(page, 0);
		HPDF_Page_MoveTo(page, p.x, p.y - 1);
		HPDF_Page_LineTo(page, p.x + w, p.y - 1);	
		HPDF_Page_Stroke(page);
	}

	p.x += w; 
	
	return 0;
}

int pdf_start_paragraph_cb(lua_State* L)
{
	return 0;
}

int pdf_end_paragraph_cb(lua_State* L)
{
	return 0;
}

int pdf_start_line_cb(lua_State* L)
{
	indent = 0;
	space  = 0;
	firstWordInLine = true;	

	lw = HPDF_Page_GetWidth(page) 
			- left  * 72 / 2.5
			- right * 72 / 2.5;

	HPDF_Page_SetLineWidth(page, lw); 
	HPDF_Page_SetWordSpace(page, 0);
	
	return 0;
}

int pdf_end_line_cb(lua_State* L)
{
	p.x = px;
	p.y -= 20;
	return 0;
}

int pdf_justify_right_cb(lua_State* L)
{
	const char* text = 
		luaL_checkstring(L, 1);
	if (!text || text[0] == 0) // this is empthy string
		return 0;

	// count text width
	float w = HPDF_Page_TextWidth(page, text);

	p.x = lw - w + left * 72 / 2.5;
	return 0;
}

int pdf_justify_center_cb(lua_State* L)
{
	const char* text = 
		luaL_checkstring(L, 1);
	if (!text || text[0] == 0) // this is empthy string
		return 0;

	// count text width
	float w = HPDF_Page_TextWidth(page, text);
		
	p.x = lw / 2 - w / 2 + left * 72 / 2.5;
	return 0;
}

int pdf_justify_both_cb(lua_State* L)
{
	const char* text = 
		luaL_checkstring(L, 1);
	if (!text || text[0] == 0) // this is empthy string
		return 0;

	// count text width
	float w = HPDF_Page_TextWidth(page, text);

	// count spaces
	int ns = 0, i;
	for (i = 0; i < strlen(text); ++i)
		if (text[i] == ' ' && i != 0)
			ns++;

	//if (ns && w < lw - indent)
	if (ns)
		space = (lw - w - indent)/ns;
	if (space < 0)
		space = 0;
	
	HPDF_Page_SetWordSpace(page, space);
		
	return 0;
}

int pdf_make_indent_cb(lua_State* L)
{
	indent = forcedouble(L, 1) * 12;
	p.x += indent;

	return 0;
}

int pdf_close_cb(lua_State *L)
{
	int ret = 0;

	const char* file_name = 
		luaL_checkstring(L, 1);
	
	if (file_name){
		/* save the document to a file */
		HPDF_SaveToFile (pdf, file_name);
	} else {
		char *err = 
			STR("can't save pdf file to path: %s",
					file_name);
		lua_getglobal(L, "pdf_error_handler");
		lua_pushstring(L, err);
		lua_call(L, 1, 0);
		ret = -1;
	}

  /* clean up */
  HPDF_Free (pdf);	
	return ret;
}

double left_cell_border; 

int pdf_draw_left_cell_border_cb(lua_State* L)
{
	int w = forceinteger(L, 1);
	
	HPDF_Page_SetLineWidth(page, 0);
	HPDF_Page_MoveTo(page, left_cell_border, rp.y + 20);
	HPDF_Page_LineTo(page, left_cell_border, rp.y - rowh * 20 + 20 - 2);	
	HPDF_Page_Stroke(page);		

	left_cell_border += w;

	return 0;
}


int pdf_set_inrow_cb(lua_State* L)
{
	left_cell_border = px;
	int h = forceinteger(L, 1);
	bool borders = lua_toboolean(L, 2);
	if (h){
		rp.x = px;
		rp.y = p.y;
		rowh = h;
		// draw borders
		if (borders){
			//top
			HPDF_Page_SetLineWidth(page, 0);
			HPDF_Page_MoveTo(page, p.x, p.y);
			HPDF_Page_LineTo(page, lw + left * 72/2.5, p.y);	
			HPDF_Page_Stroke(page);

			//right
			HPDF_Page_SetLineWidth(page, 0);
			HPDF_Page_MoveTo(page, lw+left*72/2.5, p.y);
			HPDF_Page_LineTo(page, lw+left*72/2.5, p.y - rowh * 20 - 2);	
			HPDF_Page_Stroke(page);
			
			rp.y = p.y -= 20;
			p.x = rp.x += 2;
		}
	} else {
		p.x = rp.x = px;
		p.y = rp.y - 20 * rowh + 20 - 2;
		
		rowh = 0;
	}

	return 0;
}

int pdf_end_table_cb(lua_State* L)
{
	bool borders = lua_toboolean(L, 1);
	if (borders){
		// draw bottom
		HPDF_Page_SetLineWidth(page, 0);
		HPDF_Page_MoveTo(page, p.x, p.y);
		HPDF_Page_LineTo(page, lw+left*72/2.5, p.y);	
		HPDF_Page_Stroke(page);
		p.y -= 20;
	}

	return 0;
}

int pdf_set_incell_cb(lua_State* L)
{
	int w = forceinteger(L, 1);
	bool borders = lua_toboolean(L, 2);
	if (w) {
		cellw = w;
		
	} else {
		rp.x += cellw;
		
		p.x = rp.x;
		cellw = 0;
	}
			
	return 0;
}

int pdf_stop_table_cell_line_cb(lua_State* L)
{
	p.x = rp.x;
	p.y -= 20;

	return 0;
}

int pdf_image_cb(lua_State* L)
{
	const char* text = 
		luaL_checkstring(L, 1);
	if (!text || text[0] == 0) // this is empthy string
		return -1;

	HPDF_Image image = 
		HPDF_LoadJpegImageFromFile(pdf, text);

	if (!image)
		return -1;

	HPDF_Point sz = HPDF_Image_GetSize(image);
	float w, h;
	
	if (sz.y > sz.x)
	{
		w = lw;
		h = sz.y/sz.x * w;
	} else {
		h = ph;
		w = sz.x/sz.y * h;
	}

	if (h > p.y - ph){
		// new page
		page = HPDF_AddPage(pdf);
	
		HPDF_Page_SetSize(
			page,
			pagesize,	
			pagedirection);

		HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);
		HPDF_Page_SetTextLeading(page, 20);

		p.x = px;
		p.y = py;
	}

	p.y -= h;

	HPDF_Page_DrawImage(page, image, p.x, p.y, w, h);
	

	return 0;
}


void pdf_init(const char *_argv0)
{
	argv0 = _argv0;
	const static luaL_Reg funcs[] =
	{
		{ "pdf_new",             pdf_new_cb },
		{ "pdf_close",           pdf_close_cb },
		{ "pdf_add_page",        pdf_add_page_cb },
		{ "pdf_write_text",      pdf_write_text_cb },
		{ "pdf_load_font",       pdf_load_font_cb },
		{ "pdf_set_font",         pdf_set_font_cb },
		{ "pdf_image",         pdf_image_cb },
		{ "pdf_start_paragraph", pdf_start_paragraph_cb },
		{ "pdf_end_paragraph",   pdf_end_paragraph_cb },
		{ "pdf_start_line",      pdf_start_line_cb },
		{ "pdf_end_line",        pdf_end_line_cb },
		{ "pdf_set_underline",   pdf_set_underline_cb },
		{ "pdf_set_inrow",			 pdf_set_inrow_cb },
		{ "pdf_set_incell",      pdf_set_incell_cb },
		{ "pdf_end_table",        pdf_end_table_cb },
		{ "pdf_draw_left_cell_border",        pdf_draw_left_cell_border_cb},
		{ "pdf_stop_table_cell_line",      pdf_stop_table_cell_line_cb },
		{ "pdf_justify_right",   pdf_justify_right_cb },
		{ "pdf_justify_center",  pdf_justify_center_cb },
		{ "pdf_justify_both",  pdf_justify_both_cb },
		{ "pdf_make_indent",  pdf_make_indent_cb },
		{ "linux_get_fonts_path",  linux_get_fonts_path_cb },
		{ "macos_get_fonts_path",  macos_get_fonts_path_cb },
		{ NULL,            NULL }
	};

	lua_getglobal(L, "wg");
	luaL_setfuncs(L, funcs, 0);
	
	lua_pushnumber(L, FONTSANS);
	lua_setfield(L, -2, "FONTSANS");
	lua_pushnumber(L, FONTSANSBOLD);
	lua_setfield(L, -2, "FONTSANSBOLD");
	lua_pushnumber(L, FONTSANSITALIC);
	lua_setfield(L, -2, "FONTSANSITALIC");
	lua_pushnumber(L, FONTSANSBOLDITALIC);
	lua_setfield(L, -2, "FONTSANSBOLDITALIC");
	lua_pushnumber(L, FONTMONO);
	lua_setfield(L, -2, "FONTMONO");
	lua_pushnumber(L, FONTMONOBOLD);
	lua_setfield(L, -2, "FONTMONOBOLD");
	lua_pushnumber(L, FONTMONOITALIC);
	lua_setfield(L, -2, "FONTMONOITALIC");
	lua_pushnumber(L, FONTMONOBOLDITALIC);
	lua_setfield(L, -2, "FONTMONOBOLDITALIC");
}
