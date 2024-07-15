/**
 * File              : operands.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 28.05.2024
 * Last Modified Date: 31.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef OPERANDS_H
#define OPERANDS_H

#include "doc.h"
#include <stdint.h>
#include <stdbool.h>

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

#endif /* ifndef OPERANDS_H */
