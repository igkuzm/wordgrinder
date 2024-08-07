/**
 * File              : mswordtype.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 17.01.2024
 * Last Modified Date: 07.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef MSWORDTYPE_H
#define MSWORDTYPE_H
#include <stdint.h>
#include <stdio.h>
#define fTrue 1
#define fFalse 0

/* fonts */
typedef enum {
	fnil,              // Unknown or default fonts (the 
										 // default) 
	froman,            // Roman, proportionally spaced serif 
										 // fonts
	fswiss,
	fmodern,
	fscript,
	fdecor,
	ftech,
	fbidi,             // Arabic, Hebrew, or other 
										 // bidirectional font
} FFAM;              // font family
										 
// Specifies the pitch of a font in the font table. 
typedef enum {
	fDpich,    // Default pitch
	fFpich,    // Fixed pitch
	fVpich     // Variable pitch 
} FPCH;

typedef struct font {
	int  num;          // font number
	char name[64];     // font name
	int  lname;        // len of name
	FFAM ffam;         // font family
	int  charset;      //
	int	 falt[64];     // alternative font 
	int  lfalt;        // len of falt
	FPCH fprq;         // Specifies the pitch of a font in the font table. 
	char ftype;        // font type (nil/truetype)
	int  cpg;          // codepage
} FONT;

/* colors */
typedef struct color {
	char red;          // 0-255
	char green;        // 0-255
	char blue;         // 0-255
} COLOR;

/* Character Properties */
typedef struct char_prop
{
	char fBold;
	char fUnderline;
	char fItalic;
	int  font;
	int  font1;
	int  font2;
	int  size;
	int  kern; // kerning
	int  fcolor;
	int  bcolor;
	int  charset;      //
	int  charsetEastAsian;      //
	char allCaps;    // all capital symbols
	
	// MS DOC special 
	int  cp; // character portion
	int  sprmCFSpec; // sprcial properties
	char sprmCFObj;
	char sprmCFOle2;
	char sprmCFData;
	int32_t sprmCPicLocation;
} CHP;            // CHaracter Properties

/* Paragraph justification */
typedef enum {
	justL,             // left
	justR,             // rigth
	justC,             // center
	justF              // full
} JUST;

/* Paragraph properties */
typedef struct para_prop
{
	char fIntbl;			 // if in table paragraph
	int  Itap;				 // table depth of paragraph 
  int  xaLeft;        // left indent in twips
  int  xaRight;       // right indent in twips
	int  xaFirst;       // first line indent in twips
	JUST just;         // justification
	int  s;             // paragraph style
	int  before;        // spacing before paragraph
	int  after;				 // spacing after paragraph
	// special marks
	char TTP;  // table terminating paragraph mark 
	char ITTP; // inner table terminating paragraph mark
	char ITC;  // inner table cell mark
}  PAP;               // PAragraph Properties

/* Section break type */
typedef enum {
	sbkNon,            // No section break
	sbkCol,            // Section starts a new column  
	sbkEvn,            // Section starts at an even page  
	sbkOdd,            // Section starts at an odd page 
	sbkPg              // Section starts a new page (by Default)
} SBK;

/* Page number formating */
typedef enum {
	pgDec,             // Page-number format is decimal
	pgURom,            // Page-number format is uppercase roman numeral 
	pgLRom,            // Page-number format is lowercase roman numeral 
	pgULtr,            // Page-number format is uppercase letter 
	pgLLtr             // Page-number format is lowerrcase letter 
} PGN;

/* Section properties */
typedef struct sect_prop
{
	int cCols;        // number of columns
	SBK sbk;          // section break type
	int xaPgn;        // x position of page number in twips
	int yaPgn;        // y position of page number in twips
	PGN pgnFormat;    // how the page number is formatted
	int ds;           // section style
	int   xaPage;     // page width in twips
	int   yaPage;     // page height in twips
	int   xaLeft;     // left margin in twips
	int   yaTop;      // top margin in twips
	int   xaRight;    // right margin in twips
	int   yaBottom;   // bottom margin in twips

} SEP;              // SEction Properties

/* table cell pattern */
typedef enum tbc_pattern {
	patH,             // horizontal background pattern for the cell
	patV,							// vertical background pattern for the cells
	patFD,						// forward diagonal background pattern for the cell (\\\\) 
	patBD,						// backward diagonal background pattern for the cell (////)
	patC,							// cross background pattern
	patCD,						// diagonal cross background pattern
	patDH,						// dark horizontal background
	patDV,						// dark vertical background pattern
	patDFD,           // dark forward diagonal background pattern
	patDBD,           // dark backward diagonal background pattern
	patDC,						// dark cross background pattern
	patDCD,						// dark diagonal cross background pattern
} TPA;

/* table cell alignment */
typedef	enum tbc_alignment {
	aligmT,           // Text is top-aligned in cell (the default) 
	aligmC,           // Text is centered vertically in cell
	aligmB,           // Text is bottom-aligned in cell
	aligmVL,          // Vertical text aligned left (direction bottom up)
	aligmVR,          // Vertical text aligned right (direction top down)
} TCA;

/* table row properties */
typedef struct tbr_prop {
	JUST just;
	int  trgaph[64];  // space bitwin cells
	int  ntrgaph;     // number of items in trgaph
	int  cellx[64];   // size of cell (right boundary)
	int  ncellx;      // number of items in cellx
	int  trrh;        // table row height
	int  trleft;      // Position of the leftmost edge of the 
										// table with respect to the left
										// edge of its column
	char bordT;       // border top
	char bordB;       // border bottom
	char bordL;       // border left
	char bordR;       // border right
	char bordH;       // border Horizontal
	char bordV;       // border Vertical
	
	char cbordT[64];  // cells border top 
	char cbordB[64];  // cells border bottom
	char cbordL[64];  // cells border left
	char cbordR[64];  // cells border right
	
	char header;      // this row is header
	char keep;        // keep this row from pagebreak
	char direction;   // 0 - left-to right, 1 - right to left
} TRP;

/* table cell properties */
typedef struct tbc_prop {
	TCA  alignment;
	TPA  pattern;
	int  shading;     // shading of a table cell in hundredths of a percent
	int  line_color;
	int  back_color;
	char bordT;       // border top
	char bordB;       // border bottom
	char bordL;       // border left
	char bordR;       // border right
	char clmgf;       // The first cell in a range of table cells to be merged
	char clmrg;       // Contents of the table cell are merged with those of the 
										// preceding cell
} TCP;

/* charset */
typedef enum {
	charset_ansi,
	charset_mac,
	charset_pc,
	charset_pca
} CHSET;

typedef struct msword_date {
	int   year;
	int   month;
	int   day;
	int   hour;
	int   min;
	int   sec;
} DATE;

// footnotes/endnotes type
typedef enum {
	fetT,  // 0 Footnotes only or nothing at all (the default)
  fetE,  // 1 Endnotes only.                                
  fetTE  // 2 Footnotes and endnotes both.                  	
}FET;

// footnotes/endnotes position
typedef enum {
  fepEOS,   // at the end of the section (the default)
  fepEOD,   // at the end of the document
	fepT,			// beneath text (top justified)
	fepB,			// at the bottom of the page (bottom justified)  
} FEP;

// footnotes/endnotes numbering
typedef enum {
	fenN,     // Arabic numbering (1, 2, 3, ...)
	fenAl,		// Alphabetic lowercase (a, b, c, ...) 
	fenAu,		// Alphabetic uppercase (A, B, C, ...) 
	fenRl,		// Roman lowercase (i, ii, iii, ...)  
	fenRu,		// Roman uppercase (I, II, III, ...)
	fenChi,		// Chicago Manual of Style (*, [dagger], [daggerdbl], )  
} FEN;

/* Document properties */
typedef struct doc_prop
{
	int   xaPage;     // page width in twips
	int   yaPage;     // page height in twips
	int   xaLeft;     // left margin in twips
	int   yaTop;      // top margin in twips
	int   xaRight;    // right margin in twips
	int   yaBottom;   // bottom margin in twips
	int   pgnStart;   // starting page number in twips
	char  fFacingp;   // facing pages enabled?
	char  fLandscape; // landscape or portrait??
	int   deflang;    // Default language
	int   deff;       // Default font
	int   deftab;     // Default tab width
	int   version;    // document version
	int   vern;       // internal version
	int   emin;       // Total editing time (in minutes) 
	int   npages;     // Number of pages 
	int   nwords;     // Number of words
	int   nchars;     // Number of characters including spaces
	int   ncharsws;   // Number of characters not including spaces
	int   id;         // Internal ID number 
	CHSET chset;      // charset
	FET   fet;		    // Footnote/endnote type 
	FEP   fp;         // footnotes position
	FEP   ep;         // endnotes position
	int   fb;         // Beginning footnote number (the default is 1)
	int   eb;			  	// Beginning endnote number (the default is 1) 
	int   fr;         // Continuous footnote numbering - 0, restart each page - 1
	int   er;         // Continuous endnote numbering - 0, restart each page - 1
	FEN   fn;			  	// Footnote numbering 
	FEN   en;			  	// Endnote numbering
	int		gutter;			// Gutter width in twips (the default is 0) 
	char 	fMirror; 		// Switches margin definitions on left and right pages 
} DOP;							// DOcument Properties

typedef	struct style {
	int s;           // paragraph style
	int ds;          // section style
	int sbedeon;     // based on style
	int next;        // next style
	char hidden;
	char name[64];     // style name
	int  lname;        // len of name
	CHP chp;
	PAP pap;
	CHP pap_chp;
	SEP sep;
} STYLE;

// picture type
typedef	enum {
	pict_unknown,
	pict_emf,     // Source of the picture is an EMF (enhanced metafile)
	pict_png,     // PNG
	pict_jpg,     // JPEG
	pict_mac,     // Source of the picture is QuickDraw
								// (PICT)
	pict_wmf,     // Source of the picture is a Windows metafile 
	pict_omf,     // Source of the picture is an OS/2 metafile
	pict_ibitmap, // Source of the picture is a Windows device-independent bitmap (DIB) 
	pict_dbitmap, // Source of the picture is a Windows device-dependent bitmap 
	pict_tiff,     // TIFF
} PICT_T;

typedef struct picture {
	unsigned char   *data; // binary data
	int              len;  // length of data
	
	PICT_T type;
	int    type_n;
	long   w;      // xExt field if the picture is a Windows metafile; picture width in pixels if
								 // the picture is a bitmap or from QuickDraw
	long   h;      // yExt field if the picture is a Windows metafile; picture height in pixels if
								 // the picture is a bitmap or from QuickDraw
	long   goalw;  // Desired width of the picture in twips
	long   goalh;  // Desired height of the picture in twips
	int    scalex; // Horizontal scaling value. The N argument is a value representing a percentage
								 // (the default is 100)
	int    scaley; // Vertical scaling value. The N argument is a value representing a percentage
								 // (the default is 100)
	char   scaled; // Scales the picture to fit within the specified frame. Used only with \macpict
								 // pictures
} PICT;

#endif
