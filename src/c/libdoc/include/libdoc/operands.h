/**
 * File              : operands.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 28.05.2024
 * Last Modified Date: 07.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef OPERANDS_H
#define OPERANDS_H

#include "doc.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* The ToggleOperand structure is an operand to an SPRM
 * whose spra is 0 and whose sgc is 2. It
 * modifies a Boolean character property. */
static bool ToggleOperand(cfb_doc_t *doc, BYTE operand)
{
#ifdef DEBUG
	LOG("operand: 0x%02x", operand); 
#endif
/* value (1 byte): An unsigned integer which MUST be one of
 * the following values.
 * Value Meaning
 * 0x00 The Boolean property is set to 0, which means the
 * property is turned OFF.
 * 0x01 The Boolean property is set to 1, which means the
 * property is turned ON.
 * 0x80 The Boolean property is set to match the value of
 * the property in the current style that is applied to
 * the text.
 * 0x81 The Boolean property is set to the opposite of the
 * value of the property in the current style that is
 * applied to the text. */
	switch (operand) {
		case 0x00: return false;
		case 0x01: return true;
		case 0x80: 
			{
				/* TODO: current style check */
				return false;	
			}
		case 0x81: 
			{
				/* TODO: current style check */
				return true;	
			}
		default:
			ERR("wrong ToggleOperand: 0x%02x", operand);
			break;	
	}
	return false;
}

/* 2.9.119 Ico
 * The Ico structure specifies an entry in the color palette
 * that is listed in the following table. */
static const struct {BYTE b; COLOR c;} IcoValue[] =
{
	{0x00, {0x00, 0x00, 0x00}},
	{0x01, {0x00, 0x00, 0x00}},
	{0x02, {0x00, 0x00, 0xFF}},
	{0x03, {0x00, 0xFF, 0xFF}},
	{0x04, {0x00, 0xFF, 0x00}},
	{0x05, {0xFF, 0x00, 0xFF}},
	{0x06, {0xFF, 0x00, 0x00}},
	{0x07, {0xFF, 0xFF, 0x00}},
	{0x08, {0xFF, 0xFF, 0xFF}},
	{0x09, {0x00, 0x00, 0x80}},
	{0x0A, {0x00, 0x80, 0x80}},
	{0x0B, {0x00, 0x80, 0x00}},
	{0x0C, {0x80, 0x00, 0x80}},
	{0x0D, {0x80, 0x00, 0x80}},
	{0x0E, {0x80, 0x80, 0x00}},
	{0x0F, {0x80, 0x80, 0x80}},
	{0x10, {0xC0, 0xC0, 0xC0}},
};

static const COLOR *Ico(BYTE operand)
{
#ifdef DEBUG
	LOG("operand: 0x%02x", operand); 
#endif
	int i;
	for (i = 0; i < 17; ++i) {
		if (operand == IcoValue[i].b)
			return &(IcoValue[i].c);
	}
	ERR("no color with Ico: 0x%02x", operand);
	return NULL;
}

/* 2.9.123 ItcFirstLim
 * The ItcFirstLim structure specifies a range of cells in a
 * table row. The range is inclusive of the first
 * index, and exclusive of the second. The first cell in a
 * row is at index 0. The maximum number of cells
 * in a row is 63.*/
struct ItcFirstLim {
	BYTE itcFirst; //itcFirst (8 bits): An integer value that
								 //specifies the index of the first cell in
								 //a contiguous range. The
								 //cell at this index is inside the range.
								 //This value MUST be non-negative and MUST
								 //be less than the
								 //number of cells in the row.
	BYTE itcLim;   //itcLim (8 bits): An integer value that
								 //specifies the index of the first cell
								 //beyond the contiguous
								 //range. The cell at this index is outside
								 //the range. This value MUST be greater
								 //than or equal to
								 //itcFirst and MUST be less than or equal
								 //to the number of cells in the row. When
								 //itcLim is equal
								 //to itcFirst, the range contains zero
								 //cells.
};

/* 2.9.45 CSSA
 * The CSSA structure specifies a cell spacing SPRM argument
 * used by many Table SPRMs to define table
 * cell margins and cell spacing.*/
struct CSSA {
	struct ItcFirstLim itc; 
							//itc (2 bytes): An ItcFirstLim that specifies
							//which cells this CSSA structure applies to.
	BYTE grfbrc;//grfbrc (1 byte): A bit field that specifies
							//which cell sides this cell margin or cell
							//spacing applies to.
							//The bit values and their meanings are as
							//follows.
							//Name Bit Mask Meaning
							//fbrcTop 0x01 Specifies the top side.
							//fbrcLeft 0x02 Specifies the left side.
							//fbrcBottom 0x04 Specifies the bottom side.
							//fbrcRight 0x08 Specifies the right side.
							//Setting all four side bits results in
							//fBrcSidesOnly (0x0F). All other bits MUST be
							//0.
	BYTE ftsWidth;//ftsWidth (1 byte): An Fts that specifies
								//how wWidth is defined.
	USHORT wWidth;//wWidth (2 bytes): An unsigned integer
								//value that specifies the cell margin or
								//cell spacing that is
								//applied to cells itc.itcFirst through
								//itc.itcLim – 1. The interpretation of this
								//value depends on the
								//value of ftsWidth. If ftsWidth is ftsNil
								//(0x00), then wWidth MUST be zero.
};

/* 2.9.46 CSSAOperand
 * The CSSAOperand structure is an operand that is used by
 * several Table SPRMs to specify a table cell
 * margin or cell spacing.*/
struct CSSAOperand {
	BYTE cb;   //cb (1 byte): An unsigned integer value that
						 //specifies the size of this operand in bytes,
						 //not including
						 //cb. The cb MUST be 6.
	struct CSSA cssa; //cssa (6 bytes): A CSSA that specifies the
						 //cell margin or cell spacing to apply.
};

/* 2.9.302 TableBordersOperand
 * The TableBordersOperand structure specifies a set of
 * borders for a table row. */ 
struct TableBordersOperand {
	BYTE cb;   //cb (1 byte): An unsigned integer that
						 //specifies the size, in bytes, of this
						 //TableBordersOperand
						 //structure, not including this byte. This
						 //value MUST be 0x30
	struct Brc brcTop;
						 //(8 bytes): A Brc structure that specifies the
						 //top border of the row, if it is the first row
						 //in the table.
  struct Brc brcLeft;
						 //(8 bytes): A Brc structure that specifies the
						 //logical left border of the row.
  struct Brc brcBottom; 
						 //(8 bytes): A Brc structure that specifies the
						 //bottom border of the row, if it is the last
						 //row in the table.
  struct Brc brcRight;
						 //(8 bytes): A Brc structure that specifies the
						 //logical right border of the row.
  struct Brc brcHorizontalInside;
						 //(8 bytes): A Brc structure that specifies 
						 //the horizontal border between the row and 
						 //the preceding and succeeding rows 
	struct Brc brcVerticalInside;
						 //(8 bytes): A Brc structure that specifies the
						 //vertical border between the cells in the row.
};

/* 2.9.303 TableBordersOperand80
 * The TableBordersOperand80 structure is an operand that
 * specifies the borders which are applied to
 * a row of table cells.*/
struct TableBordersOperand80 {
	BYTE cb;          //(1 byte): An unsigned integer that
										//specifies the size of this operand,
										//not including this byte. This value
										//MUST be 0x18.
  Brc80MayBeNil brcTop; 
										//(4 bytes): A Brc80MayBeNil structure
										//that specifies the top border of the
										//row, if it is the first row in the
										//table.
  Brc80MayBeNil brcLeft; 
										//(4 bytes): A Brc80MayBeNil structure
										//that specifies the logical left border
										//of the row.
  Brc80MayBeNil brcBottom; 
										//(4 bytes): A Brc80MayBeNil structure
										//that specifies the bottom border of
										//the row, if it is the last row in the
										//table.
  Brc80MayBeNil brcRight; 
										//(4 bytes): A Brc80MayBeNil structure
										//that specifies the logical right
										//border of the row.
	Brc80MayBeNil brcHorizontalInside; 
										//(4 bytes): A Brc80MayBeNil structure
										//that specifies the horizontal border
										//between cells in this table row and
										//those in the preceding or succeeding
										//table rows.
  Brc80MayBeNil brcVerticalInside; 
										//(4 bytes): A Brc80MayBeNil structure
										//that specifies the vertical border
										//between neighboring cells of this
										//table row.
};

/* 2.9.349 XAS
 * The XAS value is a 16-bit signed integer that specifies
 * horizontal distance in twips. This value MUST
 * be greater than or equal to -31680 and less than or equal
 * to 31680. */
typedef SHORT XAS;

/* 2.9.313 TC80
 * The TC80 structure specifies the border and other
 * formatting for a single cell in a table.*/
struct TC80 {
	SHORT tcgrf;      //(2 bytes): A TCGRF that specifies
										//table cell formatting
	SHORT wWidth;     //(2 bytes): An integer that specifies
										//the preferred width of the cell. The
										//width includes cell margins, but does
										//not include cell spacing. This value
										//MUST be a non-negative number. The
										//unit of measurement depends on
										//tcgrf.ftsWidth. If tcgrf.ftsWidth is
										//set to ftsPercent, the value is a
										//fraction of the width of the entire
										//table.
	Brc80MayBeNil brcTop; 
										//(4 bytes): A Brc80MayBeNil structure
										//that specifies the border to be used
										//on the top side of the table cell.
	Brc80MayBeNil brcLeft;
										//(4 bytes): A Brc80MayBeNil structure
										//that specifies the border to be used
										//on the logical left side of the table
										//cell.
	Brc80MayBeNil brcBottom;
										//(4 bytes): A Brc80MayBeNil that
										//specifies the border to be used on
										//the bottom side of the table cell.
	Brc80MayBeNil brcRight; 
										//(4 bytes): A Brc80MayBeNil that
										//specifies the border to be used on
										//the logical right side of the table
										//cell.
};

/* 2.9.321 TDefTableOperand
 * The TDefTableOperand structure is the operand that is
 * used by the sprmTDefTable value. It
 * specifies the initial layout of the columns in the
 * current table row. */
struct TDefTableOperand {
	USHORT cb;            //(2 bytes): An unsigned integer
												//that specifies the number of bytes
												//that are used by the remainder of
												//this structure, incremented by 1.
	BYTE NumberOfColumns; //(1 byte): An integer that
												//specifies the number of columns in
												//this table. The number MUST be at
												//least zero, and MUST NOT exceed
												//63.
	BYTE *rgdxaCenter;    //(variable): An array of cAS. There
												//MUST be exactly one XAS value in
												//this array for every column
												//specified in NumberOfColumns,
												//incremented by 1. The first entry
												//specifies the horizontal position
												//of the logical left edge of the
												//table, as indented from the
												//logical left page margin. The
												//remaining entries specify the
												//horizontal positions of the
												//logical right edges of each cell
												//progressing logical right across
												//the row. More specifically, the
												//positions for all edges between
												//cells are the midpoints of the
												//inter-cell spacing. The first and
												//last entries specify the positions
												//of the outer edges of the table,
												//including all cell spacing. The
												//values in the array MUST be in
												//non-decreasing order.
	BYTE *rgTc80;         // (variable): An array of TC80 that
												// specifies the default formatting
												// for a cell in the table. Each
												// TC80 in the array corresponds to
												// the equivalent column in the
												// table. If there are fewer TC80s
												// than columns, the remaining
												// columns are formatted with the
												// default TC80 formatting. If there
												// are more TC80s than columns, the
												// excess TC80s MUST be ignored.
};

static int
TDefTableOperandInit(struct Prl *prl, struct TDefTableOperand *t)
{
#ifdef DEBUG
	LOG("start");
#endif

	t->cb = *(USHORT *)(prl->operand);
	t->NumberOfColumns = *(&(prl->operand[2]));
	t->rgdxaCenter = &(prl->operand[3]);
	t->rgTc80 = NULL;

	if (t->NumberOfColumns > 0){
		int size = t->cb - ((t->NumberOfColumns + 1)*2) - 1; 
#ifdef DEBUG
	LOG("size: %d", size);
#endif
		int len  = t->NumberOfColumns * sizeof(struct TC80);
		if (size > sizeof(struct TC80)){
			t->rgTc80 = (BYTE *)malloc(len);
			if (!t->rgTc80)
				return -1;
			memset(t->rgTc80, 0xFF, len);
			int i;
			for (i=0; i < size; i++){
				t->rgTc80[i] = 
					prl->operand[3 + t->NumberOfColumns * 2 + 2 + i];
			}
		}
	}
	return 0;
};

enum BordersToApply {
	BordersToApplyTop    = 0x01, //Top border.
	BordersToApplyLeft   = 0x02, //Logical left border.
	BordersToApplyBottom = 0x04, //Bottom border.
	BordersToApplyRight  = 0x08, //Logical right border.
	BordersToApplyiTLBR  = 0x10, //Border line from top left
															 //to bottom right.
	BordersToApplyTRBL   = 0x20, //Border line from top right
															 //to bottom left.
};

/* 2.9.305 TableBrcOperand
 * The TableBrcOperand structure is an operand that
 * specifies borders for a range of cells in a table
 * row */
struct TableBrcOperand {
	BYTE cb;             //(1 byte): An unsigned 
											 //integer that
											 //specifies the size, in bytes, of
											 //the remainder of this structure.
											 //This value MUST be 11.
	struct ItcFirstLim itc;
											 //(2 bytes): An ItcFirstLim
											 //structure that specifies the range
											 //of cell columns to which the
											 //border type format is applied.
	BYTE bordersToApply; //(1 byte): An unsigned integer that
											 //specifies which borders are
											 //affected. The value MUST be the
											 //result of the bitwise OR of any
											 //subset of the following values
											 //that specifies an edge to be
											 //formatted:
											 //0x01: Top border.
											 //0x02: Logical left border.
											 //0x04: Bottom border.
											 //0x08: Logical right border.
											 //0x10: Border line from top left to
											 //bottom right.
											 //0x20: Border line from top right
											 //to bottom left.
	Brc80MayBeNil brc;   //(8 bytes): A BrcMayBeNil structure
											 //that specifies the border type
											 //that is applied to the edges
											 //which are indicated by
											 //bordersToApply.
};

/* 2.9.304 TableBrc80Operand
 * The TableBrc80Operand structure is an operand that
 * specifies borders for a range of cells in a table
 * row. */
struct TableBrc80Operand {
	BYTE cb;             //(1 byte): An unsigned integer that
											 //specifies the size, in bytes, of
											 //the remainder of this structure.
											 //The value MUST be 7.
	struct ItcFirstLim itc; 
											//(2 bytes): An ItcFirstLim structure
											//that specifies the range of cell
											//columns to apply the border
											//type format.
	BYTE bordersToApply;//(1 byte): An unsigned integer that
											//specifies which borders are
											//affected. The value
											//MUST be the result of the bitwise
											//OR of any subset of the following
											//values that specifies an edge
											//to be formatted:
											//0x01: Top border.
											//0x02: Logical left border.
											//0x04: Bottom border.
											//0x08: Logical right border.
	Brc80MayBeNil brc;  //(4 bytes): A Brc80MayBeNil
											//structure that specifies the border
											//type that is applied to the edges
											//which are indicated by
											//bordersToApply.
};

/* 2.9.314 TCellBrcTypeOperand
 * A TCellBrcTypeOperand structure specifies an array of
 * border types for table cells. */
struct TCellBrcTypeOperand {
	BYTE cb;            //(1 byte): cb (1 byte): An unsigned
											//integer that specifies the size, in
											//bytes, of rgBrcType. This
											//value MUST be evenly divisible by
											//four.
	BYTE rgBrcType[];   //(variable): An array of BrcType
											//that specifies border types for a
											//set of table cells. Each
											//cell corresponds to four bytes.
											//Every four bytes specify the border
											//types of the top, left, bottom
											//and right borders, in that order.
};

/* 2.9.21 BrcOperand
 * The BrcOperand structure is the operand to several
 * SPRMs that control borders.*/
struct BrcOperand {
	BYTE cb;            //(1 byte): An unsigned integer value
											//that specifies the size of this
											//BrcOperand, not including this
											//byte. The cb MUST be 8.
	struct Brc brc;     //(8 bytes): A BRC that specifies the
											//border to be applied.
};

/* 2.9.146 LSPD
 * The LSPD structure specifies the spacing between lines
 * in a paragraph. */
struct LSPD {
	SHORT dyaLine;     //(16 bits): An integer that specifies
										 //the spacing between lines, based on
										 //the following rules:
										 //dyaLine MUST either be between
										 //0x0000 and 0x7BC0 or between 0x8440
										 //and 0xFFFF.
										 //When dyaLine is between 0x8440 and
										 //0xFFFF, the line spacing, in twips,
										 //is exactly 0x10000 minus dyaLine.
										 //When fMultLinespace is 0x0001 and
										 //dyaLine is between 0x0000 and
										 //0x7BC0, a spacing
										 //multiplier is used to determine line
										 //spacing for this paragraph. The
										 //spacing multiplier is
										 //dyaLine/240. For example, a spacing
										 //multiplier value of 1 specifies
										 //single spacing; a spacing multiplier
										 //value of 2 specifies double spacing;
										 //and so on. The actual line spacing,
										 //in twips, is the spacing multiplier
										 //times the font size, in twips
										 //When fMultLinespace is 0x0000 and
										 //dyaLine is between 0x0000 and
										 //0x7BC0, the line spacing, in twips,
										 //is dyaLine or the number of twips
										 //necessary for single spacing,
										 //whichever value isgreater.
	SHORT fMultLinespace; 
										//(16 bits): An integer which MUST be
										//either 0x0000 or 0x0001.
};

#endif /* ifndef OPERANDS_H */
