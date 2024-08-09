/**
 * File              : pdf.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 20.07.2022
 * Last Modified Date: 09.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "HPDF/hpdf_types.h"
#include "globals.h"
#include <lua.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HPDF/hpdf.h"

#define STR(...) \
({char _s[BUFSIZ]; snprintf(_s, BUFSIZ-1, __VA_ARGS__); _s[BUFSIZ-1]=0; _s;})

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

//int generate_pdf_cb(lua_State* L)
//{

//#pragma mark - Generate PDF file
    
	//HPDF_Doc  pdf;

  //pdf = HPDF_New (
			//error_handler,			  //error handler 
			//NULL //error_handler_data
	//);
	//if (!pdf) {
		////"error: cannot create PdfDoc object\n");
		//return -1;	
	//}	

	//if (setjmp(env)) {  //jump here after error
			//HPDF_Free (pdf);
			//return -1;
	//}	

	//[> set compression mode <]
	//HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);

    //[> add UTF-8 support <]
	//HPDF_UseUTFEncodings(pdf);
	//HPDF_SetCurrentEncoder(pdf,"UTF-8");

	////make your MAGIC
	
	//int mP = 30;    //x position of main text
	//int dP = 350;   //y position of detail text	
	
  //int ypos; //y position

	//int yS = 40; //line space

  //[> add a new page object. <]
  //HPDF_Page page1 = HPDF_AddPage (pdf);
	//ypos = HPDF_Page_GetHeight (page1) - 50;
	//ypos -= yS;

	//{
		//HPDF_Font font = HPDF_GetFont (pdf, HPDF_LoadTTFontFromFile (pdf, "RobotoCondensed-Light.ttf", HPDF_TRUE), "UTF-8");
		//HPDF_Page_SetFontAndSize (page1, font, 10);
		//char * text = "Число, месяц, год "; 
		//float tw = HPDF_Page_TextWidth(page1, text);
		//draw_text(page1, mP, ypos, pdf, text);
		//ypos -= yS;
	//}

	//HPDF_Image image0;
	//if (item->image0_len > 0)
		//image0 = HPDF_LoadJpegImageFromMem(pdf, item->image0, item->image0_len);
	//else
		//image0 = HPDF_LoadJpegImageFromFile(pdf, "model.jpg");

	//HPDF_Page_DrawImage(page3, image0, 100, ypos, 400, 600);
	

 /* save the document to a file */
    //HPDF_SaveToFile (pdf, data->filepath);

    /* clean up */
    //HPDF_Free (pdf);	

	
	//return 0;
//}	

HPDF_Doc  pdf;
HPDF_Font font; 
HPDF_Page page;

float py;
float px;

float fs;

void
draw_text  (float        x,
            float        y,
						char				*text)
{
	// count text width
	HPDF_UINT b = 
		HPDF_Page_MeasureText(page, text, 
				HPDF_Page_GetLineWidth(page), HPDF_TRUE, NULL);

  HPDF_Page_BeginText(page);
	HPDF_Page_SetWordSpace(page, 100);
	HPDF_Page_TextOut(page, x, y, text);
  HPDF_Page_EndText(page);

	// move to next line
	py -= fs;
}

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

	return 0;
}

int pdf_load_font_cb(lua_State* L)
{
	const char* file_name = 
		luaL_checkstring(L, 1);
	if (!file_name)
		return 1;
	
	int _fs = forceinteger(L, 2);
	if (_fs < 1)
		return -1;
	fs = _fs;
	
	/* init font */
	font = 
		HPDF_GetFont (pdf, 
				HPDF_LoadTTFontFromFile(
					pdf, 
					file_name, 
					HPDF_TRUE), 
				"UTF-8");

	HPDF_Page_SetFontAndSize(page, font, fs);
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

	const char *pagesize = 
		luaL_checkstring(L, 2);
	if (!pagesize)
		return -1;

	bool fLandscape = forceinteger(L, 3);
	
	double left   = forcedouble(L, 4);
	double rigth  = forcedouble(L, 5);
	double top    = forcedouble(L, 6);
	double bottom = forcedouble(L, 7);
	if (left < 0 || rigth < 0 || top < 0 || bottom < 0)
		return -1;

	page = HPDF_AddPage(pdf);
	HPDF_Page_SetSize(
			page,
			pdf_page_size_from_format(pagesize),	
			fLandscape?HPDF_PAGE_LANDSCAPE:HPDF_PAGE_PORTRAIT);
	
	py = HPDF_Page_GetHeight(page) - top * 72 / 2.5;
	px = left * 72 / 2.5;

	HPDF_Page_SetLineWidth(
			page, 
			HPDF_Page_GetWidth(page) 
			- left  * 72 / 2.5
			- rigth * 72 / 2.5);

	HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);
		
	return 0;
}

int pdf_write_text_cb(lua_State* L)
{
	const char *text = 
		luaL_checkstring(L, 1);
	if (!text)
		return -1;
	
	draw_text(
			px, py, 
			(char *)text);
	
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

void pdf_init(void)
{
	const static luaL_Reg funcs[] =
	{
		{ "pdf_new",        pdf_new_cb },
		{ "pdf_close",      pdf_close_cb },
		{ "pdf_add_page",   pdf_add_page_cb },
		{ "pdf_write_text", pdf_write_text_cb },
		{ "pdf_load_font", pdf_load_font_cb },
		{ NULL,            NULL }
	};

	lua_getglobal(L, "wg");
	luaL_setfuncs(L, funcs, 0);
}
