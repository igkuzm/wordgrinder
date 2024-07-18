/**
 * File              : operands.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 28.05.2024
 * Last Modified Date: 18.07.2024
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

/* 2.9.22 BrcType
 * brcType (8 bits): An unsigned integer that specifies the
 * type of border. Values that are larger than
 * 0x1B are not valid unless they describe a page border, in
 * which case they can be a value in the range
 * of 0x40 to 0xE3, inclusive.
 * Values MUST be from the following table. The reference
 * column specifies for each brcType value the
 * ST_Border enumeration value in [ECMA-376] part 4, section
 * 2.18.4, that further specifies the meaning
 * of the border type. */
enum BrcType {
	BrcTypeNone                   = 0x00, //No border. none
	BrcTypeLine                   = 0x01, //A single line.
																				//single
	BrcTypeDouble                 = 0x03, //A double line.
																				//double
	BrcTypeSolid                  = 0x05, //A thin single
																				//solid line.
	BrcTypeDotted                 = 0x06, //A dotted border.
																				//dotted
	BrcTypeDashed                 = 0x07, //A dashed border
																				//with large gaps
																				//between the
																				//dashes. dashed
	BrcTypeDotDash                = 0x08, // A border of
																				// alternating dots
																				// and dashes.
																				// dotDash
	BrcTypeDotDotDash             = 0x09, // A border of
																				// alternating sets
																				// of two dots and
																				// one dash.
																				// dotDotDash
	BrcTypeTriple                 = 0x0A, // A triple line
																				// border. triple
	BrcTypeThinThickSmallGap      = 0x0B, // A thin outer
																				// border and a
																				// thick inner
																				// border with a
																				// small gap between
	BrcTypeThinThickMediumGap     = 0x0C, // A thin outer
																				// border and thick
																				// inner border with
																				// a small gap
																				// between
	BrcTypeThinThickLargeGap      = 0x0D, // A thin outer
																				// border, a thick
																				// middle border,
																				// and a thin inner
																				// border with
	BrcTypeThickMediumGap         = 0x0E, // A thin outer
																				// border and a
																				// thick inner
																				// border with a
																				// medium gap
																				// between
	BrcTypeThickThinMediumGap     = 0x0F, // A thin outer
																				// border and a
																				// thick inner
																				// border and a
																				// medium gap
																				// between
	BrcTypeThinThickThinMediumGap = 0x10, // A thin outer
																				// border, a thick
																				// middle border,
																				// and a thin inner
																				// border witha
																				// medium gaps
																				// between them
																				// thinThickThinMediumGap
	BrcTypeThickLargeGap          = 0x11, //A thick outer
																				//border and a thin
																				//inner border with
																				//a large gap
																				//between them.
	BrcTypeThickThinLargeGap      = 0x12, //A thin outer
																				//border and a thick
																				//inner border with
																				//a large gap
																				//between them.
	BrcTypeThinThickThinLargeGap  = 0x13, //A thin outer
																				//border, a thick
																				//middle border, and
																				//a thin inner
																				//border with large
																				//gaps between them.
	BrcTypeWave                   = 0x14, //A single wavy
																				//line. wave
	BrcTypeDoubleWave             = 0x15, //A double wavy
																				//line. doubleWave
	BrcTypeDashSmallGap           = 0x16, //A dashed border
																				//with small gaps
																				//between the
																				//dashes.
	BrcTypeDashDotStroked         = 0x17, //A border
																				//consisting of
																				//alternating groups
																				//of 5 and 1 thin
																				//diagonal lines.
	BrcTypeThreeDEmboss           = 0x18, //A thin light gray
																				//outer border, a
																				//thick medium gray
																				//middle border,
																				//and a thin black
																				//inner border with
																				//no gaps between
																				//them.
	BrcTypeThreeDEngrave          = 0x19, //A thin black outer
																				//border, a thick
																				//medium gray middle
																				//border, and a thin
																				//light gray inner
																				//border with no
																				//gaps between them.
	BrcTypeOutset                 = 0x1A, //A thin light gray
																				//outer border and a
																				//thin medium gray
																				//inner border with
																				//a large gap
																				//between them.
	BrcTypeInset                  = 0x1B, //A thin medium gray
																				//outer border and a
																				//thin light gray
																				//inner border with
																				//a large gap
																				//between them.

//0x40 An image border. apples
//0x41 An image border. archedScallops
//0x42 An image border. babyPacifier
//0x43 An image border. babyRattle
//0x44 An image border. balloons3Colors
//0x45 An image border. balloonsHotAir
//0x46 An image border. basicBlackDashes
//0x47 An image border. basicBlackDots
//0x48 An image border. basicBlackSquares
//0x49 An image border. basicThinLines
//0x4A An image border. basicWhiteDashes
//0x4B An image border. basicWhiteDots
//0x4C An image border. basicWhiteSquares
//0x4D An image border. basicWideInline
//0x4E An image border. basicWideMidline
//0x4F An image border. basicWideOutline
//0x50 An image border. bats
//0x51 An image border. birds
//0x52 An image border. birdsFlight
//0x53 An image border. cabins
//0x54 An image border. cakeSlice
//0x55 An image border. candyCorn
//0x56 An image border. celticKnotwork
//0x57 An image border. certificateBanner
//0x58 An image border. chainLink
//0x59 An image border. champagneBottle
//0x5A An image border. checkedBarBlack
//0x5B An image border. checkedBarColor
//0x5C An image border. checkered
//0x5D An image border. christmasTree
//0x5E An image border. circlesLines
//0x5F An image border. circlesRectangles
//0x60 An image border. classicalWave
//0x61 An image border. clocks
//0x62 An image border. compass
//0x63 An image border. confetti
//0x64 An image border. confettiGrays
//0x65 An image border. confettiOutline
//0x66 An image border. confettiStreamers
//0x67 An image border. confettiWhite
//0x68 An image border. cornerTriangles
//0x69 An image border. couponCutoutDashes
//0x6A An image border. couponCutoutDots
//0x6B An image border. crazyMaze
//0x6C An image border. creaturesButterfly
//0x6D An image border. creaturesFish
//0x6E An image border. creaturesInsects
//0x6F An image border. creaturesLadyBug
//0x70 An image border. crossStitch
//0x71 An image border. cup
//0x72 An image border. decoArch
//0x73 An image border. decoArchColor
//0x74 An image border. decoBlocks
//0x75 An image border. diamondsGray
//0x76 An image border. doubleD
//0x77 An image border. doubleDiamonds
//0x78 An image border. earth1
//0x79 An image border. earth2
//0x7A An image border. eclipsingSquares1
//0x7B An image border. eclipsingSquares2
//0x7C An image border. eggsBlack
//0x7D An image border. fans
//0x7E An image border. film
//0x7F An image border. firecrackers
//0x80 An image border. flowersBlockPrint
//0x81 An image border. flowersDaisies
//0x82 An image border. flowersModern1
//0x83 An image border. flowersModern2
//0x84 An image border. flowersPansy
//0x85 An image border. flowersRedRose
//0x86 An image border. flowersRoses
//0x87 An image border. flowersTeacup
//0x88 An image border. flowersTiny
//0x89 An image border. gems
//0x8A An image border. gingerbreadMan
//0x8B An image border. gradient
//0x8C An image border. handmade1
//0x8D An image border. handmade2
//0x8E An image border. heartBalloon
//0x8F An image border. heartGray
//0x90 An image border. hearts
//0x91 An image border. heebieJeebies
//0x92 An image border. holly
//0x93 An image border. houseFunky
//0x94 An image border. hypnotic
//0x95 An image border. iceCreamCones
//0x96 An image border. lightBulb
//0x97 An image border. lightning1
//0x98 An image border. lightning2
//0x99 An image border. mapPins
//0x9A An image border. mapleLeaf
//0x9B An image border. mapleMuffins
//0x9C An image border. marquee
//0x9D An image border. marqueeToothed
//0x9E An image border. moons
//0x9F An image border. mosaic
//0xA0 An image border. musicNotes
//0xA1 An image border. northwest
//0xA2 An image border. ovals
//0xA3 An image border. packages
//0xA4 An image border. palmsBlack
//0xA5 An image border. palmsColor
//0xA6 An image border. paperClips
//0xA7 An image border. papyrus
//0xA8 An image border. partyFavor
//0xA9 An image border. partyGlass
//0xAA An image border. pencils
//0xAB An image border. people
//0xAC An image border. peopleWaving
//0xAD An image border. peopleHats
//0xAE An image border. poinsettias
//0xAF An image border. postageStamp
//0xB0 An image border. pumpkin1
//0xB1 An image border. pushPinNote2
//0xB2 An image border. pushPinNote1
//0xB3 An image border. pyramids
//0xB4 An image border. pyramidsAbove
//0xB5 An image border. quadrants
//0xB6 An image border. rings
//0xB7 An image border. safari
//0xB8 An image border. sawtooth
//0xB9 An image border. sawtoothGray
//0xBA An image border. scaredCat
//0xBB An image border. seattle
//0xBC An image border. shadowedSquares
//0xBD An image border. sharksTeeth
//0xBE An image border. shorebirdTracks
//0xBF An image border. skyrocket
//0xC0 An image border. snowflakeFancy
//0xC1 An image border. snowflakes
//0xC2 An image border. sombrero
//0xC3 An image border. southwest
//0xC4 An image border. stars
//0xC5 An image border. starsTop
//0xC6 An image border. stars3d
//0xC7 An image border. starsBlack
//0xC8 An image border. starsShadowed
//0xC9 An image border. sun
//0xCA An image border. swirligig
//0xCB An image border. tornPaper
//0xCC An image border. tornPaperBlack
//0xCD An image border. trees
//0xCE An image border. triangleParty
//0xCF An image border. triangles
//0xD0 An image border. tribal1
//0xD1 An image border. tribal2
//0xD2 An image border. tribal3
//0xD3 An image border. tribal4
//0xD4 An image border. tribal5
//0xD5 An image border. tribal6
//0xD6 An image border. twistedLines1
//0xD7 An image border. twistedLines2
//0xD8 An image border. vine
//0xD9 An image border. waveline
//0xDA An image border. weavingAngles
//0xDB An image border. weavingBraid
//0xDC An image border. weavingRibbon
//0xDD An image border. weavingStrips
//0xDE An image border. whiteFlowers
//0xDF An image border. woodwork
//0xE0 An image border. xIllusions
//0xE1 An image border. zanyTriangles
//0xE2 An image border. zigZag
//0xE3 An image border. zigZagStitch
//0xFF This MUST be ignored.
};

/* 2.9.16 Brc
 * The Brc structure specifies a border. */
struct Brc {
	ULONG vc;          //cv (4 bytes): A COLORREF that
										 //specifies the color of this border.
	BYTE dptLineWidth; //(8 bits): Specifies the width of the
										 //border. Different meanings based on
										 //brcType brcType Meaning
										 //brcType < 0x40 An unsigned integer
										 //that specifies the width of the
										 //border in 1/8- point increments.
										 //Values of less than 2 are considered
										 //to be equivalent to 2.
										 //brcType >= 0x40 An unsigned integer
										 //that specifies the width of the
										 //border in 1-point increments. This
										 //value MUST be less than 32.
	BYTE brcType;      //(1 byte): A BrcType that specifies
										 //the type of this border.  
	USHORT dptspace_A_B_fReserved;
										 //dptSpace (5bits): An unsigned integer that
										 //specifies the distance from the text
										 //to the border, in points. For page
										 //borders, sprmSPgbProp can specify
										 //that this value shall specify the
										 //distance from the edge of the page to
										 //the border.
										 //A - fShadow (1 bit): If this bit is 
										 //set, the border has an additional
										 //shadow effect. For top, logical left, 
										 //and between borders, this has no 
										 //visual effect.
										 //B - fFrame (1 bit): If this bit is 
										 //set, then the border has a 
										 //three-dimensional effect. For top, 
										 //logical left, and between borders, 
										 //this has no visual effect. For 
										 //visually symmetric border types, 
										 //this has no visual effect. 
										 //fReserved (9 bits): This value is 
										 //unused and MUST be ignored.
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

/* 2.9.17 Brc80
 * The Brc80 structure describes a border. */
struct Brc80 {
	BYTE dptLineWidth; //(8 bits): An unsigned integer that
										 //specifies the width of the border in
										 //1/8-point increments. Values of less 
										 //than 2 are considered to be equivalent to 2.
	BYTE brcType;      //(1 byte): A BrcType that specifies
										 //the type of this border. This value
										 //MUST NOT be 0x1A or 0x1B.
	BYTE ico;          //(1 byte): An Ico that specifies the
										 //color of this border.
	BYTE dptSpace_fShadow_B_C; 
										 //dptSpace (5 bits): An unsigned
										 //integer that specifies the distance
										 //from the text to the border, in points.
										 //A - fShadow (1 bit): If this bit is
										 //set, the border has an additional
										 //shadow effect. For top and logical
										 //left borders, this bit has no visual
										 //effect.
										 //B - fFrame (1 bit): Specifies whether
										 //the specified border is modified to
										 //create a frame effect by reversing
										 //the appearance of the border from the
										 //edge nearest the text to the edge
										 //furthest from the text. The frame
										 //effect shall only be applied to right
										 //and bottom borders.
										 //C - reserved (1 bit): This bit MUST
										 //be zero, and MUST be ignored.
};

/* 2.9.18 Brc80MayBeNil
 * The Brc80MayBeNil structure is a Brc80 structure. When
 * all bits are set (0xFFFFFFFF when
 * interpreted as a 4-byte unsigned integer), this structure
 * specifies that the region in question has no
 * border.*/
typedef struct Brc80 Brc80MayBeNil;

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

static struct TDefTableOperand *
TDefTableOperandInit(struct Prl *prl)
{
	struct TDefTableOperand *t = 
		NEW(struct TDefTableOperand, ERR("NEW"); return NULL);

	t->cb = *(USHORT *)(prl->operand);
	t->NumberOfColumns = *(&(prl->operand[2]));
	t->rgdxaCenter = &(prl->operand[3]);
	t->rgTc80 = NULL;

	if (t->NumberOfColumns > 0){
		int size = t->cb - ((t->NumberOfColumns + 1)*2) - 1; 
		int len  = t->NumberOfColumns * sizeof(struct TC80);
		if (size > sizeof(struct TC80)){
			t->rgTc80 = MALLOC(len, return t);
			memset(t->rgTc80, 0xFF, len);
			int i;
			for (i=0; i < size; i++){
				t->rgTc80[i] = 
					prl->operand[3 + t->NumberOfColumns * 2 + 2 + i];
			}
		}
	}
	return t;
};

static void 
TDefTableOperandFree(struct TDefTableOperand *t)
{
	if (t){
		if (t->rgTc80)
			free(t->rgTc80);
		free(t);
	}
}

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
