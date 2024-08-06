/**
 * File              : doc.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 04.11.2022
 * Last Modified Date: 07.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef DOC_H_
#define DOC_H_

#ifdef __cplusplus
extern "C"{
#endif

//#define DEBUG

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../libdoc.h"
#include "../../ms-cfb/cfb.h"
#include "alloc.h"
#include "../../ms-cfb/log.h"
#include "../../ms-cfb/byteorder.h"
#include "str.h"

#define STRFCAT(str, ...) \
({ \
 char _s[BUFSIZ]; sprintf(_s, __VA_ARGS__); strcat(str, _s);\
})

typedef uint8_t  BYTE;
typedef int16_t  SHORT;
typedef uint16_t USHORT;
typedef int32_t  LONG;
typedef uint32_t ULONG;


/* [MS-DOC]: Word (.doc) Binary File Format Specifies the
 * Word (.doc) Binary File Format, which is the binary file
 * format used by Microsoft Word 97, Microsoft Word 2000,
 * Microsoft Word 2002, and Microsoft Office Word 2003.
 *
 * Characters 
 * The fundamental unit of a Word binary file is a
 * character. This includes visual characters such as
 * letters, numbers, and punctuation. It also includes
 * formatting characters such as paragraph marks, end of
 * cell marks, line breaks, or section breaks. Finally, it
 * includes anchor characters such as footnote reference
 * characters, picture anchors, and comment anchors.
 * Characters are indexed by their zero-based Character
 * Position, or CP.  
 * 
 * PLCs 
 * Many features of the Word Binary File Format pertain to a
 * range of CPs. For example, a bookmark is a range of CPs
 * that is named by the document author. As another example,
 * a field is made up of three control characters with
 * ranges of arbitrary document content between them.  The
 * Word Binary File Format uses a PLC structure to specify
 * these and other kinds of ranges of CPs. A PLC is simply a
 * mapping from CPs to other, arbitrary data. 
 *
 * Formatting 
 * The formatting of characters, paragraphs, sections,
 * tables, and pictures is specified as a set of differences
 * in formatting from the default formatting for these
 * objects. Modifications to individual properties are
 * expressed using a Prl. A Prl is a Single Property
 * Modifier, or Sprm, and an operand that specifies the new
 * value for the property. Each property has (at least) one
 * unique Sprm that modifies it. For example, sprmCFBold
 * modifies the bold formatting of text, and sprmPDxaLeft
 * modifies the logical left indent of a paragraph.  The
 * final set of properties for text, paragraphs, and tables
 * comes from a hierarchy of styles and from Prl elements
 * applied directly (for example, by the user selecting some
 * text and clicking the Bold button in the user interface).
 * Styles allow complex sets of properties to be specified
 * in a compact way.  They also allow the user to change the
 * appearance of a document without visiting every place in
 * the document where a change is necessary. The style sheet
 * for a document is specified by a STSH. 
 *
 * Tables 
 * A table consists of a set of paragraphs that has a
 * particular set of properties applied. There are special
 * characters that denote the ends of table cells and the
 * ends of table rows, but there are no characters to denote
 * the beginning of a table cell or the end of the table as
 * a whole. Tables can be nested inside other tables.
 * 
 * Pictures 
 * Pictures in the Word Binary File format can be either
 * inline or floating. An inline picture is represented by a
 * character whose Unicode value is 0x0001 and has
 * sprmCFSpec applied with a value of 1 and sprmCPicLocation
 * applied to specify the location of the picture data. A
 * floating picture is represented by an anchor character
 * with a Unicode value of 0x0008 with sprmCFSpec applied
 * with a value of 1. In addition, floating pictures are
 * referenced by a PlcfSpa structure which contains
 * additional data about the picture. A floating picture can
 * appear anywhere on the same page as its anchor. The
 * document author can choose to have the floating picture
 * rearrange the text in various ways or to leave the text
 * as is.
 *
 * The FIB 
 * The main stream of the Word Binary File Format begins
 * with a File Information Block, or FIB. The FIB specifies
 * the locations of all other data in the file. The
 * locations are specified by a pair of integers, the first
 * of which specifies the location and the second of which
 * specifies the size.  These integers appear in
 * substructures of the FIB such as the FibRgFcLcb97. The
 * location names are prefixed with fc; the size names are
 * prefixed with lcb.
 *
 * Byte Ordering 
 * Some computer architectures number bytes in a binary word
 * from left to right, which is referred to as big-endian.
 * The bit diagram for this documentation is big-endian.
 * Other architectures number the bytes in a binary word
 * from right to left, which is referred to as
 * little-endian. The underlying file format enumerations,
 * objects, and records are little-endian
 */

/*
 * Error codes
 */
enum {
	DOC_NO_ERR,     //no error
	DOC_CB_STOP,    //stopped by callback
	DOC_ERR_FILE,   //error to read file
	DOC_ERR_HEADER, //error to read header
	DOC_ERR_ALLOC,  //memory allocation error
};

/* 2.2.1 Character Position (CP)
 * A character position, which is also known as a CP, is an
 * unsigned 32-bit integer that serves as the
 * zero-based index of a character in the document text.
 * There is no requirement that the text at
 * consecutive character positions be at adjacent locations
 * in the file. The size of each character in the
 * file also varies. The location and size of each character
 * in the file can be computed using the algorithm
 * in section 2.4.1 (Retrieving Text).
 * Characters include the text of the document, anchors for
 * objects such as footnotes or textboxes, and
 * control characters such as paragraph marks and table cell
 * marks.
 * Unless otherwise specified by a particular usage, a CP
 * MUST be greater than or equal to zero and less
 * than 0x7FFFFFFF. The range of valid character positions
 * in a particular document is given by the
 * algorithm in section 2.4.1 (Retrieving Text). */

typedef ULONG CP; 
#define CPERROR 0x7FFFFFFF

/*
 * The File Information Block.
 * The Fib structure contains information about the document
 * and specifies the file pointers to various portions that
 * make up the document.  The Fib is a variable length
 * structure. With the exception of the base portion which
 * is fixed in size, every section is preceded with a count
 * field that specifies the size of the next section.
 *
 * base (32 bytes): The FibBase. 
 * 
 * csw (2 bytes): An unsigned integer that specifies the
 * count of 16-bit values corresponding to fibRgW 
 * that follow. MUST be 0x000E. 
 * 
 * fibRgW (28 bytes): The FibRgW97. 
 * 
 * cslw (2 bytes): An unsigned integer that specifies the
 * count of 32-bit values corresponding
 * to fibRgLw that follow. MUST be 0x0016. 
 *
 * fibRgLw (88 bytes): The FibRgLw97. 
 *
 * cbRgFcLcb (2 bytes): An unsigned integer that specifies
 * the count of 64-bit values corresponding to 
 * fibRgFcLcbBlob that follow. This MUST be one of the
 * following values, depending on the value of 
 * nFib.
 */
struct nFib2cbRgFcLcb {
	USHORT nFib;
	USHORT cbRgFcLcb;
};

static const struct nFib2cbRgFcLcb nFib2cbRgFcLcbTable[] = {
	{0x00C1, 0x005D}, 
	{0x00D9, 0x006C}, 
	{0x0101, 0x0088}, 
	{0x010C, 0x00A4}, 
	{0x0112, 0x00B7}, 
};

static int nFib2cbRgFcLcb_compare(
		const void *key, const void *value) 
{
    const struct nFib2cbRgFcLcb *cp1 = 
			(const struct nFib2cbRgFcLcb *)key;
    const struct nFib2cbRgFcLcb *cp2 = 
			(const struct nFib2cbRgFcLcb *)value;
    return cp1->nFib - cp2->nFib;
}

static USHORT cbRgFcLcb_get(USHORT nFib)
{
    struct nFib2cbRgFcLcb *result = 
			(struct nFib2cbRgFcLcb *)bsearch(
					&nFib, 
					nFib2cbRgFcLcbTable,
          sizeof(nFib2cbRgFcLcbTable)/
								 sizeof(nFib2cbRgFcLcbTable[0]),
          sizeof(nFib2cbRgFcLcbTable[0]), 
					nFib2cbRgFcLcb_compare);
	if (result)
		return result->cbRgFcLcb;
	return 0;
}

/*
 * fibRgFcLcbBlob (variable): The FibRgFcLcb. 
 *
 * cswNew (2 bytes): An unsigned integer that specifies the
 * count of 16-bit values corresponding to 
 * fibRgCswNew that follow. This MUST be one of the
 * following values, depending on the value of nFib. 
 */

/*
 * fibRgCswNew (variable): If cswNew is nonzero, this is
 * fibRgCswNew. Otherwise, it is not present
 * in the file. 
 */
struct nFib2cswNew {
	USHORT nFib;
	USHORT cswNew;
};

static const struct nFib2cswNew nFib2cswNewTable[] = {
	{0x00C1, 0     }, 
	{0x00D9, 0x0002}, 
	{0x0101, 0x0002}, 
	{0x010C, 0x0002}, 
	{0x0112, 0x0005}, 
};

static int nFib2cswNew_compare(
		const void *key, const void *value) 
{
    const struct nFib2cswNew *cp1 = 
			(const struct nFib2cswNew *)key;
    const struct nFib2cswNew *cp2 = 
			(const struct nFib2cswNew *)value;
    return cp1->nFib - cp2->nFib;
}

static USHORT cswNew_get(USHORT nFib)
{
    struct nFib2cswNew *result = 
			(struct nFib2cswNew *)bsearch(
					&nFib, 
					nFib2cswNewTable,
          sizeof(nFib2cswNewTable)/
								 sizeof(nFib2cswNewTable[0]),
          sizeof(nFib2cswNewTable[0]), 
					nFib2cswNew_compare);
	if (result)
		return result->cswNew;
	return 0;
}

/*
 * FibBase 
 * The FibBase structure is the fixed-size portion of the
 * Fib.*/
typedef struct FibBase 
{ 
  // Header 
  USHORT   wIdent;  //(2 bytes): An unsigned integer that
										//specifies that this is a Word Binary
										//File.  This value MUST be 0xA5EC. 
										
	USHORT   nFib;    //(2 bytes): An unsigned integer that
										//specifies the version number of the
										//file format used.  Superseded by
										//FibRgCswNew.nFibNew if it is present.
										//This value SHOULD be 0x00C1.  A
										//special empty document is installed
										//with Word 97, Word 2000, Word 2002,
										//and Office Word 2003 to allow "Create
										//New Word Document" from the operating
										//system. This document has an nFib of
										//0x00C0.  In addition the BiDi build of
										//Word 97 differentiates its documents
										//by saving 0x00C2 as the nFib. In both
										//cases treat them as if they were
										//0x00C1.

	USHORT  unused;   //(2 bytes)

	USHORT  lid;	    //(2 bytes): A LID that specifies the
										//install language of the application
										//that is producing the document. If
										//nFib is 0x00D9 or greater, then any
										//East Asian install lid or any install
										//lid with a base language of Spanish,
										//German or French MUST be recorded as
										//0x0409. If the nFib is 0x0101 or
										//greater, then any install lid with a
										//base language of Vietnamese, Thai, or
										//Hindi MUST be recorded as 0x0409.

	USHORT  pnNext;   //(2 bytes): An unsigned integer that
										//specifies the offset in the
										//WordDocument stream of the FIB for the
										//document which contains all the
										//AutoText items. If this value is 0,
										//there are no AutoText items attached.
										//Otherwise the FIB is found at file
										//location pnNext×512. If fGlsy is 1 or
										//fDot is 0, this value MUST be 0. If
										//pnNext is not 0, each FIB MUST share
										//the same values for FibRgFcLcb97.
										//fcPlcfBteChpx,
										//FibRgFcLcb97.lcbPlcfBteChpx,
										//FibRgFcLcb97.fcPlcfBtePapx,
										//FibRgFcLcb97.lcbPlcfBtePapx, and
										//FibRgLw97.cbMac

	USHORT ABCDEFGHIJKLM;
										//A - fDot (1 bit): Specifies whether
										//this is a document template.  B -
										//fGlsy (1 bit): Specifies whether this
										//is a document that contains only
										//AutoText items (see
										//FibRgFcLcb97.fcSttbfGlsy,
										//FibRgFcLcb97.fcPlcfGlsy and
										//FibRgFcLcb97.fcSttbGlsyStyle).  C -
										//fComplex (1 bit): Specifies that the
										//last save operation that was performed
										//on this document was an incremental
										//save operation.  D - fHasPic (1 bit):
										//When set to 0, there SHOULD be no
										//pictures in the document. Picture
										//watermarks could be present in the
										//document even if fHasPic is 0.  E -
										//cQuickSaves (4 bits): An unsigned
										//integer. If nFib is less than 0x00D9,
										//then cQuickSaves specifies the number
										//of consecutive times this document was
										//incrementally saved. If nFib is 0x00D9
										//or greater, then cQuickSaves MUST be
										//0xF F - fEncrypted (1 bit): Specifies
										//whether the document is encrypted or
										//obfuscated as specified in Encryption
										//and Obfuscation.  G - fWhichTblStm (1
										//bit): Specifies the Table stream to
										//which the FIB refers. When this value
										//is set to 1, use 1Table; when this
										//value is set to 0, use 0Table.  H -
										//fReadOnlyRecommended (1 bit):
										//Specifies whether the document author
										//recommended that the document be
										//opened in read-only mode.  I -
										//fWriteReservation (1 bit): Specifies
										//whether the document has a write-
										//reservation password.  J - fExtChar (1
										//bit): This value MUST be 1.  K -
										//fLoadOverride (1 bit): Specifies
										//whether to override the language
										//information and font that are
										//specified in the paragraph style at
										//istd 0 (the normal style) with the
										//defaults that are appropriate for the
										//installation language of the
										//application.  L - fFarEast (1 bit):
										//Specifies whether the installation
										//language of the application that
										//created the document was an East Asian
										//language M - fObfuscated (1 bit): If
										//fEncrypted is 1, this bit specifies
										//whether the document is obfuscated by
										//using XOR obfuscation; otherwise, this
										//bit MUST be ignored	 

	USHORT  nFibBack;
										//(2 bytes): This value SHOULD be
										//0x00BF. This value MUST be 0x00BF or
										//0x00C1. The nFibBack field is treated
										//as if it is set to 0x00BF when a
										//locale- specific version of Word 97
										//sets it to 0x00C1. 

	ULONG  lKey;      //(4 bytes): If fEncrypted is 1 and
										//fObfuscated is 1, this value specifies
										//the XOR obfuscation password verifier.
										//If fEncrypted is 1 and fObfuscated is
										//0, this value specifies the size of
										//the EncryptionHeader that is stored at
										//the beginning of the Table stream as
										//described in Encryption and
										//Obfuscation.  Otherwise, this value
										//MUST be 0. 

	BYTE  envr;       //(1 byte): This value MUST be 0, and
										//MUST be ignored. 
	BYTE NOPQRS;
										//N - fMac (1 bit): This value MUST be
										//0, and MUST be ignored.  O -
										//fEmptySpecial (1 bit): This value
										//SHOULD be 0 and SHOULD be ignored.
										//Word 97, Word 2000, Word 2002, and
										//Office Word 2003 install a minimal
										//.doc file for use with the New-
										//Microsoft Word Document of the shell.
										//This minimal .doc file has
										//fEmptySpecial set to 1.  Word uses
										//this flag to identify a document that
										//was created by using the New –
										//Microsoft Word Document of the
										//operating system shell P -
										//fLoadOverridePage (1 bit):  Specifies
										//whether to override the section
										//properties for page size, orientation,
										//and margins with the defaults that are
										//appropriate for the installation
										//language of the application.  Q -
										//reserved1 (1 bit): This value is
										//undefined and MUST be ignored.  R -
										//reserved2 (1 bit): This value is
										//undefined and MUST be ignored.  S -
										//fSpare0 (3 bits): This value is
										//undefined and MUST be ignored.

	USHORT reserved3; //(2 bytes): This value MUST be 0 and
										//MUST be ignored. 

	USHORT reserved4; //(2 bytes): This value MUST be 0 and
										//MUST be ignored. 

	ULONG reserved5;  //(4 bytes): This value MUST be 0 and
										//MUST be ignored. 

	ULONG reserved6;  //(4 bytes): This value MUST be 0 and
										//MUST be ignored. 
} FibBase;

static BYTE FibBaseA(FibBase *fibBase){
	return fibBase->ABCDEFGHIJKLM & 0x01;
}
static BYTE FibBaseB(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x02) >> 1;
}
static BYTE FibBaseC(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x04) >> 2;
}
static BYTE FibBaseD(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x08) >> 3;
}
static BYTE FibBaseE(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0xF0) >> 4;
}
static BYTE FibBaseF(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x0100) >> 8;
}
static BYTE FibBaseG(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x0200) >> 9;
}
static BYTE FibBaseH(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x0400) >> 10;
}
static BYTE FibBaseI(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x0800) >> 11;
}
static BYTE FibBaseJ(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x1000) >> 12;
}
static BYTE FibBaseK(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x2000) >> 13;
}
static BYTE FibBaseL(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x4000) >> 14;
}
static BYTE FibBaseM(FibBase *fibBase){
	return (fibBase->ABCDEFGHIJKLM & 0x8000) >> 15;
}
static BYTE FibBaseN(FibBase *fibBase){
	return fibBase->NOPQRS & 0x01;
}
static BYTE FibBaseO(FibBase *fibBase){
	return (fibBase->NOPQRS & 0x02) >> 1;
}
static BYTE FibBaseP(FibBase *fibBase){
	return (fibBase->NOPQRS & 0x04) >> 2;
}
static BYTE FibBaseQ(FibBase *fibBase){
	return (fibBase->NOPQRS & 0x08) >> 3;
}
static BYTE FibBaseR(FibBase *fibBase){
	return (fibBase->NOPQRS & 0x10) >> 4;
}
static BYTE FibBaseS(FibBase *fibBase){
	return (fibBase->NOPQRS & 0xE0) >> 5;
}

/*
 * FibRgW97
 * The FibRgW97 structure is a variable-length portion of
 * the Fib.*/
typedef struct FibRgW97 
{ 
	USHORT reserved1; //(2 bytes): This value MUST be 0 and
										//MUST be ignored. 
	USHORT reserved2; //(2 bytes): This value MUST be 0 and
										//MUST be ignored. 
	USHORT reserved3; //(2 bytes): This value MUST be 0 and
										//MUST be ignored. 
	USHORT reserved4; //(2 bytes): This value MUST be 0 and
										//MUST be ignored. 
	USHORT reserved5; //(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT reserved6; //(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT reserved7; //(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT reserved8; //(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT reserved9; //(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT reserved10;//(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT reserved11;//(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT reserved12;//(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT reserved13;//(2 bytes): This value MUST be 0 and
										//MUST be ignored.  Word 97 and Word
										//2000 can put a value here when
										//performing an incremental 
	USHORT lidFE;     //(2 bytes): A LID whose meaning depends
										//on the nFib value, which is one of the
										//following: 0x00C1 If FibBase.fFarEast
										//is "true", this is the LID of the
										//stored style names. Otherwise it MUST
										//be ignored 0x00D9, 0x0101, 0x010C,
										//0x0112 - The LID of the stored style
										//names (STD.  xstzName)

} FibRgW97;

/*
 * FibRgLw97FibRgLw97
 * The FibRgLw97 structure is the third section of the FIB.
 * This contains an array of 4-byte values.*/
typedef struct FibRgLw97 
{ 
	ULONG cbMac;     //(4 bytes): Specifies the count of bytes
									 //of those written to the WordDocument
									 //stream of the file that have any
									 //meaning. All bytes in the WordDocument
									 //stream at offset cbMac and greater MUST
									 //be ignored. 
	ULONG reserved1; //(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG reserved2; //(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG ccpText;   //(4 bytes): A signed integer that
									 //specifies the count of CPs in the main
									 //document. This value MUST be zero, 1,
									 //or greater. 
	ULONG ccpFtn;    //(4 bytes): A signed integer that
									 //specifies the count of CPs in the
									 //footnote subdocument. This value MUST
									 //be zero, 1, or greater 
	ULONG ccpHdd;    //(4 bytes): A signed integer that
									 //specifies the count of CPs in the
									 //header subdocument. This value MUST be
									 //zero, 1, or greater 
	ULONG reserved3; //(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG ccpAtn;    //(4 bytes): A signed integer that
									 //specifies the count of CPs in the
									 //comment subdocument. This value MUST be
									 //zero, 1, or greater 
	ULONG ccpEdn;    //(4 bytes): A signed integer that
									 //specifies the count of CPs in the
									 //endnote subdocument. This value MUST be
									 //zero, 1, or greater 
	ULONG ccpTxbx;   //(4 bytes): A signed integer that
									 //specifies the count of CPs in the
									 //textbox subdocument of the main
									 //document. This value MUST be zero, 1,
									 //or greater 
	ULONG ccpHdrTxbx;//(4 bytes): A signed integer that
									 //specifies the count of CPs in the
									 //textbox subdocument of the header. This
									 //value MUST be zero, 1, or greater 
	ULONG reserved4; //(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG reserved5; //(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG reserved6; //(4 bytes): This value MUST be equal or
									 //less than the number of data elements
									 //in PlcBteChpx, as specified by
									 //FibRgFcLcb97.fcPlcfBteChpx and
									 //FibRgFcLcb97.  lcbPlcfBteChpx. This
									 //value MUST be ignored. 
	ULONG reserved7; //(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG reserved8; //(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG reserved9; //(4 bytes): This value MUST be less than
									 //or equal to the number of data elements
									 //in PlcBtePapx, as specified by
									 //FibRgFcLcb97.fcPlcfBtePapx and
									 //FibRgFcLcb97.lcbPlcfBtePapx. This value
									 //MUST be ignored. 
	ULONG reserved10;//(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG reserved11;//(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG reserved12;//(4 bytes): This value is undefined and
									 //SHOULD be ignored.  Word 97, Word 2000,
									 //Word 2002, and Office Word 2003 write a
									 //nonzero value here when saving a
									 //document template with changes that
									 //require the saving of an AutoText
									 //document.
	ULONG reserved13;//(4 bytes): This value is undefined and
									 //MUST be ignored. 
	ULONG reserved14;//(4 bytes): This value is undefined and
									 //MUST be ignored. 
}FibRgLw97;

/*
 * FibRgFcLcb
 * The FibRgFcLcb structure specifies the file offsets and
 * byte counts for various portions of the data in the
 * document. The structure of FibRgFcLcb depends on the
 * value of nFib, which is one of the following.*/
enum RgFcLcb_t {
	RgFcLcbERROR_t,
	RgFcLcb97_t, 
	RgFcLcb2000_t, 
	RgFcLcb2002_t, 
	RgFcLcb2003_t, 
	RgFcLcb2007_t, 
};

struct nFib2fibRgFcLcb {
	USHORT nFib;
	enum RgFcLcb_t rgFcLcb;
};

static const struct nFib2cbRgFcLcb nFib2fibRgFcLcbTable[] = {
	{0x00C1, RgFcLcb97_t}, 
	{0x00D9, RgFcLcb2000_t}, 
	{0x0101, RgFcLcb2002_t}, 
	{0x010C, RgFcLcb2003_t}, 
	{0x0112, RgFcLcb2007_t}, 
};

static int nFib2fibRgFcLcb_compare(const void *key, const void *value) {
    const struct nFib2fibRgFcLcb *cp1 = 
			(const struct nFib2fibRgFcLcb *)key;
    const struct nFib2fibRgFcLcb *cp2 = 
			(const struct nFib2fibRgFcLcb *)value;
    return cp1->nFib - cp2->nFib;
}

static enum RgFcLcb_t rgFcLcb_get(USHORT nFib){
    struct nFib2fibRgFcLcb *result = 
			(struct nFib2fibRgFcLcb *)bsearch(
					&nFib, 
					nFib2fibRgFcLcbTable,
          sizeof(nFib2fibRgFcLcbTable)/
								 sizeof(nFib2fibRgFcLcbTable[0]),
          sizeof(nFib2fibRgFcLcbTable[0]),
				 	nFib2fibRgFcLcb_compare);
	if (result)	
		return result->rgFcLcb;
	return RgFcLcbERROR_t;
}

/*
 * FibRgFcLcb97
 * The FibRgFcLcb97 structure is a variable-length portion
 * of the Fib.*/
typedef struct FibRgFcLcb97 
{ 
	ULONG fcStshfOrig;  //(4 bytes): This value is undefined
											//and MUST be ignored. 
	ULONG lcbStshfOrig; //(4 bytes): This value is undefined
											//and MUST be ignored. 
	ULONG fcStshf;      //(4 bytes): An unsigned integer that
											//specifies an offset in the Table
											//Stream. An STSH that specifies the
											//style sheet for this document begins
											//at this offset. 
	ULONG lcbStshf;     //(4 bytes): An unsigned integer that
											//specifies the size, in bytes, of the
											//STSH that begins at offset fcStshf
											//in the Table Stream. This MUST be a
											//nonzero value. 
	ULONG fcPlcffndRef; //(4 bytes): An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcffndRef begins at this
											//offset and specifies the locations
											//of footnote references in the Main
											//Document, and whether those
											//references use auto-numbering or
											//custom symbols. If lcbPlcffndRef is
											//zero, fcPlcffndRef is undefined and
											//MUST be ignored. 
	ULONG lcbPlcffndRef;//(4 bytes): An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcffndRef that begins at offset
											//fcPlcffndRef in the Table Stream. 
	ULONG fcPlcffndTxt; //(4 bytes): An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcffndTxt begins at this
											//offset and specifies the locations
											//of each block of footnote text in
											//the Footnote Document.  If
											//lcbPlcffndTxt is zero, fcPlcffndTxt
											//is undefined and MUST be ignored. 
	ULONG lcbPlcffndTxt;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcffndTxt that begins at offset
											//fcPlcffndTxt in the Table Stream.
											//lcbPlcffndTxt MUST be zero if
											//FibRgLw97.ccpFtn is zero, and MUST
											//be nonzero if FibRgLw97.ccpFtn is
											//nonzero.
	ULONG fcPlcfandRef; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfandRef begins at this
											//offset and specifies the dates, user
											//initials, and locations of comments
											//in the Main Document. If
											//lcbPlcfandRef is zero, fcPlcfandRef
											//is undefined and MUST be ignored.
	ULONG lcbPlcfandRef;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcfandRef at offset fcPlcfandRef in
											//the Table Stream. 
	ULONG fcPlcfandTxt; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfandTxt begins at this
											//offset and specifies the locations
											//of comment text ranges in the
											//Comment Document. If lcbPlcfandTxt
											//is zero, fcPlcfandTxt is undefined,
											//and MUST be ignored. 
	ULONG lcbPlcfandTxt;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcfandTxt at offset fcPlcfandTxt in
											//the Table Stream.  lcbPlcfandTxt
											//MUST be zero if FibRgLw97.ccpAtn is
											//zero, and MUST be nonzero if
											//FibRgLw97.ccpAtn is nonzero.
	ULONG fcPlcfSed;    //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfSed begins at this
											//offset and specifies the locations
											//of property lists for each section
											//in the Main Document. If lcbPlcfSed
											//is zero, fcPlcfSed is undefined and
											//MUST be ignored.
	ULONG lcbPlcfSed;   //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcfSed that begins at offset
											//fcPlcfSed in the Table Stream. 
	ULONG fcPlcPad;     //(4 bytes):  This value is undefined
											//and MUST be ignored. 
	ULONG lcbPlcPad;    //(4 bytes):  This value MUST be zero,
											//and MUST be ignored. 
	ULONG fcPlcfPhe;    //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plc begins at this offset
											//and specifies version-specific
											//information about paragraph height.
											//This Plc SHOULD NOT be emitted and
											//SHOULD be ignored.  Word 97, Word
											//2000, and Word 2002 emit this
											//information when performing an
											//incremental save.  Office Word 2003,
											//Office Word 2007, Word 2010, and
											//Word 2013 do not emit this
											//information Word 97 reads this
											//information if FibBase.nFib is 193.
											//Word 2000 reads this information if
											//FibRgCswNew.nFibNew is 217.  Word
											//2002 reads this information if
											//FibRgCswNew.nFibNew is 257.  Office
											//Word 2003, Office Word 2007, Word
											//2010, and Word 2013 do not read this
											//information
	ULONG lcbPlcfPhe;   //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plc at offset fcPlcfPhe in the Table
											//Stream. 
	ULONG fcSttbfGlsy;  //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A SttbfGlsy that contains
											//information about the AutoText items
											//that are defined in this document
											//begins at this offset. 
	ULONG lcbSttbfGlsy; //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//SttbfGlsy at offset fcSttbfGlsy in
											//the Table Stream. If base.fGlsy of
											//the Fib that contains this
											//FibRgFcLcb97 is zero, this value
											//MUST be zero.
	ULONG fcPlcfGlsy;   //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfGlsy that contains
											//information about the AutoText items
											//that are defined in this document
											//begins at this offset.
	ULONG lcbPlcfGlsy;  //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcfGlsy at offset fcPlcfGlsy in the
											//Table Stream. If base.fGlsy of the
											//Fib that contains this FibRgFcLcb97
											//is zero, this value MUST be zero. 
	ULONG fcPlcfHdd;    //(4 bytes):  An unsigned integer that
											//specifies the offset in the Table
											//Stream where a Plcfhdd begins. The
											//Plcfhdd specifies the locations of
											//each block of header/footer text in
											//the WordDocument Stream. If
											//lcbPlcfHdd is 0, fcPlcfHdd is
											//undefined and MUST be ignored. 
	ULONG lcbPlcfHdd;   //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfhdd at offset fcPlcfHdd in the
											//Table Stream. If there is no
											//Plcfhdd, this value MUST be zero. A
											//Plcfhdd MUST exist if
											//FibRgLw97.ccpHdd indicates that
											//there are characters in the Header
											//Document (that is, if
											//FibRgLw97.ccpHdd is greater than 0).
											//Otherwise, the Plcfhdd MUST NOT
											//exist.
	ULONG fcPlcfBteChpx;//(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcBteChpx begins at the
											//offset. fcPlcfBteChpx MUST be
											//greater than zero, and MUST be a
											//valid offset in the Table Stream.
	ULONG lcbPlcfBteChpx;//(4 bytes):  An unsigned integer
											 //that specifies the size, in bytes,
											 //of the PlcBteChpx at offset
											 //fcPlcfBteChpx in the Table Stream.
											 //lcbPlcfBteChpx MUST be greater than
											 //zero. 
	ULONG fcPlcfBtePapx;//(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcBtePapx begins at the
											//offset. fcPlcfBtePapx MUST be
											//greater than zero, and MUST be a
											//valid offset in the Table Stream. 
	ULONG lcbPlcfBtePapx;//(4 bytes):  An unsigned integer
											 //that specifies the size, in bytes,
											 //of the PlcBtePapx at offset
											 //fcPlcfBtePapx in the Table Stream.
											 //lcbPlcfBteChpx MUST be greater than
											 //zero.
	ULONG fcPlcfSea;    //(4 bytes):  This value is undefined
											//and MUST be ignored.
	ULONG lcbPlcfSea;   //(4 bytes):  This value MUST be zero,
											//and MUST be ignored.
	ULONG fcSttbfFfn;   //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. An SttbfFfn begins at this
											//offset. This table specifies the
											//fonts that are used in the document.
											//If lcbSttbfFfn is 0, fcSttbfFfn is
											//undefined and MUST be ignored.
	ULONG lcbSttbfFfn;  //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//SttbfFfn at offset fcSttbfFfn in the
											//Table Stream. 
	ULONG fcPlcfFldMom; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfld begins at this
											//offset and specifies the locations
											//of field characters in the Main
											//Document. All CPs in this Plcfld
											//MUST be greater than or equal to 0
											//and less than or equal to
											//FibRgLw97.ccpText. If lcbPlcfFldMom
											//is zero, fcPlcfFldMom is undefined
											//and MUST be ignored. 
	ULONG lcbPlcfFldMom;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfld at offset fcPlcfFldMom in the
											//Table Stream. 
	ULONG fcPlcfFldHdr; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfld begins at this
											//offset and specifies the locations
											//of field characters in the Header
											//Document. All CPs in this Plcfld are
											//relative to the starting position of
											//the Header Document. All CPs in this
											//Plcfld MUST be greater than or equal
											//to zero and less than or equal to
											//FibRgLw97.ccpHdd. If lcbPlcfFldHdr
											//is zero, fcPlcfFldHdr is undefined
											//and MUST be ignored. 
	ULONG lcbPlcfFldHdr;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfld at offset fcPlcfFldHdr in the
											//Table Stream. 
	ULONG fcPlcfFldFtn; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfld begins at this
											//offset and specifies the locations
											//of field characters in the Footnote
											//Document. All CPs in this Plcfld are
											//relative to the starting position of
											//the Footnote Document. All CPs in
											//this Plcfld MUST be greater than or
											//equal to zero and less than or equal
											//to FibRgLw97.ccpFtn. If
											//lcbPlcfFldFtn is zero, fcPlcfFldFtn
											//is undefined, and MUST be ignored. 
	ULONG lcbPlcfFldFtn;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfld at offset fcPlcfFldFtn in the
											//Table Stream 
	ULONG fcPlcfFldAtn; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfld begins at this
											//offset and specifies the locations
											//of field characters in the Comment
											//Document. All CPs in this Plcfld are
											//relative to the starting position of
											//the Comment Document. All CPs in
											//this Plcfld MUST be greater than or
											//equal to zero and less than or equal
											//to FibRgLw97.ccpAtn. If
											//lcbPlcfFldAtn is zero, fcPlcfFldAtn
											//is undefined and MUST be ignored. 
	ULONG lcbPlcfFldAtn;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfld at offset fcPlcfFldAtn in the
											//Table Stream 
	ULONG fcPlcfFldMcr; //(4 bytes):  This value is undefined
											//and MUST be ignored. 
	ULONG lcbPlcfFldMcr;//(4 bytes):  This value MUST be zero,
											//and MUST be ignored. 
	ULONG fcSttbfBkmk;  //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. An SttbfBkmk that contains
											//the names of the bookmarks in the
											//document begins at this offset. If
											//lcbSttbfBkmk is zero, fcSttbfBkmk is
											//undefined and MUST be ignored.  This
											//SttbfBkmk is parallel to the Plcfbkf
											//at offset fcPlcfBkf in the Table
											//Stream. Each string specifies the
											//name of the bookmark that is
											//associated with the data element
											//which is located at the same offset
											//in that Plcfbkf. For this reason,
											//the SttbfBkmk that begins at offset
											//fcSttbfBkmk, and the Plcfbkf that
											//begins at offset fcPlcfBkf, MUST
											//contain the same number of elements. 
	ULONG lcbSttbfBkmk; //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//SttbfBkmk at offset fcSttbfBkmk. 
	ULONG fcPlcfBkf;    //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfbkf that contains
											//information about the standard
											//bookmarks in the document begins at
											//this offset. If lcbPlcfBkf is zero,
											//fcPlcfBkf is undefined and MUST be
											//ignored.  Each data element in the
											//Plcfbkf is associated, in a
											//one-to-one correlation, with a data
											//element in the Plcfbkl at offset
											//fcPlcfBkl.  For this reason, the
											//Plcfbkf that begins at offset
											//fcPlcfBkf, and the Plcfbkl that
											//begins at offset fcPlcfBkl, MUST
											//contain the same number of data
											//elements. This Plcfbkf is parallel
											//to the SttbfBkmk at offset
											//fcSttbfBkmk in the Table Stream.
											//Each data element in the Plcfbkf
											//specifies information about the
											//bookmark that is associated with the
											//element which is located at the same
											//offset in that SttbfBkmk. For this
											//reason, the Plcfbkf that begins at
											//offset fcPlcfBkf, and the SttbfBkmk
											//that begins at offset fcSttbfBkmk,
											//MUST contain the same number of
											//elements.  The largest value that a
											//CP marking the start or end of a
											//standard bookmark is allowed to have
											//is  the CP representing the end of
											//all document parts. 
	ULONG lcbPlcfBkf;   //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfbkf at offset fcPlcfBkf. 
	ULONG fcPlcfBkl;    //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfbkl that contains
											//information about the standard
											//bookmarks in the document begins at
											//this offset. If lcbPlcfBkl is zero,
											//fcPlcfBkl is undefined and MUST be
											//ignored.  Each data element in the
											//Plcfbkl is associated, in a
											//one-to-one correlation, with a data
											//element in the Plcfbkf at offset
											//fcPlcfBkf.  For this reason, the
											//Plcfbkl that begins at offset
											//fcPlcfBkl, and the Plcfbkf that
											//begins at offset fcPlcfBkf, MUST
											//contain the same number of data
											//elements. The largest value that a
											//CP marking the start or end of a
											//standard bookmark is allowed to have
											//is the value of the CP representing
											//the end of all document parts
	ULONG lcbPlcfBkl;   //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfbkl at offset fcPlcfBkl. 
	ULONG fcCmds;       //(4 bytes):  An unsigned integer that
											//specifies the offset in the Table
											//Stream of a Tcg that specifies
											//command-related customizations. If
											//lcbCmds is zero, fcCmds is undefined
											//and MUST be ignored. 
	ULONG lcbCmds;      //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Tcg at offset fcCmds. 
	ULONG fcUnused1;    //(4 bytes):  This value is undefined
											//and MUST be ignored.
	ULONG lcbUnused1;   //(4 bytes):  This value MUST be zero,
											//and MUST be ignored.
	ULONG fcSttbfMcr;   //(4 bytes):  This value is undefined
											//and MUST be ignored.
	ULONG lcbSttbfMcr;  //(4 bytes):  This value MUST be zero,
											//and MUST be ignored.
	ULONG fcPrDrvr;     //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. The PrDrvr, which contains
											//printer driver information (the
											//names of drivers, port, and so on),
											//begins at this offset. If lcbPrDrvr
											//is zero, fcPrDrvr is undefined and
											//MUST be ignored.
	ULONG lcbPrDrvr;    //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PrDrvr at offset fcPrDrvr. 
	ULONG fcPrEnvPort;  //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. The PrEnvPort that is the
											//print environment in portrait mode
											//begins at this offset. If
											//lcbPrEnvPort is zero, fcPrEnvPort is
											//undefined and MUST be ignored. 
	ULONG lcbPrEnvPort; //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PrEnvPort at offset fcPrEnvPort 
	ULONG fcPrEnvLand;  //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. The PrEnvLand that is the
											//print environment in landscape mode
											//begins at this offset. If
											//lcbPrEnvLand is zero, fcPrEnvLand is
											//undefined and MUST be ignored. 
	ULONG lcbPrEnvLand; //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PrEnvLand at offset fcPrEnvLand. 
	ULONG fcWss;        //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Selsf begins at this
											//offset and specifies the last
											//selection that was made in the Main
											//Document. If lcbWss is zero, fcWss
											//is undefined and MUST be ignored. 
	ULONG lcbWss;       //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Selsf at offset fcWss. 
	ULONG fcDop;        //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Dop begins at this offset. 
	ULONG lcbDop;       //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Dop at fcDop. This value MUST NOT be
											//zero. 
	ULONG fcSttbfAssoc; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. An SttbfAssoc that contains
											//strings that are associated with the
											//document begins at this offset. 
	ULONG lcbSttbfAssoc;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//SttbfAssoc at offset fcSttbfAssoc.
											//This value MUST NOT be zero 
	ULONG fcClx;        //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Clx begins at this offset. 
	ULONG lcbClx;       //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Clx at offset fcClx in the Table
											//Stream. This value MUST be greater
											//than zero. 
	ULONG fcPlcfPgdFtn; //(4 bytes):  This value is undefined
											//and MUST be ignored.
	ULONG lcbPlcfPgdFtn;//(4 bytes):  This value MUST be zero,
											//and MUST be ignored.
	ULONG fcAutosaveSource; //(4 bytes):  This value is
													//undefined and MUST be ignored.
	ULONG lcbAutosaveSource;//(4 bytes):  This value MUST be
													//zero, and MUST be ignored.
	ULONG fcGrpXstAtnOwners; //(4 bytes):  An unsigned integer
													 //that specifies an offset in the
													 //Table Stream. An array of XSTs
													 //begins at this offset. The
													 //value of cch for all XSTs in
													 //this array MUST be less than
													 //56. The number of entries in
													 //this array is limited to
													 //0x7FFF. This array contains the
													 //names of authors of comments in
													 //the document. The names in this
													 //array MUST be unique. If no
													 //comments are defined,
													 //lcbGrpXstAtnOwners and
													 //fcGrpXstAtnOwners MUST be zero
													 //and MUST be ignored. If any
													 //comments are in the document,
													 //fcGrpXstAtnOwners MUST point to
													 //a valid array of XSTs. 
	ULONG lcbGrpXstAtnOwners;//(4 bytes):  An unsigned integer
													 //that specifies the size, in
													 //bytes, of the XST array at
													 //offset fcGrpXstAtnOwners in the
													 //Table Stream. 
	ULONG fcSttbfAtnBkmk; //(4 bytes):  An unsigned integer
												//that specifies an offset in the
												//Table Stream. An SttbfAtnBkmk that
												//contains information about the
												//annotation bookmarks in the
												//document begins at this offset. If
												//lcbSttbfAtnBkmk is zero,
												//fcSttbfAtnBkmk is undefined and
												//MUST be ignored.  The SttbfAtnBkmk
												//is parallel to the Plcfbkf at
												//offset fcPlcfAtnBkf in the Table
												//Stream. Each element in the
												//SttbfAtnBkmk specifies information
												//about the bookmark which is
												//associated with the data element
												//that is located at the same offset
												//in that Plcfbkf, so the
												//SttbfAtnBkmk beginning at offset
												//fcSttbfAtnBkmk and the Plcfbkf
												//beginning at offset fcPlcfAtnBkf
												//MUST contain the same number of
												//elements. An additional constraint
												//upon the number of elements in the
												//SttbfAtnBkmk is specified in the
												//description of fcPlcfAtnBkf. 
	ULONG lcbSttbfAtnBkmk;//(4 bytes):  An unsigned integer
												//that specifies the size, in bytes,
												//of the SttbfAtnBkmk at offset
												//fcSttbfAtnBkmk. 
	ULONG fcUnused2;    //(4 bytes):  This value is undefined
											//and MUST be ignored.
	ULONG lcbUnused2;   //(4 bytes):  This value MUST be zero,
											//and MUST be ignored.
	ULONG fcUnused3;    //(4 bytes):  This value is undefined
											//and MUST be ignored.
	ULONG lcbUnused3;   //(4 bytes):  This value MUST be zero,
											//and MUST be ignored.
	ULONG fcPlcSpaMom;  //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfSpa begins at this
											//offset. The PlcfSpa contains shape
											//information for the Main Document.
											//All CPs in this PlcfSpa are relative
											//to the starting position of the Main
											//Document and MUST be greater than or
											//equal to zero and less than or equal
											//to ccpText in FibRgLw97.  The final
											//CP is undefined and MUST be ignored,
											//though it MUST be greater than the
											//previous entry. If there are no
											//shapes in the Main Document,
											//lcbPlcSpaMom and fcPlcSpaMom MUST be
											//zero and MUST be ignored. If there
											//are shapes in the Main Document,
											//fcPlcSpaMom MUST point to a valid
											//PlcfSpa structure
	ULONG lcbPlcSpaMom; //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcfSpa at offset fcPlcSpaMom. 
	ULONG fcPlcSpaHdr;  //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfSpa begins at this
											//offset. The PlcfSpa contains shape
											//information for the Header Document.
											//All CPs in this PlcfSpa are relative
											//to the starting position of the
											//Header Document and MUST be greater
											//than or equal to zero and less than
											//or equal to ccpHdd in FibRgLw97. The
											//final CP is undefined and MUST be
											//ignored, though this value MUST be
											//greater than the previous entry. If
											//there are no shapes in the Header
											//Document, lcbPlcSpaHdr and
											//fcPlcSpaHdr MUST both be zero and
											//MUST be ignored. If there are shapes
											//in the Header Document, fcPlcSpaHdr
											//MUST point to a valid PlcfSpa
											//structure. 
	ULONG lcbPlcSpaHdr; //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcfSpa at the offset fcPlcSpaHdr
	ULONG fcPlcfAtnBkf; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfbkf that contains
											//information about annotation
											//bookmarks in the document begins at
											//this offset. If lcbPlcfAtnBkf is
											//zero, fcPlcfAtnBkf is undefined and
											//MUST be ignored.  Each data element
											//in the Plcfbkf is associated, in a
											//one-to-one correlation, with a data
											//element in the Plcfbkl at offset
											//fcPlcfAtnBkl. For this reason, the
											//Plcfbkf that begins at offset
											//fcPlcfAtnBkf, and the Plcfbkl that
											//begins at offset fcPlcfAtnBkl, MUST
											//contain the same number of data
											//elements. The Plcfbkf is parallel to
											//the SttbfAtnBkmk at offset
											//fcSttbfAtnBkmk in the Table Stream.
											//Each data element in the Plcfbkf
											//specifies information about the
											//bookmark which is associated with
											//the element that is located at the
											//same offset in that SttbfAtnBkmk.
											//For this reason, the Plcfbkf that
											//begins at offset fcPlcfAtnBkf, and
											//the SttbfAtnBkmk that begins at
											//offset fcSttbfAtnBkmk, MUST contain
											//the same number of elements.  The CP
											//range of an annotation bookmark MUST
											//be in the Main Document part.
	ULONG lcbPlcfAtnBkf;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfbkf at offset fcPlcfAtnBkf.
	ULONG fcPlcfAtnBkl; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfbkl that contains
											//information about annotation
											//bookmarks in the document begins at
											//this offset. If lcbPlcfAtnBkl is
											//zero, then fcPlcfAtnBkl is undefined
											//and MUST be ignored.  Each data
											//element in the Plcfbkl is
											//associated, in a one-to-one
											//correlation, with a data element in
											//the Plcfbkf at offset fcPlcfAtnBkf.
											//For this reason, the Plcfbkl that
											//begins at offset fcPlcfAtnBkl, and
											//the Plcfbkf that begins at offset
											//fcPlcfAtnBkf, MUST contain the same
											//number of data elements.  The CP
											//range of an annotation bookmark MUST
											//be in the Main Document part.
	ULONG lcbPlcfAtnBkl;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfbkl at offset fcPlcfAtnBkl.
	ULONG fcPms;        //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Pms, which contains the
											//current state of a print merge
											//operation, begins at this offset. If
											//lcbPms is zero, fcPms is undefined
											//and MUST be ignored 
	ULONG lcbPms;       //(4 bytes):  An unsigned integer
											//which specifies the size, in bytes,
											//of the Pms at offset fcPms. 
	ULONG fcFormFldSttbs; //(4 bytes):  This value is
												//undefined and MUST be ignored 
	ULONG lcbFormFldSttbs;//(4 bytes):  This value MUST be
												//zero, and MUST be ignored 
	ULONG fcPlcfendRef; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfendRef that begins at
											//this offset specifies the locations
											//of endnote references in the Main
											//Document and whether those
											//references use auto-numbering or
											//custom symbols. If lcbPlcfendRef is
											//zero, fcPlcfendRef is undefined and
											//MUST be ignored. 
	ULONG lcbPlcfendRef;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcfendRef that begins at offset
											//fcPlcfendRef in the Table Stream 
	ULONG fcPlcfendTxt; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfendTxt begins at this
											//offset and specifies the locations
											//of each block of endnote text in the
											//Endnote Document.  If lcbPlcfendTxt
											//is zero, fcPlcfendTxt is undefined
											//and MUST be ignored 
	ULONG lcbPlcfendTxt;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//PlcfendTxt that begins at offset
											//fcPlcfendTxt in the Table Stream.
											//lcbPlcfendTxt MUST be zero if
											//FibRgLw97.ccpEdn is zero, and MUST
											//be nonzero if FibRgLw97.ccpEdn is
											//nonzero. 
	ULONG fcPlcfFldEdn; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. A Plcfld begins at this
											//offset and specifies the locations
											//of field characters in the Endnote
											//Document. All CPs in this Plcfld are
											//relative to the starting position of
											//the Endnote Document. All CPs in
											//this Plcfld MUST be greater than or
											//equal to zero and less than or equal
											//to FibRgLw97.ccpEdn. If
											//lcbPlcfFldEdn is zero, fcPlcfFldEdn
											//is undefined and MUST be ignored. 
	ULONG lcbPlcfFldEdn;//(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfld at offset fcPlcfFldEdn in the
											//Table Stream 
	ULONG fcUnused4;    //(4 bytes):  This value is undefined
											//and MUST be ignored 
	ULONG lcbUnused4;   //(4 bytes):  This value MUST be zero,
											//and MUST be ignored 
	ULONG fcDggInfo;    //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. An OfficeArtContent that
											//contains information about the
											//drawings in the document begins at
											//this offset. 
	ULONG lcbDggInfo;   //(4 bytes):  An unsigned integer that
											//specifies the size, in bytes, of the
											//OfficeArtContent at the offset
											//fcDggInfo. If lcbDggInfo is zero,
											//there MUST NOT be any drawings in
											//the document 
	ULONG fcSttbfRMark; //(4 bytes):  An unsigned integer that
											//specifies an offset in the Table
											//Stream. An SttbfRMark that contains
											//the names of authors who have added
											//revision marks or comments to the
											//document begins at this offset. If
											//lcbSttbfRMark is zero, fcSttbfRMark
											//is undefined and MUST be ignored 
	ULONG lcbSttbfRMark;//(4 bytes):   unsigned integer that
											//specifies the size, in bytes, of the
											//SttbfRMark at the offset
											//fcSttbfRMark 
	ULONG fcSttbfCaption; //(4 bytes):  An unsigned integer
												//that specifies an offset in the
												//Table Stream. An SttbfCaption that
												//contains information about the
												//captions that are defined in this
												//document begins at this offset.
												//If lcbSttbfCaption is zero,
												//fcSttbfCaption is undefined and
												//MUST be ignored. If this document
												//is not the Normal template, this
												//value MUST be ignored. 
	ULONG lcbSttbfCaption;//(4 bytes):  An unsigned integer
												//that specifies the size, in bytes,
												//of the SttbfCaption at offset
												//fcSttbfCaption in the Table
												//Stream. If base.fDot of the Fib
												//that contains this FibRgFcLcb97 is
												//zero, this value MUST be zero. 
	ULONG fcSttbfAutoCaption; //(4 bytes):  An unsigned
														//integer that specifies an
														//offset in the Table Stream. A
														//SttbfAutoCaption that contains
														//information about the
														//AutoCaption strings defined in
														//this document begins at this
														//offset.  If
														//lcbSttbfAutoCaption is zero,
														//fcSttbfAutoCaption is
														//undefined and MUST be ignored.
														//If this document is not the
														//Normal template, this value
														//MUST be ignored
	ULONG lcbSttbfAutoCaption;//(4 bytes):  An unsigned
														//integer that specifies the
														//size, in bytes, of the
														//SttbfAutoCaption at offset
														//fcSttbfAutoCaption in the
														//Table Stream. If base.fDot of
														//the Fib that contains this
														//FibRgFcLcb97 is zero, this
														//MUST be zero 
	ULONG fcPlcfWkb;    //(4 bytes):   unsigned integer that
											//specifies an offset in the Table
											//Stream. A PlcfWKB that contains
											//information about all master
											//documents and subdocuments begins at
											//this offset. 
	ULONG lcbPlcfWkb;   //(4 bytes):   An unsigned integer
											//that specifies the size, in bytes,
											//of the PlcfWKB at offset fcPlcfWkb
											//in the Table Stream. If lcbPlcfWkb
											//is zero, fcPlcfWkb is undefined and
											//MUST be ignored 
	ULONG fcPlcfSpl;    //(4 bytes):   An unsigned integer
											//that specifies an offset in the
											//Table Stream. A Plcfspl, which
											//specifies the state of the spell
											//checker for each text range, begins
											//at this offset. If lcbPlcfSpl is
											//zero, then fcPlcfSpl is undefined
											//and MUST be ignored. 
	ULONG lcbPlcfSpl;   //(4 bytes):    unsigned integer that
											//specifies the size, in bytes, of the
											//Plcfspl that begins at offset
											//fcPlcfSpl in the Table Stream 
	ULONG fcPlcftxbxTxt;//(4 bytes):    An unsigned integer
											//that specifies an offset in the
											//Table Stream. A PlcftxbxTxt begins
											//at this offset and specifies which
											//ranges of text are contained in
											//which textboxes. If lcbPlcftxbxTxt
											//is zero, fcPlcftxbxTxt is undefined
											//and MUST be ignored. 
	ULONG lcbPlcftxbxTxt;//(4 bytes):    An unsigned integer
											 //that specifies the size, in bytes,
											 //of the PlcftxbxTxt that begins at
											 //offset fcPlcftxbxTxt in the Table
											 //Stream.  lcbPlcftxbxTxt MUST be
											 //zero if FibRgLw97.ccpTxbx is zero,
											 //and MUST be nonzero if
											 //FibRgLw97.ccpTxbx is nonzero. 
	ULONG fcPlcfFldTxbx;//(4 bytes):    An unsigned integer
											//that specifies an offset in the
											//Table Stream. A Plcfld begins at
											//this offset and specifies the
											//locations of field characters in the
											//Textbox Document. All CPs in this
											//Plcfld are relative to the starting
											//position of the Textbox Document.
											//All CPs in this Plcfld MUST be
											//greater than or equal to zero and
											//less than or equal to
											//FibRgLw97.ccpTxbx. If lcbPlcfFldTxbx
											//is zero, fcPlcfFldTxbx is undefined
											//and MUST be ignored. 
	ULONG lcbPlcfFldTxbx;//(4 bytes):    An unsigned integer
											 //that specifies the size, in bytes,
											 //of the Plcfld at offset
											 //fcPlcfFldTxbx in the Table Stream 
	ULONG fcPlcfHdrtxbxTxt;//(4 bytes):    An unsigned integer
												 //that specifies an offset in the
												 //Table Stream. A PlcfHdrtxbxTxt
												 //begins at this offset and
												 //specifies which ranges of text
												 //are contained in which header
												 //textboxes. 
	ULONG lcbPlcfHdrtxbxTxt;//(4 bytes):    An unsigned
													//integer that specifies the size,
													//in bytes, of the PlcfHdrtxbxTxt
													//that begins at offset
													//fcPlcfHdrtxbxTxt in the Table
													//Stream.  lcbPlcfHdrtxbxTxt MUST
													//be zero if FibRgLw97.ccpHdrTxbx
													//is zero, and MUST be nonzero if
													//FibRgLw97.ccpHdrTxbx is nonzero. 
	ULONG fcPlcffldHdrTxbx;//(4 bytes):    An unsigned integer
												 //that specifies an offset in the
												 //Table Stream. A Plcfld begins at
												 //this offset and specifies the
												 //locations of field characters in
												 //the Header Textbox Document. All
												 //CPs in this Plcfld are relative
												 //to the starting position of the
												 //Header Textbox Document. All CPs
												 //in this Plcfld MUST be greater
												 //than or equal to zero and less
												 //than or equal to
												 //FibRgLw97.ccpHdrTxbx. If
												 //lcbPlcffldHdrTxbx is zero,
												 //fcPlcffldHdrTxbx is undefined,
												 //and MUST be ignored. 
	ULONG lcbPlcffldHdrTxbx;//(4 bytes):    An unsigned
													//integer that specifies the size,
													//in bytes, of the Plcfld at
													//offset fcPlcffldHdrTxbx in the
													//Table Stream 
	ULONG fcStwUser;    //(4 bytes):    An unsigned integer
											//that specifies an offset into the
											//Table Stream. An StwUser that
											//specifies the user-defined variables
											//and VBA digital signature, as
											//specified by [MS-OSHARED] section
											//2.3.2, begins at this offset. If
											//lcbStwUser is zero, fcStwUser is
											//undefined and MUST be ignored. 
	ULONG lcbStwUser;   //(4 bytes):    An unsigned integer
											//that specifies the size, in bytes,
											//of the StwUser at offset fcStwUser. 
	ULONG fcSttbTtmbd;  //(4 bytes):    An unsigned integer
											//that specifies an offset into the
											//Table Stream. A SttbTtmbd begins at
											//this offset and specifies
											//information about the TrueType fonts
											//that are embedded in the document.
											//If lcbSttbTtmbd is zero, fcSttbTtmbd
											//is undefined and MUST be ignored. 
	ULONG lcbSttbTtmbd; //(4 bytes):    An unsigned integer
											//that specifies the size, in bytes,
											//of the SttbTtmbd at offset
											//fcSttbTtmbd. 
	ULONG fcCookieData; //(4 bytes):    An unsigned integer
											//that specifies an offset in the
											//Table Stream. An RgCdb begins at
											//this offset. If lcbCookieData is
											//zero, fcCookieData is undefined and
											//MUST be ignored. Otherwise,
											//fcCookieData MAY be ignored.  Office
											//Word 2007, Word 2010, and Word 2013
											//ignore this data.
	ULONG lcbCookieData;//(4 bytes):    An unsigned integer
											//that specifies the size, in bytes,
											//of the RgCdb at offset fcCookieData
											//in the Table Stream 
	ULONG fcPgdMotherOldOld;//(4 bytes):    An unsigned
													//integer that specifies an offset
													//in the Table Stream. The
													//deprecated document page layout
													//cache begins at this offset.
													//Information SHOULD NOT be
													//emitted at this offset and
													//SHOULD be ignored. If
													//lcbPgdMotherOldOld is zero,
													//fcPgdMotherOldOld is undefined
													//and MUST be ignored.  Word 97
													//emits information at offset
													//fcPgdMotherOldOld. Neither Word
													//2000, Word 2002, Office Word
													//2003, Office Word 2007, Word
													//2010, nor Word 2013 emit this
													//information.  Word 97 reads this
													//information. Word 2000, Word
													//2002, Office Word 2003, Office
													//Word 2007, Word 2010, and Word
													//2013 ignore this information.
	ULONG lcbPgdMotherOldOld;//(4 bytes):    An unsigned
													 //integer that specifies the
													 //size, in bytes, of the
													 //deprecated document page layout
													 //cache at offset
													 //fcPgdMotherOldOld in the Table
													 //Stream.
	ULONG fcBkdMotherOldOld;//(4 bytes):    An unsigned
													//integer that specifies an offset
													//in the Table Stream. Deprecated
													//document text flow break cache
													//begins at this offset.
													//Information SHOULD NOT be
													//emitted at this offset and
													//SHOULD be ignored. If
													//lcbBkdMotherOldOld is zero,
													//fcBkdMotherOldOld is undefined
													//and MUST be ignored.  Word 97
													//emits information at offset
													//fcBkdMotherOldOld. Neither Word
													//2000, Word 2002, Office Word
													//2003, Office Word 2007, Word
													//2010, nor Word 2013 emit this
													//information.  Word 97 reads this
													//information. Word 2000, Word
													//2002, Office Word 2003, Office
													//Word 2007, Word 2010, and Word
													//2013 ignore this information.
	ULONG lcbBkdMotherOldOld;//(4 bytes):    An unsigned
													 //integer that specifies the
													 //size, in bytes, of the
													 //deprecated document text flow
													 //break cache at offset
													 //fcBkdMotherOldOld in the Table
													 //Stream.
	ULONG fcPgdFtnOldOld;//(4 bytes):    An unsigned integer
											 //that specifies an offset in the
											 //Table Stream. Deprecated footnote
											 //layout cache begins at this offset.
											 //Information SHOULD NOT be emitted
											 //at this offset and SHOULD be
											 //ignored. If lcbPgdFtnOldOld is
											 //zero, fcPgdFtnOldOld is undefined
											 //and MUST be ignored Word 97 emits
											 //information at offset
											 //fcPgdFtnOldOld. Neither Word 2000,
											 //Word 2002, Office Word 2003, Office
											 //Word 2007, Word 2010, nor Word 2013
											 //emit this information Word 97 reads
											 //this information. Word 2000, Word
											 //2002, Office Word 2003, Office Word
											 //2007, Word 2010, and Word 2013
											 //ignore this information.
	ULONG lcbPgdFtnOldOld;//(4 bytes):    An unsigned integer
												//that specifies the size, in bytes,
												//of the deprecated footnote layout
												//cache at offset fcPgdFtnOldOld in
												//the Table Stream 
	ULONG fcBkdFtnOldOld;//(4 bytes):    An unsigned integer
											 //that specifies an offset in the
											 //Table Stream. The deprecated
											 //footnote text flow break cache
											 //begins at this offset. Information
											 //SHOULD NOT be emitted at this
											 //offset and SHOULD be ignored. If
											 //lcbBkdFtnOldOld is zero,
											 //fcBkdFtnOldOld is undefined and
											 //MUST be ignored.  Word 97 emits
											 //information at offset
											 //fcBkdFtnOldOld. Neither Word 2000,
											 //Word 2002, Office Word 2003, Office
											 //Word 2007, Word 2010, nor Word 2013
											 //emit this information.  Word 97
											 //reads this information. Word 2000,
											 //Word 2002, Office Word 2003, Office
											 //Word 2007, Word 2010, and Word 2013
											 //ignore this information.
	ULONG lcbBkdFtnOldOld;//(4 bytes):    An unsigned integer
												//that specifies the size, in bytes,
												//of the deprecated footnote text
												//flow break cache at offset
												//fcBkdFtnOldOld in the Table
												//Stream. 
	ULONG fcPgdEdnOldOld;//(4 bytes):    An unsigned integer
											 //that specifies an offset in the
											 //Table Stream. The deprecated
											 //endnote layout cache begins at this
											 //offset. Information SHOULD NOT be
											 //emitted at this offset and SHOULD
											 //be ignored. If lcbPgdEdnOldOld is
											 //zero, fcPgdEdnOldOld is undefined
											 //and MUST be ignored.  Word 97 emits
											 //information at offset
											 //fcPgdEdnOldOld. Neither Word 2000,
											 //Word 2002, Office Word 2003, Office
											 //Word 2007, Word 2010, nor Word 2013
											 //emit this information Word 97 reads
											 //this information. Word 2000, Word
											 //2002, Office Word 2003, Office Word
											 //2007, Word 2010, and Word 2013
											 //ignore this information.
	ULONG lcbPgdEdnOldOld;//(4 bytes):    An unsigned integer
												//that specifies the size, in bytes,
												//of the deprecated endnote layout
												//cache at offset fcPgdEdnOldOld in
												//the Table Stream. 
	ULONG fcBkdEdnOldOld;//(4 bytes):     unsigned integer
											 //that specifies an offset in the
											 //Table Stream. The deprecated
											 //endnote text flow break cache
											 //begins at this offset. Information
											 //SHOULD NOT be emitted at this
											 //offset and SHOULD be ignored. If
											 //lcbBkdEdnOldOld is zero,
											 //fcBkdEdnOldOld is undefined and
											 //MUST be ignored.  Word 97 emits
											 //information at offset
											 //fcBkdEdnOldOld. Neither Word 2000,
											 //Word 2002, Office Word 2003, Office
											 //Word 2007, Word 2010, nor Word 2013
											 //emit this information.  Only Word
											 //97 reads this information.
	ULONG lcbBkdEdnOldOld;//(4 bytes):     An unsigned integer
												//that specifies the size, in bytes,
												//of the deprecated endnote text
												//flow break cache at offset
												//fcBkdEdnOldOld in the Table
												//Stream. 
	ULONG fcSttbfIntlFld; //(4 bytes):     This value is
												//undefined and MUST be ignored. 
	ULONG lcbSttbfIntlFld;//(4 bytes):     This value MUST be
												//zero, and MUST be ignored. 
	ULONG fcRouteSlip;  //(4 bytes):     An unsigned integer
											//that specifies an offset in the
											//Table Stream. A RouteSlip that
											//specifies the route slip for this
											//document begins at this offset. This
											//value SHOULD be ignored.
											//fcRouteSlip is only saved and read
											//by Word 97, Word 2000, Word 2002,
											//and Office Word 2003.
	ULONG lcbRouteSlip; //(4 bytes):     An unsigned integer
											//that specifies the size, in bytes,
											//of the RouteSlip at offset
											//fcRouteSlip in the Table Stream 
	ULONG fcSttbSavedBy;//(4 bytes):     An unsigned integer
											//that specifies an offset in the
											//Table Stream. A SttbSavedBy that
											//specifies the save history of this
											//document begins at this offset. This
											//value SHOULD be ignored.
											//SttbSavedBy is only saved and read
											//by Word 97 and Word 2000.
	ULONG lcbSttbSavedBy;//(4 bytes):     An unsigned integer
											 //that specifies the size, in bytes,
											 //of the SttbSavedBy at the offset
											 //fcSttbSavedBy. This value SHOULD be
											 //zero SttbSavedBy is only saved and
											 //read by Word 97 and Word 2000
	ULONG fcSttbFnm;    //(4 bytes):     An unsigned integer
											//that specifies an offset in the
											//Table Stream. An SttbFnm that
											//contains information about the
											//external files that are referenced
											//by this document begins at this
											//offset. If lcbSttbFnm is zero,
											//fcSttbFnm is undefined and MUST be
											//ignored. 
	ULONG lcbSttbFnm;   //(4 bytes):     An unsigned integer
											//that specifies the size, in bytes,
											//of the SttbFnm at the offset
											//fcSttbFnm. 
	ULONG fcPlfLst;     //(4 bytes):     An unsigned integer
											//that specifies an offset in the
											//Table Stream. A PlfLst that contains
											//list formatting information begins
											//at this offset. An array of LVLs is
											//appended to the PlfLst.  lcbPlfLst
											//does not account for the array of
											//LVLs. The size of the array of LVLs
											//is specified by the LSTFs in PlfLst.
											//For each LSTF whose fSimpleList is
											//set to 0x1, there is one LVL in the
											//array of LVLs that specifies the
											//level formatting of the single level
											//in the list which corresponds to the
											//LSTF. And, for each LSTF whose
											//fSimpleList is set to 0x0, there are
											//9 LVLs in the array of LVLs that
											//specify the level formatting of the
											//respective levels in the list which
											//corresponds to the LSTF. This array
											//of LVLs is in the same respective
											//order as the LSTFs in PlfLst. If
											//lcbPlfLst is 0, fcPlfLst is
											//undefined and MUST be ignored. 
	ULONG lcbPlfLst;    //(4 bytes):     An unsigned integer
											//that specifies the size, in bytes,
											//of the PlfLst at the offset
											//fcPlfLst. This does not include the
											//size of the array of LVLs that are
											//appended to the PlfLst 
	ULONG fcPlfLfo;     //(4 bytes):     An unsigned integer
											//that specifies an offset in the
											//Table Stream. A PlfLfo that contains
											//list formatting override information
											//begins at this offset. If lcbPlfLfo
											//is zero, fcPlfLfo is undefined and
											//MUST be ignored 
	ULONG lcbPlfLfo;    //(4 bytes):     An unsigned integer
											//that specifies the size, in bytes,
											//of the PlfLfo at the offset fcPlfLfo 
	ULONG fcPlcfTxbxBkd;//(4 bytes):     An unsigned integer
											//that specifies an offset in the
											//Table Stream. A PlcfTxbxBkd begins
											//at this offset and specifies which
											//ranges of text go inside which
											//textboxes 
	ULONG lcbPlcfTxbxBkd;//(4 bytes):     An unsigned integer
											 //that specifies the size, in bytes,
											 //of the PlcfTxbxBkd that begins at
											 //offset fcPlcfTxbxBkd in the Table
											 //Stream lcbPlcfTxbxBkd MUST be zero
											 //if FibRgLw97.ccpTxbx is zero, and
											 //MUST be nonzero if
											 //FibRgLw97.ccpTxbx is nonzero. 
	ULONG fcPlcfTxbxHdrBkd;//(4 bytes):     An unsigned
												 //integer that specifies an offset
												 //in the Table Stream. A
												 //PlcfTxbxHdrBkd begins at this
												 //offset and specifies which ranges
												 //of text are contained inside
												 //which header textboxes. 
	ULONG lcbPlcfTxbxHdrBkd;//(4 bytes):     An unsigned
													//integer that specifies the size,
													//in bytes, of the PlcfTxbxHdrBkd
													//that begins at offset
													//fcPlcfTxbxHdrBkd in the Table
													//Stream.  lcbPlcfTxbxHdrBkd MUST
													//be zero if FibRgLw97.ccpHdrTxbx
													//is zero, and MUST be nonzero if
													//FibRgLw97.ccpHdrTxbx is nonzero.
	ULONG fcDocUndoWord9;//(4 bytes):     An unsigned integer
											 //that specifies an offset in the
											 //WordDocument Stream.
											 //Version-specific undo information
											 //begins at this offset. This
											 //information SHOULD NOT be emitted
											 //and SHOULD be ignored.  Word 97 and
											 //Word 2000 write this information
											 //when the user chooses to save
											 //versions in the document. Word
											 //2002, Office Word 2003, Office Word
											 //2007, Word 2010, and Word 2013 do
											 //not write this information Word 97,
											 //Word 2000, Word 2002, and Office
											 //Word 2003 read this information.
											 //Office Word 2007, Word 2010, and
											 //Word 2013 ignore it.
	ULONG lcbDocUndoWord9;//(4 bytes):     An unsigned
												//integer. If this is nonzero,
												//version-specific undo information
												//exists at offset fcDocUndoWord9 in
												//the WordDocument Stream 
	ULONG fcRgbUse;     //(4 bytes):     An unsigned integer
											//that specifies an offset in the
											//WordDocument Stream.
											//Version-specific undo information
											//begins at this offset. This
											//information SHOULD NOT be emitted
											//and SHOULD be ignored.  Word 97,
											//Word 2000, Word 2002, and Office
											//Word 2003 write this information
											//when the user chooses to save
											//versions in the document.  Neither
											//Office Word 2007, Word 2010, nor
											//Word 2013 write this information.
											//Word 97, Word 2000, Word 2002, and
											//Office Word 2003 read this
											//information. Office Word 2007, Word
											//2010, and Word 2013 ignore it.
	ULONG lcbRgbUse;    //(4 bytes):      unsigned integer
											//that specifies the size, in bytes,
											//of the version-specific undo
											//information at offset fcRgbUse in
											//the WordDocument Stream 
	ULONG fcUsp;        //(4 bytes):      An unsigned integer
											//that specifies an offset in the
											//WordDocument Stream. Version-
											//specific undo information begins at
											//this offset. This information SHOULD
											//NOT be emitted and SHOULD be
											//ignored.  Word 97, Word 2000, Word
											//2002, and Office Word 2003 write
											//this information when the user
											//chooses to save versions in the
											//document.  Neither Office Word 2007,
											//Word 2010, nor Word 2013 write this
											//information.  Word 97, Word 2000,
											//Word 2002, and Office Word 2003 read
											//this information. Office Word 2007,
											//Word 2010, and Word 2013 ignore it
	ULONG lcbUsp;       //(4 bytes):      An unsigned integer
											//that specifies the size, in bytes,
											//of the version-specific undo
											//information at offset fcUsp in the
											//WordDocument Stream. 
	ULONG fcUskf;       //(4 bytes):      An unsigned integer
											//that specifies an offset in the
											//Table Stream. Version-specific undo
											//information begins at this offset.
											//This information SHOULD NOT be
											//emitted and SHOULD be ignored.  Word
											//97, Word 2000, Word 2002, and Office
											//Word 2003 write this information
											//when the user chooses to save
											//versions in the document.  Neither
											//Office Word 2007, Word 2010, nor
											//Word 2013 write this information.
											//Word 97, Word 2000, Word 2002, and
											//Office Word 2003 read this
											//information. Office Word 2007, Word
											//2010, and Word 2013 ignore it
	ULONG lcbUskf;      //(4 bytes):       unsigned integer
											//that specifies the size, in bytes,
											//of the version-specific undo
											//information at offset fcUskf in the
											//Table Stream 
	ULONG fcPlcupcRgbUse;//(4 bytes):       An unsigned
											 //integer that specifies an offset in
											 //the Table Stream. A Plc begins at
											 //this offset and contains
											 //version-specific undo information.
											 //This information SHOULD NOT be
											 //emitted and SHOULD be ignored Word
											 //97, Word 2000, Word 2002, and
											 //Office Word 2003 write this
											 //information when the user chooses
											 //to save versions in the document.
											 //Neither Office Word 2007, Word
											 //2010, nor Word 2013 write this
											 //information.  Word 97, Word 2000,
											 //Word 2002, and Office Word 2003
											 //read this information. Office Word
											 //2007, Word 2010, and wd15 ignore it
	ULONG lcbPlcupcRgbUse;//(4 bytes):       An unsigned
												//integer that specifies the size,
												//in bytes, of the Plc at offset
												//fcPlcupcRgbUse in the Table Stream
	ULONG fcPlcupcUsp;  //(4 bytes):       An unsigned integer
											//that specifies an offset in the
											//Table Stream. A Plc begins at this
											//offset and contains version-specific
											//undo information. This information
											//SHOULD NOT be emitted and SHOULD be
											//ignored.  Word 97, Word 2000, Word
											//2002, and Office Word 2003 write
											//this information when the user
											//chooses to save versions in the
											//document.  Neither Office Word 2007,
											//Word 2010, nor Word 2013 write this
											//information Word 97, Word 2000, Word
											//2002, and Office Word 2003 read this
											//information. Office Word 2007, Word
											//2010, and Word 2013 ignore it
	ULONG lcbPlcupcUsp; //(4 bytes):       An unsigned integer
											//that specifies the size, in bytes,
											//of the Plc at offset fcPlcupcUsp in
											//the Table Stream 
	ULONG fcSttbGlsyStyle;//(4 bytes):       An unsigned
												//integer that specifies an offset
												//in the Table Stream. A
												//SttbGlsyStyle, which contains
												//information about the styles that
												//are used by the AutoText items
												//which are defined in this
												//document, begins at this offset. 
	ULONG lcbSttbGlsyStyle;//(4 bytes):       An unsigned
												 //integer that specifies the size,
												 //in bytes, of the SttbGlsyStyle at
												 //offset fcSttbGlsyStyle in the
												 //Table Stream. If base.fGlsy of
												 //the Fib that contains this
												 //FibRgFcLcb97 is zero, this value
												 //MUST be zero. 
	ULONG fcPlgosl;     //(4 bytes):       An unsigned integer
											//that specifies an offset in the
											//Table Stream. A PlfGosl begins at
											//the offset. If lcbPlgosl is zero,
											//fcPlgosl is undefined and MUST be
											//ignored 
	ULONG lcbPlgosl;    //(4 bytes):       An unsigned integer
											//that specifies the size, in bytes,
											//of the PlfGosl at offset fcPlgosl in
											//the Table Stream 
	ULONG fcPlcocx;     //(4 bytes):       An unsigned integer
											//that specifies an offset in the
											//Table Stream. A RgxOcxInfo that
											//specifies information about the OLE
											//controls in the document begins at
											//this offset. When there are no OLE
											//controls in the document, fcPlcocx
											//and lcbPlcocx MUST be zero and MUST
											//be ignored. If there are any OLE
											//controls in the document, fcPlcocx
											//MUST point to a valid RgxOcxInfo 
	ULONG lcbPlcocx;    //(4 bytes):       An unsigned integer
											//that specifies the size, in bytes,
											//of the RgxOcxInfo at the offset
											//fcPlcocx 
	ULONG fcPlcfBteLvc; //(4 bytes):       An unsigned integer
											//that specifies an offset in the
											//Table Stream. A deprecated numbering
											//field cache begins at this offset.
											//This information SHOULD NOT be
											//emitted and SHOULD ignored. If
											//lcbPlcfBteLvc is zero, fcPlcfBteLvc
											//is undefined and MUST be ignored
											//Word 97, Word 2000, Word 2002, and
											//Office Word 2003 write this
											//information. Neither Office Word
											//2007, Word 2010, nor Word 2013 write
											//this information Word 97, Word 2000,
											//Word 2002, and Office Word 2003 read
											//this information. Office Word 2007,
											//Word 2010, and Word 2013 ignore it
	ULONG lcbPlcfBteLvc;//(4 bytes):       An unsigned integer
											//that specifies the size, in bytes,
											//of the deprecated numbering field
											//cache at offset fcPlcfBteLvc in the
											//Table Stream. This value SHOULD<58>
											//be zero 
	ULONG dwLowDateTime;//(4 bytes):       The low-order part
											//of a FILETIME structure, as
											//specified by [MS- DTYP], that
											//specifies when the document was last
											//saved 
	ULONG dwHighDateTime;//(4 bytes):       The high-order
											 //part of a FILETIME structure, as
											 //specified by [MS- DTYP], that
											 //specifies when the document was
											 //last saved 
	ULONG fcPlcfLvcPre10;//(4 bytes):       An unsigned
											 //integer that specifies an offset in
											 //the Table Stream. The deprecated
											 //list level cache begins at this
											 //offset.  Information SHOULD NOT be
											 //emitted at this offset and SHOULD
											 //be ignored. If lcbPlcfLvcPre10 is
											 //zero, fcPlcfLvcPre10 is undefined
											 //and MUST be ignored.  Word 97 emits
											 //information at offset
											 //fcPlcfLvcPre10 when performing an
											 //incremental save. Word 2000 emits
											 //information at offset
											 //fcPlcfLvcPre10 on every save.
											 //Neither Word 2002, Office Word
											 //2003, Office Word 2007, Word 2010,
											 //nor Word 2013 emit information at
											 //offset fcPlcfLvcPre10 and the value
											 //of fcPlcfLvcPre10 is undefined Word
											 //97 and Word 2000 read this
											 //information. Word 2002, Office Word
											 //2003, Office Word 2007, Word 2010,
											 //and Word 2013 ignore it
	ULONG lcbPlcfLvcPre10;//(4 bytes):       An unsigned
												//integer that specifies the size,
												//in bytes, of the deprecated list
												//level cache at offset
												//fcPlcfLvcPre10 in the Table
												//Stream. This value SHOULD be zero
												//Word 97 and Word 2000 write
												//lcbPlcfLvcPre10 with the size, in
												//bytes, of the information emitted
												//at offset fcPlcfLvcPre10. Word
												//2002, Office Word 2003, Office
												//Word 2007, Word 2010, and Word
												//2013 write 0 to lcbPlcfLvcPre10
	ULONG fcPlcfAsumy;  //(4 bytes):       An unsigned integer
											//that specifies an offset in the
											//Table Stream. A PlcfAsumy begins at
											//the offset. If lcbPlcfAsumy is zero,
											//fcPlcfAsumy is undefined and MUST be
											//ignored 
	ULONG lcbPlcfAsumy; //(4 bytes):        unsigned integer
											//that specifies the size, in bytes,
											//of the PlcfAsumy at offset
											//fcPlcfAsumy in the Table Stream 
	ULONG fcPlcfGram;   //(4 bytes):       An unsigned integer
											//that specifies an offset in the
											//Table Stream. A Plcfgram, which
											//specifies the state of the grammar
											//checker for each text range, begins
											//at this offset. If lcbPlcfGram is
											//zero, then fcPlcfGram is undefined
											//and MUST be ignored
	ULONG lcbPlcfGram;  //(4 bytes):       An unsigned integer
											//that specifies the size, in bytes,
											//of the Plcfgram that begins at
											//offset fcPlcfGram in the Table
											//Stream
	ULONG fcSttbListNames;//(4 bytes):     An unsigned integer
												//that specifies an offset in the
												//Table Stream. A SttbListNames,
												//which specifies the LISTNUM field
												//names of the lists in the
												//document, begins at this offset.
												//If lcbSttbListNames is zero,
												//fcSttbListNames is undefined and
												//MUST be ignored 
	ULONG lcbSttbListNames;//(4 bytes):     An unsigned
												 //integer that specifies the size,
												 //in bytes, of the SttbListNames at
												 //the offset fcSttbListNames 
	ULONG fcSttbfUssr;  //(4 bytes):     An unsigned integer
											//that specifies an offset in the
											//Table Stream. The deprecated,
											//version-specific undo information
											//begins at this offset. This
											//information SHOULD NOT be emitted
											//and SHOULD be ignored Word 97, Word
											//2000, Word 2002, and Office Word
											//2003 write this information when the
											//user chooses to save versions in the
											//document.  Neither Office Word 2007,
											//Word 2010, nor Word 2013 write this
											//information Word 97, Word 2000, Word
											//2002, and Office Word 2003 read this
											//information. Office Word 2007, Word
											//2010, and Word 2013 ignore it
	ULONG lcbSttbfUssr; //(4 bytes):     An unsigned integer
											//that specifies the size, in bytes,
											//of the deprecated, version-specific
											//undo information at offset
											//fcSttbfUssr in the Table Stream 
} FibRgFcLcb97;

/*
 * FibRgFcLcb2000
 * The FibRgFcLcb2000 structure is a variable-sized portion
 * of the Fib. It extends the 
 * FibRgFcLcb97.*/
typedef struct FibRgFcLcb2000 
{
	FibRgFcLcb97 rgFcLcb97;//(744 bytes): The contained FibRgFcLcb97
	ULONG fcPlcfTch;    //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. A PlcfTch begins at this offset and specifies a cache 
						   //of table characters. Information at this offset SHOULD be ignored. If 
						   //lcbPlcfTch is zero, fcPlcfTch is undefined and MUST be ignored 
	ULONG lcbPlcfTch;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the PlcfTch at offset fcPlcfTch 
	ULONG fcRmdThreading;//(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. An RmdThreading that specifies the data concerning the 
						   //e-mail messages and their authors in this document begins at this 
						   //offset 
	ULONG lcbRmdThreading;//(4 bytes):   An unsigned integer that specifies the size, in bytes,
						   //of the RmdThreading at the offset fcRmdThreading. This value MUST NOT
						   //be zero. 
	ULONG fcMid;        //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. A double-byte character Unicode string that specifies 
						   //the message identifier of the document begins at this offset. This 
						   //value MUST be ignored 
	ULONG lcbMid;       //(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the double-byte character Unicode string at offset fcMid. This 
						   //value MUST be ignored 
	ULONG fcSttbRgtplc; //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. A SttbRgtplc that specifies the styles of lists in the 
						   //document begins at this offset. If lcbSttbRgtplc is zero, 
						   //fcSttbRgtplc is undefined and MUST be ignored 
	ULONG lcbSttbRgtplc;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the SttbRgtplc at the offset fcSttbRgtplc 
	ULONG fcMsoEnvelope;//(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. An MsoEnvelopeCLSID, which specifies the envelope data
						   //as specified by [MS-OSHARED] section 2.3.8.1, begins at this offset. 
						   //If lcbMsoEnvelope is zero, fcMsoEnvelope is undefined and MUST be 
						   //ignored 
	ULONG lcbMsoEnvelope;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the MsoEnvelopeCLSID at the offset fcMsoEnvelope 
	ULONG fcPlcfLad;    //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. A Plcflad begins at this offset and specifies the 
						   //language auto-detect state of each text range. If lcbPlcfLad is zero, 
						   //fcPlcfLad is undefined and MUST be ignored 
	ULONG lcbPlcfLad;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the Plcflad that begins at offset fcPlcfLad in the Table Stream 
	ULONG fcRgDofr;     //(4 bytes):   An unsigned integer that specifies an offset in the Table
						   //Stream. A variable-length array with elements of type Dofrh begins at 
						   //that offset. The elements of this array are records that support the 
						   //frame set and list style features. If lcbRgDofr is zero, fcRgDofr is 
						   //undefined and MUST be ignored. 
	ULONG lcbRgDofr;    //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the array that begins at offset fcRgDofr in the Table Stream.
	ULONG fcPlcosl;     //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A PlfCosl begins at the offset. If lcbPlcosl is zero, fcPlcosl
						   //is undefined and MUST be ignored.
	ULONG lcbPlcosl;    //(4 bytes):   An unsigned integer that specifies the size, in bytes, of 
	                       //the PlfCosl at offset fcPlcosl in the Table Stream.
	ULONG fcPlcfCookieOld;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. A PlcfcookieOld begins at this offset. If 
						   //lcbPlcfcookieOld is zero, fcPlcfcookieOld is undefined and MUST be 
						   //ignored. fcPlcfcookieOld MAY be ignored. 
						   //Word 2002, Office Word 2003, Office Word 2007, Word 2010, and Word 
						   //2013 ignore this value.
	ULONG lcbPlcfCookieOld;//(4 bytes):   An unsigned integer that specifies the size, in bytes,
                           //of the PlcfcookieOld at offset fcPlcfcookieOld in the Table Stream. 
	ULONG fcPgdMotherOld;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. The deprecated document page layout cache begins at this
						   //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						   //ignored. If lcbPgdMotherOld is zero, fcPgdMotherOld is undefined and 
						   //MUST be ignored.
						   //Word 2000 and Word 2002 emit information at offset fcPgdMotherOld. 
						   //Neither Word 97, Office Word 2003, Office Word 2007, Word 2010, nor 
						   //Word 2013 emit this information
						   //Word 2000 and Word 2002 read this information. Word 97, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore this 
						   //information
	ULONG lcbPgdMotherOld;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the deprecated document page layout cache at offset fcPgdMotherOld 
						   //in the Table Stream 
	ULONG fcBkdMotherOld;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. The deprecated document text flow break cache begins at 
						   //this offset. Information SHOULD NOT be emitted at this offset and 
						   //SHOULD be ignored. If lcbBkdMotherOld is zero, fcBkdMotherOld is 
						   //undefined and MUST be ignored 
						   //Word 2000 and Word 2002 emit information at offset fcBkdMotherOld. 
						   //Neither Word 97, Office Word 2003, Office Word 2007, Word 2010, nor 
						   //Word 2013 emit this information
						   //Word 2000 and Word 2002 read this information. Word 97, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore this 
						   //information
	ULONG lcbBkdMotherOld;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the deprecated document text flow break cache at offset 
						   //fcBkdMotherOld in the Table Stream 
	ULONG fcPgdFtnOld;  //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. The deprecated footnote layout cache begins at this offset. 
						   //Information SHOULD NOT be emitted at this offset and SHOULD be 
						   //ignored. If lcbPgdFtnOld is zero, fcPgdFtnOld is undefined and MUST be
						   //ignored 
						   //Word 2000 and Word 2002 emit information at offset fcPgdFtnOld. 
						   //Neither Word 97, Office Word 2003, Office Word 2007, Word 2010, nor 
						   //Word 2013 emit this information
						   //Word 2000 and Word 2002 read this information. Word 97, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore this 
						   //information
	ULONG lcbPgdFtnOld; //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the deprecated footnote layout cache at offset fcPgdFtnOld in the 
						   //Table Stream
	ULONG fcBkdFtnOld;  //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. The deprecated footnote text flow break cache begins at this 
						   //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						   //ignored. If lcbBkdFtnOld is zero, fcBkdFtnOld is undefined and MUST be
						   //ignored
						   //Word 2000 and Word 2002 emit information at offset fcBkdFtnOld. 
						   //Neither Word 97, Office Word 2003, Office Word 2007, Word 2010, nor 
						   //Word 2013 emit this information
						   //Word 2000 and Word 2002 read this information. Word 97, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore this 
						   //information
	ULONG lcbBkdFtnOld; //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the deprecated footnote text flow break cache at offset fcBkdFtnOld 
						   //in the Table Stream
	ULONG fcPgdEdnOld;  //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. The deprecated endnote layout cache begins at this offset. 
						   //Information SHOULD NOT be emitted at this offset and SHOULD be 
						   //ignored. If lcbPgdEdnOld is zero, fcPgdEdnOld is undefined and MUST be
						   //ignored
						   //Word 2000 and Word 2002 emit information at offset fcPgdEdnOld. 
						   //Neither Word 97, Office Word 2003, Office Word 2007, Word 2010, nor 
						   //Word 2013 emit this information
						   //Word 2000 and Word 2002 read this information. Word 97, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore this 
						   //information
	ULONG lcbPgdEdnOld; //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the deprecated endnote layout cache at offset fcPgdEdnOld in the Table
						   //Stream
	ULONG fcBkdEdnOld;  //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. The deprecated endnote text flow break cache begins at this 
						   //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						   //ignored. If lcbBkdEdnOld is zero, fcBkdEdnOld is undefined and MUST be
						   //ignored
						   //Word 2000 and Word 2002 emit information at offset fcBkdEdnOld. 
						   //Neither Word 97, Office Word 2003, Office Word 2007, Word 2010, nor 
						   //Word 2013 emit this information
						   //Word 2000 and Word 2002 read this information. Word 97, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore this 
						   //information
	ULONG lcbBkdEdnOld; //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the deprecated endnote text flow break cache at offset fcBkdEdnOld in 
						   //the Table Stream
} FibRgFcLcb2000;

/*
 * FibRgFcLcb2002
 * The FibRgFcLcb2002 structure is a variable-sized portion of the Fib. It extends the 
 * FibRgFcLcb2000.
 */
typedef struct FibRgFcLcb2002 
{
	FibRgFcLcb2000 rgFcLcb2000;//(864 bytes):  The contained FibRgFcLcb2000.
	ULONG fcUnused1;    //(4 bytes):   This value is undefined and MUST be ignored.
	ULONG lcbUnused1;   //(4 bytes):   This value MUST be zero, and MUST be ignored.
	ULONG fcPlcfPgp;    //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A PGPArray begins at this offset. If lcbPlcfPgp is 0, 
						   //fcPlcfPgp is undefined and MUST be ignored
	ULONG lcbPlcfPgp;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the PGPArray that is stored at offset fcPlcfPgp
	ULONG fcPlcfuim;    //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A Plcfuim begins at this offset. If lcbPlcfuim is zero, 
						   //fcPlcfuim is undefined and MUST be ignored
	ULONG lcbPlcfuim;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the Plcfuim at offset fcPlcfuim
	ULONG fcPlfguidUim; //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A PlfguidUim begins at this offset. If lcbPlfguidUim is zero, 
						   //fcPlfguidUim is undefined and MUST be ignored
	ULONG lcbPlfguidUim;//(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the PlfguidUim at offset fcPlfguidUim
	ULONG fcAtrdExtra;  //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. An AtrdExtra begins at this offset. If lcbAtrdExtra is zero, 
						   //fcAtrdExtra is undefined and MUST be ignored
	ULONG lcbAtrdExtra; //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the AtrdExtra at offset fcAtrdExtra in the Table Stream
	ULONG fcPlrsid;     //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A PLRSID begins at this offset. If lcbPlrsid is zero, fcPlrsid
						   //is undefined and MUST be ignored
	ULONG lcbPlrsid;    //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the PLRSID at offset fcPlrsid in the Table Stream
	ULONG fcSttbfBkmkFactoid;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. An SttbfBkmkFactoid containing information about smart 
						   //tag bookmarks in the document begins at this offset. If 
						   //lcbSttbfBkmkFactoid is zero, fcSttbfBkmkFactoid is undefined and MUST 
						   //be ignored.  
						   //The SttbfBkmkFactoid is parallel to the Plcfbkfd at offset 
						   //fcPlcfBkfFactoid in the Table Stream. Each element in the 
						   //SttbfBkmkFactoid specifies information about the bookmark that is 
						   //associated with the data element which is located at the same offset 
						   //in that Plcfbkfd. For this reason, the SttbfBkmkFactoid that begins at
						   //offset fcSttbfBkmkFactoid, and the Plcfbkfd that begins at offset 
						   //fcPlcfBkfFactoid, MUST contain the same number of elements
	ULONG lcbSttbfBkmkFactoid;//(4 bytes):   An unsigned integer that specifies the size, in 
	                       //bytes, of the SttbfBkmkFactoid at offset fcSttbfBkmkFactoid 
	ULONG fcPlcfBkfFactoid;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. A Plcfbkfd that contains information about the smart tag
						   //bookmarks in the document begins at this offset. If lcbPlcfBkfFactoid 
						   //is zero, fcPlcfBkfFactoid is undefined and MUST be ignored.  
						   //Each data element in the Plcfbkfd is associated, in a one-to-one 
						   //correlation, with a data element in the Plcfbkld at offset 
						   //fcPlcfBklFactoid. For this reason, the Plcfbkfd that begins at offset 
						   //fcPlcfBkfFactoid, and the Plcfbkld that begins at offset 
						   //fcPlcfBklFactoid, MUST contain the same number of data elements. The 
						   //Plcfbkfd is parallel to the SttbfBkmkFactoid at offset 
						   //fcSttbfBkmkFactoid in the Table Stream. Each data element in the 
						   //Plcfbkfd specifies information about the bookmark that is associated 
						   //with the element which is located at the same offset in that 
						   //SttbfBkmkFactoid. For this reason, the Plcfbkfd that begins at offset 
						   //fcPlcfBkfFactoid, and the SttbfBkmkFactoid that begins at offset 
						   //fcSttbfBkmkFactoid, MUST contain the same number of elements
	ULONG lcbPlcfBkfFactoid;//(4 bytes):   An unsigned integer that specifies the size, in 
	                       //bytes, of the Plcfbkfd at offset fcPlcfBkfFactoid 
	ULONG fcPlcfcookie; //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A Plcfcookie begins at this offset. If lcbPlcfcookie is zero, 
						   //fcPlcfcookie is undefined and MUST be ignored. fcPlcfcookie MAY be 
						   //ignored 
						   //Office Word 2007, Word 2010, and Word 2013 ignore this value.
	ULONG lcbPlcfcookie;//(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the Plcfcookie at offset fcPlcfcookie in the Table Stream
	ULONG fcPlcfBklFactoid;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. A Plcfbkld that contains information about the smart tag
						   //bookmarks in the document begins at this offset. If lcbPlcfBklFactoid 
						   //is zero, fcPlcfBklFactoid is undefined and MUST be ignored.  
						   //Each data element in the Plcfbkld is associated, in a one-to-one 
						   //correlation, with a data element in the Plcfbkfd at offset 
						   //fcPlcfBkfFactoid. For this reason, the Plcfbkld that begins at offset 
						   //fcPlcfBklFactoid, and the Plcfbkfd that begins at offset 
						   //fcPlcfBkfFactoid, MUST contain the same number of data elements
	ULONG lcbPlcfBklFactoid;//(4 bytes):   An unsigned integer that specifies the size, in 
	                       //bytes, of the Plcfbkld at offset fcPlcfBklFactoid 
	ULONG fcFactoidData;//(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A SmartTagData begins at this offset and specifies information
						   //about the smart tag recognizers that are used in this document. 
						   //If lcbFactoidData is zero, fcFactoidData is undefined and MUST be 
						   //ignored 
	ULONG lcbFactoidData;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the SmartTagData at offset fcFactoidData in the Table Stream
	ULONG fcDocUndo;    //(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //WordDocument Stream. Version-specific undo information begins at this 
						   //offset. This information SHOULD NOT be emitted and SHOULD be ignored 
						   //Word 2002 and Office Word 2003 write this information when the user 
						   //chooses to save versions in the document.  Neither Word 97, Word 2000,
						   //Office Word 2007, Word 2010, nor Word 2013 write this information
						   //Word 2002 and Office Word 2003 read this information.  Word 97, Word 
						   //2000, Office Word 2007, Word 2010, and Word 2013 ignore it
	ULONG lcbDocUndo;   //(4 bytes):   An unsigned integer. If this value is nonzero, 
	                       //version-specific undo information exists at offset fcDocUndo in the 
						   //WordDocument Stream 
	ULONG fcSttbfBkmkFcc;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. An SttbfBkmkFcc that contains information about the 
						   //format consistency-checker bookmarks in the document begins at this 
						   //offset. If lcbSttbfBkmkFcc is zero, fcSttbfBkmkFcc is undefined and 
						   //MUST be ignored.  
						   //The SttbfBkmkFcc is parallel to the Plcfbkfd at offset fcPlcfBkfFcc in
						   //the Table Stream. Each element in the SttbfBkmkFcc specifies 
						   //information about the bookmark that is associated with the data 
						   //element which is located at the same offset in that Plcfbkfd. For this
						   //reason, the SttbfBkmkFcc that begins at offset fcSttbfBkmkFcc, and the
						   //Plcfbkfd that begins at offset fcPlcfBkfFcc, MUST contain the same 
						   //number of elements 
	ULONG lcbSttbfBkmkFcc;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the SttbfBkmkFcc at offset fcSttbfBkmkFcc 
	ULONG fcPlcfBkfFcc; //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A Plcfbkfd that contains information about format 
						   //consistency-checker bookmarks in the document begins at this offset. 
						   //If lcbPlcfBkfFcc is zero, fcPlcfBkfFcc is undefined and MUST be 
						   //ignored 
						   //Each data element in the Plcfbkfd is associated, in a one-to-one 
						   //correlation, with a data element in the Plcfbkld at offset 
						   //fcPlcfBklFcc. For this reason, the Plcfbkfd that begins at offset 
						   //fcPlcfBkfFcc and the Plcfbkld that begins at offset fcPlcfBklFcc MUST 
						   //contain the same number of data elements. The Plcfbkfd is parallel to 
						   //the SttbfBkmkFcc at offset fcSttbfBkmkFcc in the Table Stream. Each 
						   //data element in the Plcfbkfd specifies information about the bookmark 
						   //that is associated with the element which is located at the same 
						   //offset in that SttbfBkmkFcc. For this reason, the Plcfbkfd that begins
						   //at offset fcPlcfBkfFcc and the SttbfBkmkFcc that begins at offset 
						   //fcSttbfBkmkFcc MUST contain the same number of elements
	ULONG lcbPlcfBkfFcc;//(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the Plcfbkfd at offset fcPlcfBkfFcc
	ULONG fcPlcfBklFcc; //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A Plcfbkld that contains information about the format 
						   //consistency-checker bookmarks in the document begins at this offset. 
						   //If lcbPlcfBklFcc is zero, fcPlcfBklFcc is undefined and MUST be 
						   //ignored.  
						   //Each data element in the Plcfbkld is associated, in a one-to-one 
						   //correlation, with a data element in the Plcfbkfd at offset 
						   //fcPlcfBkfFcc. For this reason, the Plcfbkld that begins at offset 
						   //fcPlcfBklFcc, and the Plcfbkfd that begins at offset fcPlcfBkfFcc, 
						   //MUST contain the same number of data elements
	ULONG lcbPlcfBklFcc;//(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the Plcfbkld at offset fcPlcfBklFcc
	ULONG fcSttbfbkmkBPRepairs;//(4 bytes):   An unsigned integer that specifies an offset in 
	                       //the Table Stream. An SttbfBkmkBPRepairs that contains information 
						   //about the repair bookmarks in the document begins at this offset. 
						   //If lcbSttbfBkmkBPRepairs is zero, fcSttbfBkmkBPRepairs is undefined 
						   //and MUST be ignored. 
						   //The SttbfBkmkBPRepairs is parallel to the Plcfbkf at offset 
						   //fcPlcfBkfBPRepairs in the Table Stream. Each element in the 
						   //SttbfBkmkBPRepairs specifies information about the bookmark that is 
						   //associated with the data element which is located at the same offset 
						   //in that Plcfbkf. For this reason, the SttbfBkmkBPRepairs that begins 
						   //at offset fcSttbfBkmkBPRepairs, and the Plcfbkf that begins at offset 
						   //fcPlcfBkfBPRepairs, MUST contain the same number of elements
	ULONG lcbSttbfbkmkBPRepairs;//(4 bytes):   An unsigned integer that specifies the size, in 
	                       //bytes, of the SttbfBkmkBPRepairs at offset fcSttbfBkmkBPRepairs 
	ULONG fcPlcfbkfBPRepairs;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. A Plcfbkf that contains information about the repair 
						   //bookmarks in the document begins at this offset. If 
						   //lcbPlcfBkfBPRepairs is zero, fcPlcfBkfBPRepairs is undefined and MUST 
						   //be ignored.  
						   //Each data element in the Plcfbkf is associated, in a one-to-one 
						   //correlation, with a data element in the Plcfbkl at offset 
						   //fcPlcfBklBPRepairs. For this reason, the Plcfbkf that begins at offset 
						   //fcPlcfBkfBPRepairs, and the Plcfbkl that begins at offset 
						   //fcPlcfBklBPRepairs, MUST contain the same number of data elements. 
						   //The Plcfbkf is parallel to the SttbfBkmkBPRepairs at offset 
						   //fcSttbfBkmkBPRepairs in the Table Stream. Each data element in the 
						   //Plcfbkf specifies information about the bookmark that is associated 
						   //with the element which is located at the same offset in that 
						   //SttbfBkmkBPRepairs. For this reason, the Plcfbkf that begins at offset 
						   //fcPlcfbkfBPRepairs, and the SttbfBkmkBPRepairs that begins at offset 
						   //fcSttbfBkmkBPRepairs, MUST contain the same number of elements. The 
						   //CPs in this Plcfbkf MUST NOT exceed the CP that represents the end of 
						   //the Main Document part 
	ULONG lcbPlcfbkfBPRepairs;//(4 bytes):   An unsigned integer that specifies the size, in 
	                       //bytes, of the Plcfbkf at offset fcPlcfbkfBPRepairs 
	ULONG fcPlcfbklBPRepairs;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. A Plcfbkl that contains information about the repair 
						   //bookmarks in the document begins at this offset. If 
						   //lcbPlcfBklBPRepairs is zero, fcPlcfBklBPRepairs is undefined and MUST 
						   //be ignored. 
						   //Each data element in the Plcfbkl is associated, in a one-to-one 
						   //correlation, with a data element in the Plcfbkf at offset 
						   //fcPlcfBkfBPRepairs. For this reason, the Plcfbkl that begins at offset 
						   //fcPlcfBklBPRepairs, and the Plcfbkf that begins at offset 
						   //fcPlcfBkfBPRepairs, MUST contain the same number of data elements.  
						   //The CPs that are contained in this Plcfbkl MUST NOT exceed the CP that
						   //represents the end of the Main Document part 
	ULONG lcbPlcfbklBPRepairs;//(4 bytes):   An unsigned integer that specifies the size, in 
	                       //bytes, of the Plcfbkl at offset fcPlcfBklBPRepairs 
	ULONG fcPmsNew;     //(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. A new Pms, which contains the current state of a print 
						   //merge operation, begins at this offset. If lcbPmsNew is zero, 
						   //fcPmsNew is undefined and MUST be ignored 
	ULONG lcbPmsNew;    //(4 bytes):   An unsigned integer which specifies the size, in bytes, 
	                       //of the Pms at offset fcPmsNew. 
	ULONG fcODSO;       //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. Office Data Source Object (ODSO) data that is used to perform 
						   //mail merge begins at this offset. The data is stored in an array of 
						   //ODSOPropertyBase items. The ODSOPropertyBase items are of variable 
						   //size and are stored contiguously. The complete set of properties that
						   //are contained in the array is determined by reading each 
						   //ODSOPropertyBase, until a total of lcbODSO bytes of data are read. If 
						   //lcbODSO is zero, fcODSO is undefined and MUST be ignored 
	ULONG lcbODSO;      //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the Office Data Source Object data at offset fcODSO in the Table 
						   //Stream
	ULONG fcPlcfpmiOldXP;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. The deprecated paragraph mark information cache begins 
						   //at this offset. Information SHOULD NOT be emitted at this offset and 
						   //SHOULD be ignored. If lcbPlcfpmiOldXP is zero, fcPlcfpmiOldXP is 
						   //undefined and MUST be ignored
						   //Word 2002 emits information at offset fcPlcfpmiOldXP. Neither Word 97,
						   //Word 2000, Office Word 2003, Office Word 2007, Word 2010, nor Word 
						   //2013 emit information at this offset and the value of fcPlcfpmiOldXP 
						   //is undefined
						   //Word 2002 reads this information. Word 97, Word 2000, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore it
	ULONG lcbPlcfpmiOldXP;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the deprecated paragraph mark information cache at offset 
						   //fcPlcfpmiOldXP in the Table Stream. This value SHOULD be zero. 
						   //Word 2002 writes lcbPlcfpmiOldXP with the size, in bytes, of the 
						   //information emitted at offset fcPlcfpmiOldXP. Office Word 2003, Office
						   //Word 2007, Word 2010, and Word 2013 write 0 to lcbPlcfpmiOldXP. 
						   //Neither Word 97 nor Word 2000 write a FibRgFcLcb2002.
	ULONG fcPlcfpmiNewXP;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. The deprecated paragraph mark information cache begins 
						   //at this offset. Information SHOULD NOT be emitted at this offset and 
						   //SHOULD be ignored. If lcbPlcfpmiNewXP is zero, fcPlcfpmiNewXP is 
						   //undefined and MUST be ignored 
						   //Word 2002 emits information at offset fcPlcfpmiNewXP. Neither Word 97, 
						   //Word 2000, Office Word 2003, Office Word 2007, Word 2010, nor Word 
						   //2013 emit information at this offset and the value of fcPlcfpmiNewXP 
						   //is undefined
						   //Word 2002 reads this information. Word 97, Word 2000, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore it
	ULONG lcbPlcfpmiNewXP;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the deprecated paragraph mark information cache at offset 
						   //fcPlcfpmiNewXP in the Table Stream. This value SHOULD be zero
						   //Word 2002 writes lcbPlcfpmiNewXP with the size, in bytes, of the 
						   //information emitted at offset fcPlcfpmiNewXP. Office Word 2003, Office
						   //Word 2007, Word 2010, and Word 2013 write 0 to lcbPlcfpmiNewXP. 
						   //Neither Word 97 nor Word 2000 write a FibRgFcLcb2002.
	ULONG fcPlcfpmiMixedXP;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. The deprecated paragraph mark information cache begins 
						   //at this offset. Information SHOULD NOT be emitted at this offset and 
						   //SHOULD be ignored. If lcbPlcfpmiMixedXP is zero, fcPlcfpmiMixedXP is 
						   //undefined and MUST be ignored 
						   //Word 2002 emits information at offset fcPlcfpmiMixedXP. Neither Word 
						   //97, Word 2000, Office Word 2003, Office Word 2007, Word 2010, nor Word
						   //2013 emit information at this offset and the value of fcPlcfpmiMixedXP
						   //is undefined
						   //Word 2002 reads this information. Word 97, Word 2000, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore it
	ULONG lcbPlcfpmiMixedXP;//(4 bytes):   An unsigned integer that specifies the size, in 
	                       //bytes, of the deprecated paragraph mark information cache at offset 
						   //fcPlcfpmiMixedXP in the Table Stream. This value SHOULD be zero 
						   //Word 2002 writes lcbPlcfpmiMixedXP with the size, in bytes, of the 
						   //information emitted at offset fcPlcfpmiMixedXP. Office Word 2003, 
						   //Office Word 2007, Word 2010, and Word 2013 write 0 to 
						   //lcbPlcfpmiMixedXP. Neither Word 97 nor Word 2000 write a 
						   //FibRgFcLcb2002.
	ULONG fcUnused2;    //(4 bytes):   This value is undefined and MUST be ignored. 
	ULONG lcbUnused2;   //(4 bytes):   This value MUST be zero, and MUST be ignored. 
	ULONG fcPlcffactoid;//(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A Plcffactoid, which specifies the smart tag recognizer state 
						   //of each text range, begins at this offset. If lcbPlcffactoid is zero, 
						   //fcPlcffactoid is undefined and MUST be ignored. 
	ULONG lcbPlcffactoid;//(4 bytes):   An unsigned integer that specifies the size, in bytes of
						   //the Plcffactoid that begins at offset fcPlcffactoid in the Table 
						   //Stream.
	ULONG fcPlcflvcOldXP;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. The deprecated listnum field cache begins at this 
						   //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						   //ignored. If lcbPlcflvcOldXP is zero, fcPlcflvcOldXP is undefined and 
						   //MUST be ignored
						   //Word 2002 emits information at offset fcPlcflvcOldXP. Neither Word 97,
						   //Word 2000, Office Word 2003, Office Word 2007, Word 2010, nor Word 
						   //2013 emit information at this offset and the value of fcPlcflvcOldXP 
						   //is undefined.
						   //Word 2002 reads this information. Word 97, Word 2000, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore it
	ULONG lcbPlcflvcOldXP;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the deprecated listnum field cache at offset fcPlcflvcOldXP in the 
						   //Table Stream. This value SHOULD be zero 
						   //Word 2002 writes lcbPlcflvcOldXP with the size, in bytes, of the 
						   //information emitted at offset fcPlcflvcOldXP. Office Word 2003, Office
						   //Word 2007, Word 2010, and Word 2013 write 0 to lcbPlcflvcOldXP. 
						   //Neither Word 97 nor Word 2000 write a FibRgFcLcb2002.
	ULONG fcPlcflvcNewXP;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. The deprecated listnum field cache begins at this 
						   //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						   //ignored. If lcbPlcflvcNewXP is zero, fcPlcflvcNewXP is undefined and 
						   //MUST be ignored. 
						   //Word 2002 emits information at offset fcPlcflvcNewXP. Neither Word 97,
						   //Word 2000, Office Word 2003, Office Word 2007, Word 2010, nor Word 
						   //2013 emit information at this offset and the value of fcPlcflvcNewXP 
						   //is undefined
						   //Word 2002 reads this information. Word 97, Word 2000, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore it
	ULONG lcbPlcflvcNewXP;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the deprecated listnum field cache at offset fcPlcflvcNewXP in the 
						   //Table Stream. This value SHOULD be zero. 
						   //Word 2002 writes lcbPlcflvcNewXP with the size, in bytes, of the 
						   //information emitted at offset fcPlcflvcNewXP. Office Word 2003, Office
						   //Word 2007, Word 2010, and Word 2013 write 0 to lcbPlcflvcNewXP. 
						   //Neither Word 97 nor Word 2000 write a FibRgFcLcb2002.
	ULONG fcPlcflvcMixedXP;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. The deprecated listnum field cache begins at this 
						   //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						   //ignored. If lcbPlcflvcMixedXP is zero, fcPlcflvcMixedXP is undefined 
						   //and MUST be ignored 
						   //Word 2002 emits information at offset fcPlcflvcMixedXP. Neither Word 
						   //97, Word 2000, Office Word 2003, Office Word 2007, Word 2010, nor Word
						   //2013 emit information at this offset and the value of fcPlcflvcMixedXP
						   //is undefined
						   //Word 2002 reads this information. Word 97, Word 2000, Office Word 
						   //2003, Office Word 2007, Word 2010, and Word 2013 ignore it
	ULONG lcbPlcflvcMixedXP;//(4 bytes):   An unsigned integer that specifies the size, in 
	                       //bytes, of the deprecated listnum field cache at offset 
						   //fcPlcflvcMixedXP in the Table Stream. This value SHOULD be zero 
						   //Word 2002 writes lcbPlcflvcMixedXP with the size, in bytes, of the 
						   //information emitted at offset fcPlcflvcMixedXP. Office Word 2003, 
						   //Office Word 2007, Word 2010, and Word 2013 write 0 to 
						   //lcbPlcflvcMixedXP. Neither Word 97 nor Word 2000 write a 
						   //FibRgFcLcb2002.
} FibRgFcLcb2002;

/*
 * FibRgFcLcb2003
 * The FibRgFcLcb2003 structure is a variable-sized portion of the Fib.  It extends the 
 * FibRgFcLcb2002.
 */
typedef struct FibRgFcLcb2003 
{
	FibRgFcLcb2002 rgFcLcb2002;//(1088 bytes): The contained FibRgFcLcb2002.
	ULONG fcHplxsdr;    //(4 bytes):   An unsigned integer that specifies an offset in the Table 
	                       //Stream. An Hplxsdr structure begins at this offset. This structure 
						   //specifies information about XML schema definition (XSD) references 
	ULONG lcbHplxsdr;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, of 
	                       //the Hplxsdr structure at the offset fcHplxsdr in the Table Stream. If 
						   //lcbHplxsdr is zero, then fcHplxsdr is undefined and MUST be ignored 
	ULONG fcSttbfBkmkSdt;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. An SttbfBkmkSdt that contains information about the 
						   //structured document tag bookmarks in the document begins at this 
						   //offset. If lcbSttbfBkmkSdt is zero, then fcSttbfBkmkSdt is undefined 
						   //and MUST be ignored.  The SttbfBkmkSdt is parallel to the Plcbkfd at 
						   //offset fcPlcfBkfSdt in the Table Stream. Each element in the 
						   //SttbfBkmkSdt specifies information about the bookmark that is 
						   //associated with the data element which is located at the same offset in
						   //that Plcbkfd. For this reason, the SttbfBkmkSdt that begins at offset 
						   //fcSttbfBkmkSdt, and the Plcbkfd that begins at offset fcPlcfBkfSdt, 
						   //MUST contain the same number of elements 
	ULONG lcbSttbfBkmkSdt;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the SttbfBkmkSdt at offset fcSttbfBkmkSdt 
	ULONG fcPlcfBkfSdt; //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A Plcbkfd that contains information about the structured 
						   //document tag bookmarks in the document begins at this offset. If 
						   //lcbPlcfBkfSdt is zero, fcPlcfBkfSdt is undefined and MUST be ignored.  
						   //Each data element in the Plcbkfd is associated, in a one-to-one 
						   //correlation, with a data element in the Plcbkld at offset 
						   //fcPlcfBklSdt. For this reason, the Plcbkfd that begins at offset 
						   //fcPlcfBkfSdt, and the Plcbkld that begins at offset fcPlcfBklSdt, MUST
						   //contain the same number of data elements. The Plcbkfd is parallel to 
						   //the SttbfBkmkSdt at offset fcSttbfBkmkSdt in the Table Stream. Each 
						   //data element in the Plcbkfd specifies information about the bookmark 
						   //that is associated with the element which is located at the same 
						   //offset in that SttbfBkmkSdt. For this reason, the Plcbkfd that begins
						   //at offset fcPlcfBkfSdt, and the SttbfBkmkSdt that begins at offset 
						   //fcSttbfBkmkSdt, MUST contain the same number of elements 
	ULONG lcbPlcfBkfSdt;//(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the Plcbkfd at offset fcPlcfBkfSdt.
	ULONG fcPlcfBklSdt; //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A Plcbkld that contains information about the structured 
						   //document tag bookmarks in the document begins at this offset. If 
						   //lcbPlcfBklSdt is zero, fcPlcfBklSdt is undefined and MUST be ignored.  
						   //Each data element in the Plcbkld is associated, in a one-to-one 
						   //correlation, with a data element in the Plcbkfd at offset 
						   //fcPlcfBkfSdt. For this reason, the Plcbkld that begins at offset 
						   //fcPlcfBklSdt, and the Plcbkfd that begins at offset fcPlcfBkfSdt MUST 
						   //contain the same number of data elements.
	ULONG lcbPlcfBklSdt;//(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the Plcbkld at offset fcPlcfBklSdt.
	ULONG fcCustomXForm;//(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. An array of 16-bit Unicode characters, which specifies the 
						   //full path and file name of the XML Stylesheet to apply when saving 
						   //this document in XML format, begins at this offset. If lcbCustomXForm 
						   //is zero, fcCustomXForm is undefined and MUST be ignored.
	ULONG lcbCustomXForm;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the array at offset fcCustomXForm in the Table Stream. This value 
						   //MUST be less than or equal to 4168 and MUST be evenly divisible by 
						   //two.
	ULONG fcSttbfBkmkProt;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. An SttbfBkmkProt that contains information about 
						   //range-level protection bookmarks in the document begins at this 
						   //offset. If lcbSttbfBkmkProt is zero, fcSttbfBkmkProt is undefined and 
						   //MUST be ignored.  
						   //The SttbfBkmkProt is parallel to the Plcbkf at offset fcPlcfBkfProt 
						   //in the Table Stream. Each element in the SttbfBkmkProt specifies 
						   //information about the bookmark that is associated with the data 
						   //element which is located at the same offset in that Plcbkf. For this 
						   //reason, the SttbfBkmkProt that begins at offset fcSttbfBkmkProt, and 
						   //the Plcbkf that begins at offset fcPlcfBkfProt, MUST contain the same 
						   //number of elements. 
	ULONG lcbSttbfBkmkProt;//(4 bytes):   An unsigned integer that specifies the size, in bytes,
                           //of the SttbfBkmkProt at offset fcSttbfBkmkProt. 
	ULONG fcPlcfBkfProt;//(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. A Plcbkf that contains information about range-level 
						   //protection bookmarks in the document begins at this offset. If 
						   //lcbPlcfBkfProt is zero, then fcPlcfBkfProt is undefined and MUST be 
						   //ignored. 
						   //Each data element in the Plcbkf is associated, in a one-to-one 
						   //correlation, with a data element in the Plcbkl at offset 
						   //fcPlcfBklProt. For this reason, the Plcbkf that begins at offset 
						   //fcPlcfBkfProt, and the Plcbkl that begins at offset fcPlcfBklProt, 
						   //MUST contain the same number of data elements. The Plcbkf is parallel 
						   //to the SttbfBkmkProt at offset fcSttbfBkmkProt in the Table Stream. 
						   //Each data element in the Plcbkf specifies information about the 
						   //bookmark that is associated with the element which is located at the 
						   //same offset in that SttbfBkmkProt. For this reason, the Plcbkf that 
						   //begins at offset fcPlcfBkfProt, and the SttbfBkmkProt that begins at 
						   //offset fcSttbfBkmkProt, MUST contain the same number of elements.
	ULONG lcbPlcfBkfProt;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the Plcbkf at offset fcPlcfBkfProt.
	ULONG fcPlcfBklProt;//(4 bytes):   An unsigned integer that specifies an offset in the Table
						   //Stream. A Plcbkl containing information about range-level protection 
						   //bookmarks in the document begins at this offset. If lcbPlcfBklProt is 
						   //zero, then fcPlcfBklProt is undefined and MUST be ignored.  
						   //Each data element in the Plcbkl is associated in a one-to-one 
						   //correlation with a data element in the Plcbkf at offset fcPlcfBkfProt,
						   //so the Plcbkl beginning at offset fcPlcfBklProt and the Plcbkf 
						   //beginning at offset fcPlcfBkfProt MUST contain the same number of data
						   //elements.
	ULONG lcbPlcfBklProt;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the Plcbkl at offset fcPlcfBklProt.
	ULONG fcSttbProtUser;//(4 bytes):   An unsigned integer that specifies an offset in the 
	                       //Table Stream. A SttbProtUser that specifies the usernames that are 
						   //used for range-level protection begins at this offset. 
	ULONG lcbSttbProtUser;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
	                       //of the SttbProtUser at the offset fcSttbProtUser. 
	ULONG fcUnused;     //(4 bytes):   This value is undefined and MUST be ignored. 
	ULONG lcbUnused;    //(4 bytes):   This value MUST be zero, and MUST be ignored. 
	ULONG fcPlcfpmiOld; //(4 bytes):   An unsigned integer that specifies an offset in the Table
                           //Stream. Deprecated paragraph mark information cache begins at this 
						   //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						   //ignored. If lcbPlcfpmiOld is zero, then fcPlcfpmiOld is undefined and 
						   //MUST be ignored. 
						   //Only Office Word 2003 emits information at offset fcPlcfpmiOld; 
						   //Neither Office Word 2007, Word 2010, nor Word 2013 emit information 
						   //at this offset and the value of fcPlcfpmiOld is undefined
						   //Only Office Word 2003 reads this information.
	ULONG lcbPlcfpmiOld;//(4 bytes):   An unsigned integer that specifies the size, in bytes, of
                           //the deprecated paragraph mark information cache at offset fcPlcfpmiOld
						   //in the Table Stream. SHOULD be zero.
						   //Office Word 2003 writes lcbPlcfpmiOld with the size, in bytes, of the 
						   //information emitted at offset fcPlcfpmiOld; Office Word 2007, Word 
						   //2010, and Word 2013 write 0 to lcbPlcfpmiOld.
	ULONG fcPlcfpmiOldInline;//(4 bytes):   An unsigned integer that specifies an offset in the
						   //Table Stream. Deprecated paragraph mark information cache begins at 
						   //this offset. Information SHOULD NOT be emitted at this offset and 
						   //SHOULD be ignored. If lcbPlcfpmiOldInline is zero, then 
						   //fcPlcfpmiOldInline is undefined and MUST be ignored.
						   //Only Office Word 2003 emits information at offset fcPlcfpmiOldInline;
						   //Neither Office Word 2007, Word 2010, nor Word 2013 emit information 
						   //at this offset and the value of fcPlcfpmiOldInline is undefined
						   //Only Office Word 2003 reads this information
	ULONG lcbPlcfpmiOldInline;//(4 bytes):   An unsigned integer that specifies the size, in 
						   //bytes, of the deprecated paragraph mark information cache at offset 
						   //fcPlcfpmiOldInline in the Table Stream. SHOULD be zero
						   //Office Word 2003 writes lcbPlcfpmiOldInline with the size, in bytes, 
						   //of the information emitted at offset fcPlcfpmiOldInline; Office Word 
						   //2007, Word 2010, and Word 2013 write 0 to lcbPlcfpmiOldInline
	ULONG fcPlcfpmiNew; //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. Deprecated paragraph mark information cache begins at 
						   //this offset. Information SHOULD NOT be emitted at this offset and 
						   //SHOULD be ignored. If lcbPlcfpmiNew is zero, then fcPlcfpmiNew is 
						   //undefined and MUST be ignored. 
						   //Only Office Word 2003 emits information at offset fcPlcfpmiNew; 
						   //Neither Office Word 2007, Word 2010, nor Word 2013 emit information 
						   //at this offset and the value of fcPlcfpmiNew is undefined
						   //Only Office Word 2003 reads this information
	ULONG lcbPlcfpmiNew;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the deprecated paragraph mark information cache at offset 
						   //fcPlcfpmiNew in the Table Stream. SHOULD be zero. 
						   //Office Word 2003 writes lcbPlcfpmiNew with the size, in bytes, of the
						   //information emitted at offset fcPlcfpmiNew; Office Word 2007, Word 
						   //2010, and Word 2013 write 0 to lcbPlcfpmiNew
	ULONG fcPlcfpmiNewInline;//(4 bytes):   An unsigned integer that specifies an offset in the
						   //Table Stream. Deprecated paragraph mark information cache begins at 
						   //this offset. Information SHOULD NOT be emitted at this offset and
						   //SHOULD be ignored. If lcbPlcfpmiNewInline is zero, then 
						   //fcPlcfpmiNewInline is undefined and MUST be ignored 
						   //Only Office Word 2003 emits information at offset fcPlcfpmiNewInline;
						   //Neither Office Word 2007, Word 2010, nor Word 2013 emit information
						   //at this offset and the value of fcPlcfpmiNewInline is undefined
						   //Only Office Word 2003 reads this information
	ULONG lcbPlcfpmiNewInline;//(4 bytes):   An unsigned integer that specifies the size, in 
						   //bytes, of the deprecated paragraph mark information cache at offset 
						   //fcPlcfpmiNewInline in the Table Stream. SHOULD be zero.
						   //Office Word 2003 writes lcbPlcfpmiNewInline with the size, in bytes, 
						   //of the information emitted at offset fcPlcfpmiNewInline; Office Word 
						   //2007, Word 2010, and Word 2013 write 0 to lcbPlcfpmiNewInline
	ULONG fcPlcflvcOld; //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. Deprecated listnum field cache begins at this offset. 
						   //Information SHOULD NOT be emitted at this offset and SHOULD be 
						   //ignored. If lcbPlcflvcOld is zero, then fcPlcflvcOld is undefined 
						   //and MUST be ignored
						   //Only Office Word 2003 emits information at offset fcPlcflvcOld; 
						   //Neither Office Word 2007, Word 2010, nor Word 2013 emit information 
						   //at this offset and the value of fcPlcflvcOld is undefined.
						   //Only Office Word 2003 reads this information
	ULONG lcbPlcflvcOld;//(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the deprecated listnum field cache at offset fcPlcflvcOld in the 
						   //Table Stream. SHOULD be zero 
						   //Office Word 2003 writes lcbPlcflvcOld with the size, in bytes, of the
						   //information emitted at offset fcPlcflvcOld; Office Word 2007, Word 
						   //2010, and Word 2013 write 0 to lcbPlcflvcOld
	ULONG fcPlcflvcOldInline;//(4 bytes):   An unsigned integer that specifies an offset in the
						   //Table Stream. Deprecated listnum field cache begins at this offset. 
						   //Information SHOULD NOT be emitted at this offset and SHOULD be 
						   //ignored. If lcbPlcflvcOldInline is zero, fcPlcflvcOldInline is 
						   //undefined and MUST be ignored 
						   //Only Office Word 2003 emits information at offset fcPlcflvcOldInline;
						   //Neither Office Word 2007, Word 2010, nor Word 2013 emit information 
						   //at this offset and the value of fcPlcflvcOldInline is undefined
						   //Only Office Word 2003 reads this information
	ULONG lcbPlcflvcOldInline;//(4 bytes):   An unsigned integer that specifies the size, in 
						   //bytes, of the deprecated listnum field cache at offset 
						   //fcPlcflvcOldInline in the Table Stream. SHOULD be zero
						   //Office Word 2003 writes lcbPlcflvcOldInline with the size, in bytes,
						   //of the information emitted at offset fcPlcflvcOldInline; Office Word
						   //2007, Word 2010, and Word 2013 write 0 to lcbPlcflvcOldInline
	ULONG fcPlcflvcNew; //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. Deprecated listnum field cache begins at this offset.
						   //Information SHOULD NOT be emitted at this offset and SHOULD be 
						   //ignored. If lcbPlcflvcNew is zero, fcPlcflvcNew is undefined and 
						   //MUST be ignored 
						   //Only Office Word 2003 emits information at offset fcPlcflvcNew; 
						   //Neither Office Word 2007, Word 2010, nor Word 2013 emit information 
						   //at this offset and the value of fcPlcflvcNew is undefined
						   //Only Office Word 2003 reads this information
	ULONG lcbPlcflvcNew; //(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the deprecated listnum field cache at offset fcPlcflvcNew in the 
						   //Table Stream. SHOULD be zero 
						   //Office Word 2003 writes lcbPlcflvcNew with the size, in bytes, of the
						   //information emitted at offset fcPlcflvcNew; Office Word 2007, Word 
						   //2010, and Word 2013 write 0 to lcbPlcflvcNew
	ULONG fcPlcflvcNewInline; //(4 bytes):   An unsigned integer that specifies an offset in 
						   //the Table Stream. Deprecated listnum field cache begins at this 
						   //offset. Information SHOULD NOT be emitted at this offset and SHOULD 
						   //be ignored. If lcbPlcflvcNewInline is zero, fcPlcflvcNewInline is 
						   //undefined and MUST be ignored 
						   //Only Office Word 2003 emits information at offset fcPlcflvcNewInline;
						   //Neither Office Word 2007, Word 2010, nor Word 2013 emit information
						   //at this offset and the value of fcPlcflvcNewInline is undefined
						   //Only Office Word 2003 reads this information
	ULONG lcbPlcflvcNewInline; //(4 bytes):   An unsigned integer that specifies the size, in 
						   //bytes, of the deprecated listnum field cache at offset 
						   //fcPlcflvcNewInline in the Table Stream. SHOULD be zero 
						   //Office Word 2003 writes lcbPlcflvcNewInline with the size, in bytes, 
						   //of the information emitted at offset fcPlcflvcNewInline; Office Word 
						   //2007, Word 2010, and Word 2013 write 0 to lcbPlcflvcNewInline
	ULONG fcPgdMother;  //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. Deprecated document page layout cache begins at this 
						   //offset. Information SHOULD NOT be emitted at this offset and 
						   //SHOULD be ignored. If lcbPgdMother is zero, fcPgdMother is 
						   //undefined and MUST be ignored 
						   //Office Word 2003 emits information at offset fcPgdMother. Neither 
						   //Word 97, Word 2000, Office Word 2003, Office Word 2007, Word 2010, 
						   //nor Word 2013 emit this information
						   //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						   //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						   //information
	ULONG lcbPgdMother; //(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the deprecated document page layout cache at offset fcPgdMother 
						   //in the Table Stream 
	ULONG fcBkdMother;  //(4 bytes):   An unsigned integer that specifies an offset in the 
						   //Table Stream. Deprecated document text flow break cache begins at 
						   //this offset. Information SHOULD NOT be emitted at this offset and 
						   //SHOULD be ignored. If lcbBkdMother is zero, then fcBkdMother is 
						   //undefined and MUST be ignored 
						   //Office Word 2003 emits information at offset fcBkdMother. Neither 
						   //Word 97, Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 
						   //2013 emit this information
						   //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						   //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						   //information
	ULONG lcbBkdMother; //(4 bytes):   An unsigned integer that specifies the size, in bytes, 
						   //of the deprecated document text flow break cache at offset 
						   //fcBkdMother in the Table Stream 
	ULONG fcAfdMother; //(4 bytes):   An unsigned integer that specifies an offset in the Table
						  //Stream. Deprecated document author filter cache begins at this offset.
						  //Information SHOULD NOT be emitted at this offset and SHOULD be 
						  //ignored. If lcbAfdMother is zero, then fcAfdMother is undefined and 
						  //MUST be ignored 
						  //Office Word 2003 emits information at offset fcAfdMother. Neither Word
						  //97, Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 2013 
						  //emit this information
						  //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						  //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						  //information
	ULONG lcbAfdMother;//(4 bytes):   An unsigned integer that specifies the size, in bytes, of
						  //the deprecated document author filter cache at offset fcAfdMother in 
						  //the Table Stream
	ULONG fcPgdFtn;    //(4 bytes):   An unsigned integer that specifies an offset in the Table
						  //Stream. Deprecated footnote layout cache begins at this offset. 
						  //Information SHOULD NOT be emitted at this offset and SHOULD be 
						  //ignored. If lcbPgdFtn is zero, then fcPgdFtn is undefined and MUST be 
						  //ignored
						  //Office Word 2003 emits information at offset fcPgdFtn. Neither Word 
						  //97, Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 2013 
						  //emit this information
						  //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						  //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						  //information
	ULONG lcbPgdFtn;   //(4 bytes):   unsigned integer that specifies the size, in bytes, of 
						  //the deprecated footnote layout cache at offset fcPgdFtn in the Table 
						  //Stream
	ULONG fcBkdFtn;    //(4 bytes):   An unsigned integer that specifies an offset in the Table
						  //Stream. The deprecated footnote text flow break cache begins at this
						  //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						  //ignored. If lcbBkdFtn is zero, fcBkdFtn is undefined and MUST be 
						  //ignored 
						  //Office Word 2003 emits information at offset fcBkdFtn. Neither Word 
						  //97, Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 2013 
						  //emit this information
						  //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						  //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						  //information
	ULONG lcbBkdFtn;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
						  //the deprecated footnote text flow break cache at offset fcBkdFtn in 
						  //the Table Stream
	ULONG fcAfdFtn;    //(4 bytes):   An unsigned integer that specifies an offset in the Table
						  //Stream. The deprecated footnote author filter cache begins at this 
						  //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						  //ignored. If lcbAfdFtn is zero, fcAfdFtn is undefined and MUST be 
						  //ignored
						  //Office Word 2003 emits information at offset fcAfdFtn. Neither Word 
						  //97, Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 2013 
						  //emit this information
						  //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						  //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						  //information
	ULONG lcbAfdFtn;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
						  //the deprecated footnote author filter cache at offset fcAfdFtn in the
						  //Table Stream
	ULONG fcPgdEdn;    //(4 bytes):   An unsigned integer that specifies an offset in the Table
						  //Stream. The deprecated endnote layout cache begins at this offset. 
						  //Information SHOULD NOT be emitted at this offset and SHOULD be 
						  //ignored. If lcbPgdEdn is zero, then fcPgdEdn is undefined and MUST be 
						  //ignored
						  //Office Word 2003 emits information at offset fcPgdEdn. Neither Word 
						  //97, Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 2013 
						  //emit this information
						  //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						  //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						  //information
	ULONG lcbPgdEdn;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
						  //the deprecated endnote layout cache at offset fcPgdEdn in the Table 
						  //Stream
	ULONG fcBkdEdn;    //(4 bytes):   An unsigned integer that specifies an offset in the Table
						  //Stream. The deprecated endnote text flow break cache begins at this 
						  //offset. Information SHOULD NOT be emitted at this offset and SHOULD be
						  //ignored. If lcbBkdEdn is zero, fcBkdEdn is undefined and MUST be 
						  //ignored
						  //Office Word 2003 emits information at offset fcBkdEdn. Neither Word 
						  //97, Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 2013 
						  //emit this information
						  //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						  //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						  //information
	ULONG lcbBkdEdn;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
						  //the deprecated endnote text flow break cache at offset fcBkdEdn in the
						  //Table Stream
	ULONG fcAfdEdn;    //(4 bytes):   An unsigned integer that specifies an offset in the Table
						  //Stream. Deprecated endnote author filter cache begins at this offset.
						  //Information SHOULD NOT be emitted at this offset and SHOULD be 
						  //ignored. If lcbAfdEdn is zero, then fcAfdEdn is undefined and MUST be 
						  //ignored
						  //Office Word 2003 emits information at offset fcAfdEdn. Neither Word 
						  //97, Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 2013 
						  //emit this information
						  //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						  //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						  //information
	ULONG lcbAfdEdn;   //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
						  //the deprecated endnote author filter cache at offset fcAfdEdn in the
						  //Table Stream
	ULONG fcAfd;       //(4 bytes):   An unsigned integer that specifies an offset in the Table
						  //Stream. A deprecated AFD structure begins at this offset. Information
						  //SHOULD NOT be emitted at this offset and SHOULD be ignored. If lcbAfd
						  //is zero, fcAfd is undefined and MUST be ignored
						  //Office Word 2003 emits information at offset fcAfd. Neither Word 97, 
						  //Word 2000, Word 2002, Office Word 2007, Word 2010, nor Word 2013 emit 
						  //information at this offset
						  //Office Word 2003 reads this information. Word 97, Word 2000, Word 
						  //2002, Office Word 2007, Word 2010, and Word 2013 ignore this 
						  //information
	ULONG lcbAfd;      //(4 bytes):   An unsigned integer that specifies the size, in bytes, of
						  //the deprecated AFD structure at offset fcAfd in the Table Stream
} FibRgFcLcb2003;

/*
 * FibRgFcLcb2007
 * The FibRgFcLcb2007 structure is a variable-sized portion of the Fib. It extends the 
 * FibRgFcLcb2003.
 */
typedef struct FibRgFcLcb2007 
{
	FibRgFcLcb2003 rgFcLcb2003;//(1312 bytes): The contained FibRgFcLcb2003.
	ULONG fcPlcfmthd;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbPlcfmthd; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcSttbfBkmkMoveFrom;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbSttbfBkmkMoveFrom; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcPlcfBkfMoveFrom;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbPlcfBkfMoveFrom; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcPlcfBklMoveFrom;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbPlcfBklMoveFrom; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcSttbfBkmkMoveTo;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbSttbfBkmkMoveTo; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcPlcfBkfMoveTo;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbPlcfBkfMoveTo; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcPlcfBklMoveTo;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbPlcfBklMoveTo; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcUnused1;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbUnused1; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcUnused2;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbUnused2; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcUnused3;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbUnused3; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcSttbfBkmkArto;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbSttbfBkmkArto; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcPlcfBkfArto;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbPlcfBkfArto; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcPlcfBklArto;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbPlcfBklArto; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcArtoData;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbArtoData; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcUnused4;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbUnused4; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcUnused5;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbUnused5; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcUnused6;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbUnused6; //(4 bytes):   This value MUST be zero, and MUST be ignored
	ULONG fcOssTheme;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbOssTheme; //(4 bytes):   This value MUST be zero, and MUST be ignored
						  //Neither Office Word 2007, Word 2010, nor Word 2013 write 0 here, but 
						  //all three ignore this value when loading files
	ULONG fcColorSchemeMapping;  //(4 bytes):   This value is undefined and MUST be ignored
	ULONG lcbColorSchemeMapping; //(4 bytes):   This value MUST be zero, and MUST be ignored
						  //Neither Office Word 2007, Word 2010, nor Word 2013 write 0 here, but 
						  //all three ignore this value when loading files

} FibRgFcLcb2007;

/*
 * FibRgCswNew
 * The FibRgCswNew structure is an extension to the Fib structure that exists only if Fib.cswNew 
 * is nonzero.
 */
typedef struct FibRgCswNew 
{
	USHORT nFibNew;//(2 bytes): An unsigned integer that specifies the version number of the 
					      //file format that is used. This value MUST be one of the following
					      //0x00D9, 0x0101, 0x010C, 0x0112
	USHORT rgCswNewData[4];
} FibRgCswNew;

enum rgCswNewData_t {
	FibRgCswNewData2000_t,
	FibRgCswNewData2007_t,
};

struct nFibNew2rgCswNewData {
	USHORT nFibNew;
	enum rgCswNewData_t rgCswNewData;
};

static const struct nFibNew2rgCswNewData nFibNew2rgCswNewDataTable[] = {
	{0x00D9, FibRgCswNewData2000_t}, 
	{0x0101, FibRgCswNewData2000_t}, 
	{0x010C, FibRgCswNewData2000_t}, 
	{0x0112, FibRgCswNewData2007_t}, 
};

static enum rgCswNewData_t rgCswNewData_get(USHORT nFibNew){
	if (nFibNew == 0x0112)
		return FibRgCswNewData2007_t;
	return FibRgCswNewData2000_t;
}

/*
 * FibRgCswNewData2000
 * The FibRgCswNewData2000 structure is a variable-sized portion of the Fib.
 */
typedef struct FibRgCswNewData2000 
{
	USHORT cQuickSavesNew;//(2 bytes): An unsigned integer that specifies the number of times 
					      //that this document was incrementally saved since the last full save. 
						  //This value MUST be between 0 and 0x000F, inclusively 
} FibRgCswNewData2000;

/*
 * FibRgCswNewData2007
 * The FibRgCswNewData2007 structure is a variable-sized portion of the Fib. It extends the 
 * FibRgCswNewData2000.
 */
typedef struct FibRgCswNewData2007 
{
	FibRgCswNewData2000 rgCswNewData2000;//The contained FibRgCswNewData2000.
	USHORT lidThemeOther;//(2 bytes): This value is undefined and MUST be ignored 
	USHORT lidThemeFE;   //(2 bytes): This value is undefined and MUST be ignored 
	USHORT lidThemeCS;   //(2 bytes): This value is undefined and MUST be ignored 
} FibRgCswNewData2007;

/*
 * Sprm
 * The Sprm structure specifies a modification to a property
 * of a character, paragraph, table, or section. */ 
typedef USHORT Sprm;
							//ispmd (9 bits): An unsigned 
							//integer that, when combined 
							//with fSpec, specifies the property being 
							//modified. See 
							//the tables in the Single Property 
							//Modifiers section (2.6) 
							//for the complete list of valid ispmd, 
							//fSpec, spra 
							//combinations for each sgc. 
							// 
							// A - fSpec (1 bit): When combined with 
							// ispmd, specifies the 
							// property being modified. See the tables 
							// in the Single 
							// Property Modifiers section (2.6) for the 
							// complete list of 
							// valid ispmd, fSpec, spra combinations for 
							// each sgc.
							// 
							// sgc (3 bits): An unsigned integer that 
							// specifies the kind of
							// document content to which this Sprm applies. 
							// The following 
							// table specifies the valid values and 
							// their meanings.
							// Sgc: Meaning
							// 1:    Sprm is modifying a paragraph property.
							// 2:    Sprm is modifying a character property.
							// 3:    Sprm is modifying a picture property.
							// 4:    Sprm is modifying a section property.
							// 5:    Sprm is modifying a table property.
							// 
							// spra (3 bits): An unsigned integer that 
							// specifies the size 
							// of the operand of this Sprm. The following 
							// table specifies 
							// the valid values and their meanings 
							// Spra: Meaning
							// 0 Operand is a ToggleOperand (which is 1 
							// byte in size).
							// 1 Operand is 1 byte.
							// 2 Operand is 2 bytes.
							// 3 Operand is 4 bytes.
							// 4 Operand is 2 bytes.
							// 5 Operand is 2 bytes.
							// 6 Operand is of variable length. The 
							// first byte of the 
							// operand indicates the size of the rest of 
							// the operand, 
							// except in the cases of sprmTDefTable 
							// and sprmPChgTabs.
							// 7 Operand is 3 bytes.

static USHORT SprmIspmd(Sprm sprm){
	return sprm  & 0x01FF;
}
static BYTE SprmFSpec(Sprm sprm){
	return (sprm / 512) & 0x0001;
}
static BYTE SprmSgc(Sprm sprm){
	return (sprm / 1024) & 0x0007;
}
static BYTE SprmSpra(Sprm sprm){
	return sprm/8192;
}


/*
 * Prl
 * The Prl structure is a Sprm that is followed by an 
 * operand. The Sprm specifies a property to 
 * modify, and the operand specifies the new value.*/
struct Prl {
	Sprm sprm;          //(2 bytes): Sprm which specifies 
											//the property to be modified
	BYTE operand[];  //(variable): The meaning of the 
											//operand depends on the sprm(Single 
					            //Property Modifiers).
};

/*
 * PrcData
 * The PrcData structure specifies an array of Prl elements and the size of the array.
 */
struct PrcData {
	USHORT cbGrpprl; //(2 byte): A signed integer that specifies the size of GrpPrl, in bytes. 
					   //This value MUST be less than or equal to 0x3FA2
	struct Prl *GrpPrl;//(variable):  An array of Prl elements. GrpPrl contains a whole number 
					   //of Prl elements.
};


/*
 * Prc
 * The Prc structure specifies a set of properties for document content that is referenced by a 
 * Pcd structure.
 */
struct Prc {
	BYTE clxt;         //(1 byte): This value MUST be 0x01
	struct PrcData *data; //(variable):  PrcData that specifies a set of properties.
};

struct FcCompressedSpecialChar {
	BYTE  byte;
	USHORT unicodeCharacter;
};

static const struct FcCompressedSpecialChar FcCompressedSpecialChars[] = 
{
	 {0x82, 0x201A},
	 {0x83, 0x0192},
	 {0x84, 0x201E},
	 {0x85, 0x2026},
	 {0x86, 0x2020},
	 {0x87, 0x2021},
	 {0x88, 0x02C6},
	 {0x89, 0x2030},
	 {0x8A, 0x0160},
	 {0x8B, 0x2039},
	 {0x8C, 0x0152},
	 {0x91, 0x2018},
	 {0x92, 0x2019},
	 {0x93, 0x201C},
	 {0x94, 0x201D},
	 {0x95, 0x2022},
	 {0x96, 0x2013},
	 {0x97, 0x2014},
	 {0x98, 0x02DC},
	 {0x99, 0x2122},
	 {0x9A, 0x0161},
	 {0x9B, 0x203A},
	 {0x9C, 0x0153},
	 {0x9F, 0x0178},
};

static int FcCompressedSpecialChar_compare(const void *key, const void *value) {
    const struct FcCompressedSpecialChar *cp1 = (const struct FcCompressedSpecialChar *)key;
    const struct FcCompressedSpecialChar *cp2 = (const struct FcCompressedSpecialChar *)value;
    return cp1->byte - cp2->byte;
}

static USHORT FcCompressedSpecialChar_get(USHORT nFib){
    struct FcCompressedSpecialChar *result = (struct FcCompressedSpecialChar *)bsearch(&nFib, FcCompressedSpecialChars,
            sizeof(FcCompressedSpecialChars)/sizeof(FcCompressedSpecialChars[0]),
            sizeof(FcCompressedSpecialChars[0]), FcCompressedSpecialChar_compare);
	if (result)
		return result->unicodeCharacter;
	return 0;
}

/*
 * FcCompressed
 * The FcCompressed structure specifies the location of text in the WordDocument Stream.
 */
struct FcCompressed {
	ULONG fc; //fc (30 bits): An unsigned integer that 
							 //specifies an offset in the 
							 //WordDocument Stream where the text starts. 
							 //If fCompressed is zero, the text 
							 //is an array of 16-bit Unicode characters 
							 //starting at offset fc. If 
							 //fCompressed is 1, the text starts at 
							 //offset fc/2 and is an array of 8-bit 
							 //Unicode characters, except for the values 
							 //which are mapped to Unicode 
							 //characters as follows
							 //0x82 0x201A 
							 //0x83 0x0192 
							 //0x84 0x201E 
							 //0x85 0x2026 
							 //0x86 0x2020 
							 //0x87 0x2021 
							 //0x88 0x02C6 
							 //0x89 0x2030 
							 //0x8A 0x0160 
							 //0x8B 0x2039 
							 //0x8C 0x0152 
							 //0x91 0x2018 
							 //0x92 0x2019 
							 //0x93 0x201C 
							 //0x94 0x201D 
							 //0x95 0x2022 
							 //0x96 0x2013 
							 //0x97 0x2014 
							 //0x98 0x02DC 
							 //0x99 0x2122 
							 //0x9A 0x0161 
							 //0x9B 0x203A 
							 //0x9C 0x0153 
							 //0x9F 0x0178
							 //A - fCompressed (1 bit): A bit that 
							 //specifies whether the text is compressed.
							 //B - r1 (1 bit): This bit MUST be zero, 
							 //and MUST be ignored.
};
static bool FcCompressed(struct FcCompressed fc){
	/* TODO: byte order */
	if ((fc.fc & 0x40000000) == 0x40000000) 
							 //if compressed - then ANSI
		return true;
	return false;
};
static ULONG FcValue(struct FcCompressed fc){
	//return fc.fc & 0xFFFFFFFC;	
	return fc.fc & 0x3FFFFFFF;	
}

/*
 * Pcd
 * The Pcd structure specifies the location of text in the
 * WordDocument Stream and additional properties for this
 * text. A Pcd structure is an element of a PlcPcd
 * structure.*/
struct Pcd {
	USHORT ABCfR2; //A - fNoParaLast (1 bit): If this bit is
									 //1, the text MUST NOT contain a
									 //paragraph mark.  B - fR1 (1 bit): This
									 //field is undefined and MUST be ignored
									 //C - fDirty (1 bit): This field MUST be
									 //0 fR2 (13 bits): This field is
									 //undefined and MUST be ignored
	struct FcCompressed fc;
									 //(4 bytes): An FcCompressed structure
									 //that specifies the location of the text
									 //in the WordDocument Stream 
	USHORT prm;    //A Prm structure that specifies
									 //additional properties for this text.
};

/*
 * PlcPcd
 * The PlcPcd structure is a PLC whose data elements are
 * Pcds (8 bytes each). A PlcPcd MUST NOT 
 * contain duplicate CPs.*/
struct PlcPcd {
	ULONG *aCp; //(variable): An array of CPs that
								 //specifies the starting points of text
								 //ranges. The end of each range is the
								 //beginning of the next range. All CPs MUST
								 //be greater than or equal to zero. If any
								 //of the fields ccpFtn, ccpHdd, ccpAtn,
								 //ccpEdn, ccpTxbx, or ccpHdrTxbx from
								 //FibRgLw97 are nonzero, then the last CP
								 //MUST be equal to the sum of those fields
								 //plus ccpText+1. Otherwise, the last CP
								 //MUST be equal to ccpText.
	ULONG aCPl; //number of CP in array

	struct Pcd *aPcd;
								//(variable):  An array of Pcds (8 bytes
								//each) that specify the location of text in
								//the WordDocument stream and any additional
								//properties of the text.  If
								//aPcd[i].fc.fCompressed is 1, then the byte
								//offset of the last character of the text
								//referenced by aPcd[i] is given by the
								//following.  (aPcd[i].fc.fc/2) + aCP[i+1] -
								//aCP[i] - 1; Otherwise, the byte offset of
								//the last character of the text referenced
								//by aPcd[i] is given by the following
								//aPcd[i].fc.fc + 2(aCP[i+1]  - aCP[i] - 1)
								//Because aCP MUST be sorted in ascending
								//order and MUST NOT contain duplicate CPs,
								//(aCP[i+1]-aCP[i])>0, for all valid indexes
								//i of aPcd.  Because a PLC MUST contain one
								//more CP than a data element, i+1 is a
								//valid index of aCP if i is a valid index
								//of aPcd.
	ULONG aPcdl;//number of Pcd in array
};

/*
 * Pcdt
 * The Pcdt structure contains a PlcPcd structure and
 * specifies its size.*/
struct Pcdt {
	BYTE clxt;    //(1 byte): This value MUST be 0x02

	ULONG lcb;    //(4 bytes): An unsigned integer that 
									 //specifies the size, in bytes, of the 
				           //PlcPcd structure.

	struct PlcPcd PlcPcd; 
									//(variable): A PlcPcd structure.  As with
									//all Plc elements, the size that is
									//specified by lcb MUST result in a whole
									//number of Pcd structures in this PlcPcd
									//structure.
};

/*
 * Clx
 * The Clx structure is an array of zero, 1, or more Prcs 
 * followed by a Pcdt.*/
struct Clx {
	struct Prc *RgPrc; 
							//(variable): An array of Prc. If this array
							//is empty, the first byte of the Clx MUST be
							//0x02. 0x02 is invalid as the first byte of a
							//Prc, but required for the Pcdt.

	struct Pcdt *Pcdt; 
							//(variable): A Pcdt.
};

/* The PnFkpPapx structure specifies the offset of a PapxFkp 
 * in the WordDocument Stream.*/
typedef LONG PnFkpPapx;
/* pn (22 bits): An unsigned integer that specifies the 
 * offset in the WordDocument Stream of a PapxFkp structure. 
 * The PapxFkp structure begins at an offset of pn×512.
 * unused (10 bits): This value is undefined and MUST be 
 * ignored. */
static ULONG pnFkpPapx_pn(PnFkpPapx p)
{
	return p & 0x3FFFFF;
}

typedef ULONG PnFkpChpx;
static ULONG pnFkpChpx_pn(PnFkpChpx p)
{
	return p & 0x3FFFFF;
}


/* 2.8.5 PlcBteChpx
 * The PlcBteChpx structure is a PLC that maps the offsets
 * of text in the WordDocument stream to the
 * character properties of that text. Where most PLCs map
 * CPs to data, the PlcBteChpx maps stream
 * offsets to data instead. A PlcBteChpx MUST NOT contain
 * duplicate stream offsets.*/
struct PlcBteChpx {
	ULONG *aFc;       //aFC (variable): An array of
											 //unsigned integers. Each element in
											 //this array specifies an offset in
											 //the
											 //WordDocument stream where text
											 //begins. The end of each range is
											 //the beginning of the next
											 //range. As with all PLCs, the
											 //elements of aFC MUST be sorted in
											 //ascending order
	ULONG *aPnBteChpx;   //aPnBteChpx (variable): An array of
											 //PnFkpChpx (4 bytes each). Each
											 //element of this array specifies
											 //the location in the WordDocument
											 //stream of a ChpxFkp. That ChpxFkp
											 //contains the character
											 //properties for the text at the
											 //corresponding offset in aFC
};
struct PlcBteChpx * plcbteChpx_get(
		FILE *fp, ULONG offset, ULONG size, int *n);

void plcbteChpx_free(struct PlcBteChpx *p);


struct PlcBtePapx {
 ULONG *aFc;     // An array of unsigned integers. Each
								// element in this array specifies an offset
								// in the
								// WordDocument stream. The elements of aFC
								// MUST be sorted in ascending order, and
								// there MUST
								// NOT be any duplicate entries	
 ULONG *aPnBtePapx;
                // An array of PnFkpPapx. The ith entry in
								// aPnBtePapx is a PnFkpPapx that
								// specifies the properties of all
								// paragraphs, table rows, and table cells
								// whose last character occurs
								// at an offset in the WordDocument stream
								// larger than or equal to aFC[i] but
								// smaller than
								// aFC[i+1]; aPnBtePapx MUST contain one
								// less entry than aFC 
};

struct PlcBtePapx * plcbtePapx_get(
		FILE *fp, ULONG offset, ULONG size, int *n);

void plcbtePapx_free(struct PlcBtePapx *p);

/* 2.9.222 Rca
 * The Rca structure is used to define the coordinates of a
 * rectangular area in the document. Unless
 * otherwise specified by the other structures that use this
 * structure, the origin is at the top left of the
 * page and the units are in twips.*/
struct Rca {
	LONG left;           //(4 bytes): An integer that
											 //specifies the X coordinate of the
											 //top left corner of the rectangle.
	LONG top;            //(4 bytes): An integer that
											 //specifies the Y coordinate of the
											 //top left corner of the rectangle.
	LONG right;          //(4 bytes): An integer that
											 //specifies the X coordinate of the
											 //bottom right corner of the rectangle.
	LONG bottom;         //(4 bytes): An integer that
											 //specifies the X coordinate of the
											 //bottom right corner of the rectangle.
};

/* 2.9.253 Spa
 * The Spa structure specifies information about the shapes
 * and drawings that the document contains.*/
struct Spa {
	LONG lid;            //(4 bytes): An integer that
											 //specifies the identifier of a shape
											 //that is contained in the
											 //OfficeArtDggContainer structure.
											 //This value corresponds to the spid
											 //field of an OfficeArtFSP structure
											 //that specifies the data for this
											 //shape. OfficeArtDggContainer and
											 //OfficeArtFSP are specified in
											 //[MS-ODRAW] sections 2.2.12 and
											 //2.2.40, respectively.
	struct Rca rca;      //(16 bytes): An Rca structure that
											 //specifies the rectangle where the
											 //drawing exists. 
											 //The coordinates of rca are in twips.
	USHORT A_bx_by_wr_wrk_B_C_D;    
											 //A - fHdr (1 bit): This bit is
											 //undefined and MUST be ignored.
											 //bx (2 bits): An unsigned integer
											 //that specifies the horizontal
											 //position of the origin that is used
											 //to calculate the rca. This MUST be
											 //one of the following values.
											 //Value Meaning
											 //0 Anchored at the leading margin of
											 //the page.
											 //1 Anchored at the leading edge of
											 //the page.
											 //2 Anchored at the leading edge of
											 //the column.
											 //by (2 bits): An unsigned integer
											 //that specifies the vertical
											 //position of the origin that is used
											 //to calculate the rca. This MUST be
											 //one of the following values.
											 //Value Meaning
											 //0 Anchored at the top margin of the
											 //page.
											 //1 Anchored at the top edge of the
											 //page.
											 //2 Anchored at the top edge of the
											 //paragraph.
											 //wr (4 bits): An unsigned integer
											 //that specifies the style of text
											 //wrapping around this shape. This
											 //MUST be one of the following values.
											 //Value Meaning
											 //0 Wrap text around the object.
											 //1 No text wrapping around the
											 //object. No text appears on either
											 //side of the shape
											 //(top and bottom wrapping).
											 //2 Wrap text around an absolutely
											 //positioned object (square
											 //wrapping).
											 //3 Display as if the shape is not
											 //there. The shape appears either in
											 //front of or behind
											 //the text, based on fBelowText.
											 //4 Wrap text tightly around this
											 //shape, following its contour only
											 //on the left and right
											 //sides (tight wrapping).
											 //5 Wrap text tightly around this
											 //shape, following its contour on all
											 //sides (through wrapping).
											 //wrk (4 bits): An unsigned integer
											 //that specifies the details of the
											 //text wrapping around this shape.
											 //This field MUST be ignored when wr
											 //is 1 or 3. This MUST be one of the
											 //following values.
											 //Value Meaning
											 //0 Allow text wrapping on both sides
											 //of the shape.
											 //1 Allow text wrapping only on the
											 //left side of the shape.
											 //2 Allow text wrapping only on the
											 //right side of the shape.
											 //3 Allow text wrapping only on the
											 //largest side of the shape.
											 //B - fRcaSimple (1 bit): MUST be
											 //zero.
											 //C - fBelowText (1 bit): An unsigned
											 //integer that specifies whether this
											 //shape is behind the text. A
											 //value of 1 specifies that the shape
											 //appears behind the paragraph. A
											 //value of 0 specifies that the
											 //shape appears in front of the text
											 //and obscures it. If wr is not 3,
											 //this field MUST be ignored.
											 //D - fAnchorLock (1 bit): An
											 //unsigned integer that specifies
											 //whether the anchor of the shape is
											 //locked to its current paragraph.
	BYTE cTxbx[4];			 //(4 bytes): This value is undefined
											 //and MUST be ignored.
};

/* 2.8.27 PlcfSpa
 * The PlcfSpa structure is a PLC structure in which the
 * data elements are SPA structures (26 bytes
 * each).*/
struct PlcfSpa {
	CP *aCP;        //(variable): An array of CPs. Each CP
									//specifies the position in the document
									//part of the anchor
									//for a shape. This array MUST NOT contain
									//duplicate CPs. The characters at all but
									//the last CP MUST be 0x08 and MUST have
									//sprmCFSpec applied with a value of 1.
									//See sprmCFSpec for more information.
	struct Spa *aSpa; 
									//(variable): An array of SPAs (26 bytes
									//each) that specify properties for the
									//shape at the corresponding CP.
};

/* 2.9.23 BxPap
 * The BxPap structure specifies the offset of a PapxInFkp
 * in PapxFkp. */
struct BxPap {
 BYTE bOffset;// bOffset (1 byte): An unsigned integer
								 // that specifies the offset of a PapxInFkp
								 // in a PapxFkp. The
								 // offset of the PapxInFkp is bOffset*2. If
								 // bOffset is 0 then there is no PapxInFkp
								 // for this
								 // paragraph and this paragraph has the
								 // default properties as specified in
								 // section 2.6.2.
 BYTE reserved[12];
								 // reserved (12 bytes): Specifies
								 // version-specific paragraph height
								 // information. This value
								 // SHOULD<203> be 0 and SHOULD<204> be
								 // ignored.
};

/* 2.9.114 GrpPrlAndIstd
 * The GrpPrlAndIstd structure specifies the style and
 * properties that are applied to a paragraph, a
 * table row, or a table cell. */
struct GrpPrlAndIstd {
	USHORT istd; // istd (2 bytes): An integer that
								 // specifies the style that is applied to
								 // this paragraph, cell marker or
								 // table row marker. See Applying
								 // Properties for more details about how to
								 // interpret this value.
	BYTE grpprl[];
								 // grpprl (variable): An array of Prl
								 // elements. Specifies the properties of
								 // this paragraph, table row, or
								 // table cell. This array MUST contain a
								 // whole number of Prl elements.
};

/* 2.9.175 PapxInFkp
 * The PapxInFkp structure specifies a set of text
 * properties. */
struct PapxInFkp {
 BYTE cb;     // cb (1 byte): An unsigned integer that
								 // specifies the size of the grpprlInPapx.
								 // If this value is not 0,
								 // the grpprlInPapx is 2×cb-1 bytes long.
								 // If this value is 0, the size is
								 // specified by the first byte of
								 // grpprlInPapx.
 BYTE grpprlInPapx[];
								 // grpprlInPapx (variable): If cb is 0, the
								 // first byte of grpprlInPapx (call it cb')
								 // is an unsigned
								 // integer that specifies the size of the
								 // rest of grpprlInPapx. cb' MUST be at
								 // least 1. After cb',
								 // there are 2×cb' more bytes in
								 // grpprlInPapx. The bytes after cb' form a
								 // GrpPrlAndIstd.
								 // If cb is nonzero, grpprlInPapx is
								 // GrpPrlAndIstd.
};


/* The PapxFkp structure maps paragraphs, table rows, and
 * table cells to their properties. A PapxFkp
 * structure is 512 bytes in size, with cpara in the last
 * byte. The elements of rgbx specify the locations
 * of PapxInFkp structures that start at offsets between the
 * end of rgbx and cpara within this
 * PapxFkp structure. */
struct PapxFkp {
	ULONG *rgfc;  // An array of 4-byte unsigned integers.
									 // Each element of this array specifies
									 // an offset
									 // in the WordDocument Stream where a
									 // paragraph of text begins, or where an
									 // end of row mark
									 // exists. This array MUST be sorted in
									 // ascending order and MUST NOT contain
									 // duplicates. Each
									 // paragraph begins immediately after the
									 // end of the previous paragraph. The
									 // count of elements that
									 // this array contains is cpara
									 // incremented by 1. The last element
									 // does not specify the beginning of
									 // a paragraph; instead it specifies the
									 // end of the last paragraph.
									 //
	struct BxPap *rgbx; // An array of BxPap, followed by
									 // PapxInFkp structures. The elements of
									 // this array,
									 // which has cpara elements and parallels
									 // rgfc, each specify the offset of one
									 // of the PapxInFkp
									 // structures in this PapxFkp structure.
									 // Each PapxInFkp specifies the paragraph
									 // properties for the paragraph at the
									 // corresponding offset
									 // in rgfc or the table properties for
									 // the table row whose end of row mark is
									 // located at the
									 // corresponding offset in rgfc. 
	BYTE  cpara;  // An unsigned integer that specifies the
									 // total number of paragraphs, table
									 // rows, or
									 // table cells for which this PapxFkp
									 // structure specifies formatting. This
									 // field occupies the last byte
};

static void papxFkp_init(
		struct PapxFkp *papxFkp, BYTE buf[512],
		FILE *fp, ULONG offset)
{
	memset(buf, 0, 512);
	fseek(fp, offset, SEEK_SET);
	if (fread(buf, 512, 1, fp) != 1)
	{
		ERR("fread");
		return;
	}

	papxFkp->cpara = buf[511];
	papxFkp->rgfc = (ULONG *)buf;
	papxFkp->rgbx = (struct BxPap *)(&(buf[(papxFkp->cpara + 1)*4]));
#ifdef DEBUG
LOG("PapxFkp->cpara: %d", papxFkp->cpara);
//int i;
//for (i = 0; i < papxFkp->cpara+1; ++i) {
	//LOG("PapxFkp.rgfc[%d]: %d ", i, papxFkp->rgfc[i]);	
//}
//for (i = 0; i < papxFkp->cpara; ++i) {
	//LOG("rgbx[%d].bOffset: %d ", i, papxFkp->rgbx[i].bOffset);	
//}
#endif
}

/* 2.9.33 ChpxFkp
 * The ChpxFkp structure maps text to its character
 * properties. A ChpxFkp structure is 512 bytes in
 * size, with crun in the last byte. The elements of rgb
 * point to Chpxs that start at offsets between crun
 * and the end of rgb.*/
struct ChpxFkp {
	ULONG *rgfc; //rgfc (variable): An array of 4-byte
									//unsigned integers. Each element of this
									//array spafsin the WordDocument Stream
									//where a run of text begins. This array
									//MUST be sorted in ascending
									//order and MUST NOT contain duplicates.
									//Each run ends at the beginning of the
									//next run. This
									//array contains crun+1 elements, where
									//the last element specifies the end of
									//the last run.
	
	BYTE *rgb;   //rgb (variable): An array of 1-byte
									//unsigned integers, followed by an array
									//of Chpx structures. The
									//elements of this array, which has crun
									//elements and parallels rgfc, each
									//specify the offset of one
									//of the Chpxs within this ChpxFkp. The
									//offset is computed by multiplying the
									//value of the byte by 2. For each i from
									//0 to crun, rgb[i]×2 MUST either specify
									//an offset, in bytes, between the end of
									//the array and crun, or be equal to zero,
									//which specifies that there is no Chpx
									//associated with this
									//element of rgb. Each Chpx specifies the
									//character properties for the run of text
									//that is indicated by the
									//corresponding element of rgfc.

	BYTE crun;   //crun (1 byte): An unsigned integer that
									//specifies the number of runs of text
									//this ChpxFkp describes.
									//Crun is the last byte of the ChpxFkp.
									//Crun MUST be at least 0x01, and MUST NOT
									//exceed 0x65,
									//as that would cause rgfc and rgb to grow
									//too large for the ChpxFkp to be 512
									//bytes.
}; 

static void chpxFkp_init(
		struct ChpxFkp *chpxFkp, BYTE buf[512],
		FILE *fp, ULONG offset)
{
	memset(buf, 0, 512);
	fseek(fp, offset, SEEK_SET);
	if (fread(buf, 512, 1, fp) != 1)
	{
		ERR("fread");
		return;
	}

	chpxFkp->crun = buf[511];
	chpxFkp->rgfc = (ULONG *)buf;
	chpxFkp->rgb = &(buf[(chpxFkp->crun + 1)*4]);
#ifdef DEBUG
LOG("ChpxFkp->cpara: %d", chpxFkp->crun);
//int i;
//for (i = 0; i < chpxFkp->crun+1; ++i) {
	//LOG("ChpxFkp.rgfc[%d]: %d ", i, chpxFkp->rgfc[i]);	
//}
//for (i = 0; i < chpxFkp->crun; ++i) {
	//LOG("rgb[%d]: %d ", i, chpxFkp->rgb[i]);	
//}
#endif
}

/* 2.9.32 Chpx
 * The Chpx structure specifies a set of properties for
 * text. */
struct Chpx {
 BYTE cb;     //(1 byte): An unsigned integer that
								 //specifies the size of grpprl, in bytes
 BYTE grpprl; //grpprl (variable): An array of Prl.
								 //Specifies the properties. This array MUST
								 //contain a whole number
								 //of Prls
};


/*
 * FIB
 * The Fib structure is located at offset 0 of the WordDocument Stream.
 */

typedef struct Fib 
{
	FibBase *base;        //MUST be present and has fixed size
	USHORT csw;         //(2 bytes): An unsigned integer that 
												//specifies the count of 16-bit 
												//values corresponding to fibRgW that 
												//follow. MUST be 0x000E. 
	FibRgW97 *rgW97;      //Fib.csw * 2 bytes
    USHORT cslw;      //(2 bytes): An unsigned integer 
												//that specifies the count of 32-bit 
												//values corresponding to fibRgLw 
												//that follow. MUST be 0x0016. 
	FibRgLw97 *rgLw97;    //Fib.cslw * 4 bytes
	USHORT cbRgFcLcb;   //(2 bytes): An unsigned integer 
												//that specifies the count of 64-bit 
												//values corresponding to 
												//fibRgFcLcbBlob that follow. This 
												//MUST be one of the following values,
												//depending on the value of nFib.
	ULONG *rgFcLcb;    //Fib.cbRgFcLcb * 8 bytes
	USHORT cswNew;      //(2 bytes): An unsigned integer 
												//that specifies the count of 16-bit 
												//values corresponding to 
												//fibRgCswNew that follow. This MUST 
												//be one of the following values, 
												//depending on the value of nFib. 
	FibRgCswNew *rgCswNew;
} Fib;

enum {
	stkPar = 1,
	stkCha = 2,
	stkTab = 3,
	stkNum = 4,
};

/* 2.9.260 StdfBase
 * The StdfBase structure specifies general information
 * about a style. */
struct StdfBase {
	USHORT sti_A_B_C_D;//sti (12 bits): An unsigned integer
											 //that specifies the invariant style
											 //identifier for application-defined
											 //styles, or 0x0FFE for user-defined
											 //styles.
											 //The sti identifies which styles in
											 //the stylesheet correspond to which
											 //application-defined styles. An
											 //application-defined style can have
											 //different names in different
											 //languages, but it MUST have the
											 //same sti value regardless of
											 //language. The sti values correspond
											 //to the "Index within Built-in
											 //Styles" table column that is
											 //specified in [ECMA-376] part 4,
											 //section 2.7.3.9 (name).
											 //A - fScratch (1 bit): This bit is
											 //undefined and MUST be ignored.
											 //B - fInvalHeight (1 bit): Specifies
											 //whether the paragraph height
											 //information in the fcPlcfPhe field
											 //of FibRgFcLcb97, for any paragraphs
											 //having this paragraph style, MUST
											 //be ignored.
											 //SHOULD<239> be 0.
											 //C - fHasUpe (1 bit): This bit is
											 //undefined and MUST be ignored.
											 //D - fMassCopy (1 bit): This bit is
											 //undefined and MUST be ignored.
 	USHORT stk_istdBase;
											 //stk (4 bits): An unsigned integer
											 //that specifies the type of this
											 //style, which corresponds to the
											 //afsattribute of the style element
											 //as specified in [ECMA-376] part 4,
											 //section 2.7.3.17 (Style
											 //Definition). This MUST be one of
											 //the following values:
											 //Value Meaning
											 //1 Paragraph style, as specified by
											 //the "paragraph" value in [ECMA-376]
											 //part 4,
											 //section 2.18.90 (ST_StyleType).
											 //2 Character style, as specified by
											 //the "character" value in [ECMA-376]
											 //part 4,
											 //section 2.18.90 (ST_StyleType).
											 //3 Table style, as specified by the
											 //"table" value in [ECMA-376] part 4,
											 //section
											 //2.18.90 (ST_StyleType).
											 //4 Numbering style, as specified by
											 //the "numbering" value in [ECMA-376]
											 //part 4,
											 //section 2.18.90 (ST_StyleType).
											 //istdBase (12 bits): An unsigned
											 //integer that specifies the istd
											 //(see the rglpstd array in the STSH
											 //structure) of the parent style from
											 //which this style inherits in the
											 //style inheritance tree, or 0x0FFF
											 //if this style does not inherit from
											 //any other style in the current
											 //document. The meaning of the
											 //parent style is specified in the
											 //basedOn element in [ECMA-376] part
											 //4, section 2.7.3.3. However,
											 //the style reference in that
											 //specification is a style identifier
											 //rather than an istd, and an
											 //istdBase
											 //value of 0x0FFF corresponds to
											 //omitting the basedOn element.
											 //The istdBase value MUST be an index
											 //that refers to a valid non-empty
											 //style in the array of style
											 //definitions. The istdBase value
											 //MUST NOT be the same as the istd of
											 //the current style and MUST
											 //NOT cause a loop in the style
											 //inheritance tree.
USHORT cupx_istdNext;//cupx (4 bits): An unsigned integer
											 //that specifies the count of
											 //formatting sets inside the
											 //structure,
											 //specified to style type, that is
											 //contained in the GrLPUpxSw.
											 //Each type of style contains a
											 //different structure within
											 //GrLPUpxSw, as shown in the
											 //following
											 //table. The cupx value specifies the
											 //count of structures within the
											 //structure that is contained in the
											 //GrLPUpxSw. For each type of style,
											 //the cupx MUST be equal to the
											 //values that are shown in the
											 //table, depending on whether the
											 //style is revision-marked (in a
											 //revision-marked style the
											 //fHasOriginalStyle value in
											 //StdfPost2000 is 1; in a
											 //non-revision-marked style the value
											 //is 0.)
											 //Table and numbering styles MUST NOT
											 //be revision-marked.
											 //stk value
											 //GrLPUpxSw
											 //contains
											 //cupx for non-revision-
											 //marked style
											 //cupx for revision-
											 //marked-style
											 //1 (paragraph) StkParaGRLPUPX 2 3
											 //2 (character) StkCharGRLPUPX 1 2
											 //3 (table) StkTableGRLPUPX 3 N/A
											 //4 (numbering) StkListGRLPUPX 1 N/A
											 //istdNext (12 bits): An unsigned
											 //integer that specifies the istd
											 //(see rglpstd in STSH) of the style
											 //which is automatically applied to a
											 //new paragraph created following a
											 //paragraph with the current
											 //style, as specified in more detail
											 //in [ECMA-376] part 4, section
											 //2.7.3.10 (next). However, the style
											 //reference in that specification is
											 //a style identifier rather than an
											 //istd.
											 //The istdNext value MUST be an index
											 //that refers to a valid non-empty
											 //style in the array of style
											 //definitions.
USHORT bchUpe;       //bchUpe (2 bytes): An unsigned
											 //integer that specifies the size, in
											 //bytes, of std in LPStd. This value
											 //MUST be equal to cbStd in LPStd. 
USHORT grfstd;       //grfstd (2 bytes): A GRFSTD that
											 //specifies miscellaneous style
											 //properties.
};

static USHORT StdfBaseIstdBase(struct StdfBase *p)
{
	USHORT x = ((p->stk_istdBase & 0xFFF0) >> 4);
	return x;
}

static BYTE StdfBaseStk(struct StdfBase *p)
{
	USHORT x = p->stk_istdBase & 0xF;
	return (BYTE)x;
}

static BYTE StdfBaseCupx(struct StdfBase *p)
{
	USHORT x = p->cupx_istdNext & 0xF;
	return (BYTE)x;
}

static USHORT StdfBaseIstdNext(struct StdfBase *p)
{
	USHORT x = ((p->cupx_istdNext & 0xFFF0) >> 4);
	return x;
}


/* 2.9.259 Stdf
 * The Stdf structure specifies general information about
 * the style.*/
struct Stdf {
	struct StdfBase stdfBase; //stdfBase (10 bytes): An
														//StdfBase structure that
														//specifies general information
														//about the style
	BYTE StdfPost2000OrNone[8];
														//StdfPost2000OrNone (8 bytes):
														//An StdfPost2000OrNone that
														//specifies general information
														//about the style.
};

/* 2.9.353 Xst
 * The Xst structure is a string. The string is prepended by
 * its length and is not null-terminated.*/
struct Xst {
	USHORT cch;       //cch (2 bytes): An unsigned integer
										//that specifies the number of
										//characters that are contained in the
										//rgtchar array.

	USHORT rgtchar[]; //rgtchar (variable): An array of 16-bit
										//Unicode characters that make up a
										//string.
};

/* 2.9.354 Xstz
 * The Xstz structure is a string. The string is prepended
 * by its length and is null-terminated.*/
struct Xstz {
	struct Xst *xst_chTerm;
										// xst (variable): An Xst structure that
										// is prepended with a value which
										// specifies the length of the
										// string.

										// chTerm (2 bytes): A null-terminating
										// character. This value MUST be zero.
};

/* 2.9.258 STD
 * The STD structure specifies a style definition.*/
struct STD {
	struct Stdf stdf;      //stdf (variable): An Stdf that
										     //specifies basic information about the
										     //style.

	BYTE xstzName_grLPUpxSw[];//xstzName (variable): An Xstz
												 //structure that specifies the
												 //primary style name followed by
												 //anybinalternate names (aliases),
												 //with meaning as specified in
												 //[ECMA-376] part 4, section
												 //2.7.3.9 (name)
												 //and [ECMA-376] part 4, section
												 //2.7.3.1 (aliases). The primary
												 //style name and any alternate
												 //style
												 //names are combined into one
												 //string, with a comma character
												 //(U+002C) separating the primary
												 //style name and any alternate
												 //style names. If there are no
												 //alternate style names, the
												 //trailing
												 //comma is omitted.
												 //Each name, whether primary or
												 //alternate, MUST NOT be empty and
												 //MUST be unique within all
												 //names in the stylesheet.

								         //grLPUpxSw (variable): A GrLPUpxSw
												 //structure that specifies the
												 //formatting for the style.
};

struct STD_noStdfPost2000 {
	struct StdfBase stdf;  //stdf (variable): An Stdf that
										     //specifies basic information about the
										     //style.

	BYTE xstzName_grLPUpxSw[]; //xstzName (variable): An Xstz
												 //structure that specifies the
												 //primary style name followed by
												 //anybinalternate names (aliases),
												 //with meaning as specified in
												 //[ECMA-376] part 4, section
												 //2.7.3.9 (name)
												 //and [ECMA-376] part 4, section
												 //2.7.3.1 (aliases). The primary
												 //style name and any alternate
												 //style
												 //names are combined into one
												 //string, with a comma character
												 //(U+002C) separating the primary
												 //style name and any alternate
												 //style names. If there are no
												 //alternate style names, the
												 //trailing
												 //comma is omitted.
												 //Each name, whether primary or
												 //alternate, MUST NOT be empty and
												 //MUST be unique within all
												 //names in the stylesheet.

								         //grLPUpxSw (variable): A GrLPUpxSw
												 //structure that specifies the
												 //formatting for the style.
};


/* 2.9.274 Stshif
 * The Stshif structure specifies general stylesheet
 * information.*/
struct Stshif {
	USHORT cstd;                  //cstd (2 bytes): An
																//unsigned integer that
																//specifies the count of
																//elements in STSH.rglpstd.
																//This value
																//MUST be equal to or
																//greater than 0x000F, and
																//MUST be less than 0x0FFE.

	USHORT cbSTDBaseInFile;       //cbSTDBaseInFile (2 bytes):
																//An unsigned integer that
																//specifies the size, in
																//bytes, of the Stdf
																//structure. The Stdf
																//structure contains an
																//StdfBase structure that is
																//followed by a
																//StdfPost2000OrNone
																//structure which contains
																//an optional StdfPost2000
																//structure. This value
																//MUST be 0x000A when the
																//Stdf structure does not
																//contain an StdfPost2000
																//structure and MUST
																//be 0x0012 when the Stdf
																//structure does contain an
																//StdfPost2000 structure.

  USHORT A_fReserved;           //A - fStdStylenamesWritten
																//(1 bit): This value MUST
																//be 1 and MUST be ignored.

																//fReserved (15 bits): This
																//value MUST be zero and
																//MUST be ignored.

	USHORT stiMaxWhenSaved;       //stiMaxWhenSaved (2 bytes):
																//An unsigned integer that
																//specifies a value that is
																//1 larger than the
																//largest StdfBase.sti index
																//of any application-defined
																//style. This SHOULD<241> be
																//equal to the
																//largest sti index that is
																//defined in the
																//application, incremented
																//by 1.

	USHORT istdMaxFixedWhenSaved; //istdMaxFixedWhenSaved (2
																//bytes): An unsigned
																//integer that specifies the
																//count of elements at
																//the start of STSH.rglpstd
																//that are reserved for
																//fixed-index
																//application-defined
																//styles. This value
																//MUST be 0x000F.

	USHORT nVerBuiltInNamesWhenSaved;
																//nVerBuiltInNamesWhenSaved
																//(2 bytes): An unsigned
																//integer that specifies the
																//binthe style names as
																//defined by the application
																//that writes the file. This
																//value SHOULD<242> be 0.

	USHORT ftcAsci;               //ftcAsci (2 bytes): A
																//signed integer that
																//specifies an operand value
																//for the sprmCRgFtc0 for
																//default
																//document formatting, as
																//defined in the section
																//Determining Formatting
																//Properties.

	USHORT ftcFE;                 //ftcFE (2 bytes): A signed
																//integer that specifies an
																//operand value for the
																//sprmCRgFtc1 for default
																//document formatting, as
																//defined in the section
																//Determining Formatting
																//Properties.

	USHORT ftcOther;              //ftcOther (2 bytes): A
																//signed integer that
																//specifies an operand value
																//for the sprmCRgFtc2 for
																//default
																//document formatting, as
																//defined in the section
																//Determining Formatting
																//Properties.
};

/* 2.9.272 STSHI
 * The STSHI structure specifies general stylesheet and
 * related information.*/
struct STSHI {
	struct Stshif stshif; //stshif (18 bytes): An Stshif that
												//specifies general stylesheet
												//information.

	USHORT ftcBi;					//ftcBi (2 bytes): A signed integer
												//that specifies an operand value
												//for the sprmCFtcBi for default
												//document formatting, as defined in
												//the section Determining Formatting
												//Properties.


	
	BYTE StshiLsd_StshiB[];
												//StshiLsd (variable): An StshiLsd
												//that specifies latent style data.
												
												//StshiB (variable): An STSHIB. This
												//MUST be ignored.
};


/* 2.9.135 LPStd
 * The LPStd structure specifies a length-prefixed style
 * definition. */
struct LPStd {
	SHORT cbStd;  //cbStd (2 bytes): A signed integer that
									//specifies the size, in bytes, of std.
									//This value MUST NOT be
									//less than 0. LPStd structures are stored
									//on even-byte boundaries, but this length
									//MUST NOT
									//include this padding.
									//A style definition can be empty, in
									//which case cbStd MUST be 0.
	int8_t STD[];   //std (variable): An STD that specifies
									//the style definition.
};

/* 2.9.136 LPStshi
 * The LPStshi structure specifies general stylesheet
 * information.*/
struct LPStshi {
	USHORT cbStshi; //cbStshi (2 bytes): An unsigned integer
										//that specifies the size, in bytes, of
										//stshi
	struct STSHI stshi[];//stshi (variable): A stshi that
										//specifies general stylesheet
										//information
};

/* 2.9.271 STSH
 * The STSH structure specifies the stylesheet for a
 * document. The stylesheet describes the styles that
 * are available within a document as well as their
 * formatting.
 * An istd is an index into rglpstd that is used to
 * reference a particular style definition. The istd values
 * are used internally within the stylesheet, such as in the
 * istdBase member of the StdfBase structure,
 * as well as externally outside the stylesheet, such as in
 * Sprm structures such as sprmPIstd. An istd
 * value MUST be greater than or equal to 0x0000 and less
 * than 0x0FFE.
 * Each FIB MUST contain a stylesheet.*/
struct STSH {
	struct LPStshi *lpstshi; //lpstshi (variable): An LPStshi that
										//specifies information about the
										//stylesheet.
	BYTE *rglpstd; //rglpstd (variable): An array of LPStd
										//that specifies the style definitions.
										//The beginning of the rglpstd array is
										//reserved for specific "fixed-index"
										//application-defined styles.
										//A particular fixed-index,
										//application-defined style has the same
										//istd value in every stylesheet. The
										//rglpstd MUST contain an LPStd for each
										//of these fixed-index styles and the
										//order MUST match
										//the order in the following table.
										//istd sti of application-defined style
										//(see sti in StdfBase)
};

struct STSH *STSH_get(FILE *fp, 
		ULONG off, ULONG size, int *n);

void STSH_free(struct STSH *stsh);

struct LPStd *LPStd_at_index(
		BYTE *rglpstd, int size, int index);

/* 2.9.336 UpxChpx
 * The UpxChpx structure specifies the character formatting
 * properties that differ from the parent style
 * as defined by StdfBase.istdBase. */
struct UpxChpx {
	BYTE *grpprlChpx_padding; //grpprlChpx (variable): An
														//array of Prl elements that
														//specifies character formatting
														//properties.
														//This array MUST contain only
														//character Sprm structures.
														//However, this array MUST NOT
														//contain
														//any Sprm structure that
														//specifies a property that is
														//preserved across the
														//application of the
														//sprmCIstd value. Finally, this
														//array MUST NOT contain any of
														//the following:
														//1. sprmCFSpecVanish
														//2. sprmCIstd
														//3. sprmCIstdPermute
														//4. sprmCPlain
														//5. sprmCMajority
														//6. sprmCDispFldRMark
														//7. sprmCIdslRMarkDel
														//8. sprmCLbcCRJ
														//9. sprmCPbiIBullet
														//10. sprmCPbiGrf
														//Additionally, character,
														//paragraph, and list styles
														//MUST NOT contain the sprmCCnf
														//value.
														
														//padding (variable): A
														//UPXPadding structure that
														//specifies the padding that is
														//required to make
														//the UpxChpx structure an even
														//length.
};

/* 2.9.338 UpxPapx
 * The UpxPapx structure specifies the paragraph formatting
 * properties that differ from the parent style,
 * as defined by StdfBase.istdBase.*/
struct UpxPapx {
	USHORT istd;           //(optional) 
												 //istd (2 bytes): An unsigned
												 //integer that specifies the istd
												 //value of the paragraph style. The
												 //istd
												 //value MUST be equal to the
												 //current style.
	
	BYTE *grpprlPapx_padding;      
												 //(variable)
												 //grpprlPapx (variable): An array
												 //of Prl elements that specify
												 //paragraph formatting properties.
												 //This
												 //array MUST contain only paragraph
												 //Sprm structures.
												 //List styles MUST contain only the
												 //sprmPIlfo value.
												 //Paragraph and table styles MUST
												 //NOT contain any Sprm structure
												 //that specifies a property that is
												 //preserved across the application
												 //of the sprmPIstd value.
												 //Additionally, paragraph and table
												 //styles
												 //MUST NOT contain any of the
												 //following:
												 // sprmPIstd
												 // sprmPIstdPermute
												 // sprmPIncLvl
												 // sprmPNest80
												 // sprmPChgTabs
												 // sprmPDcs
												 // sprmPHugePapx
												 // sprmPFInnerTtp
												 // sprmPFOpenTch
												 // sprmPNest
												 // sprmPFNoAllowOverlap
												 // sprmPIstdListPermute
												 // sprmPTableProps
												 // sprmPTIstdInfo
												 //Additionally, paragraph styles
												 //MUST NOT contain sprmPCnf.
	
												 //padding (variable): A UPXPadding
												 //value that specifies the padding
												 //that is required to make the
												 //UpxPapx structure an even length.
};

/* 2.9.140 LPUpxPapx
 * The LPUpxPapx structure specifies paragraph formatting
 * properties.
 * The structure is padded to an even length, but the length
 * in cbUpx MUST NOT include this padding.*/
struct LPUpxPapx {
	USHORT cbUpx;    //cbUpx (2 bytes): An unsigned integer
									 //that specifies the size, in bytes, of
									 //PAPX, not including the
									 //(potential) padding.
	struct UpxPapx PAPX[];
									 //PAPX (variable): A UpxPapx that
									 //specifies paragraph formatting
									 //properties.
};

/* 2.9.138 LPUpxChpx
 * The LPUpxChpx structure specifies character formatting
 * properties. This structure is padded to an
 * even length, but the length in cbUpx MUST NOT include
 * this padding.*/
struct LPUpxChpx{
	USHORT cbUpx;         // cbUpx (2 bytes): An unsigned
												// integer that specifies the size,
												// in bytes, of CHPX. This value
												// does not include the padding.
	
	struct UpxChpx CHPX[];//CHPX (variable): A UpxChpx that
												//specifies character formatting
												//properties.
};

/* 2.9.267 StkParaGRLPUPX
 * The StkParaGRLPUPX structure that specifies the
 * formatting properties for a paragraph style. All
 * members of StkParaGRLPUPX are optional, but those that
 * are present MUST appear in the order
 * that is specified in the following table. Additionally,
 * the number of members that are present MUST
 * match the cupx member of StdfBase for the style. */
struct StkParaGRLPUPX {
	BYTE *data;
															//lpUpxPapx (variable): A
															//LPUpxPapx that specifies the
															//paragraph formatting
															//properties for the
															//style.
  
															//lpUpxChpx (variable): A
															//LPUpxChpx that specifies the
															//character formatting
															//properties for the
															//style.
	
															//StkParaLpUpxGrLpUpxRM
															//(variable): A
															//StkParaLPUpxGrLPUpxRM that
															//specifies the revision-
															//marking information and
															//formatting for the style.
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

/* PICTURES */
#define OfficeArtRecTypeOfficeArtDggContainer           0xF000
#define OfficeArtRecTypeOfficeArtBStoreContainer        0xF001
#define OfficeArtRecTypeOfficeArtSpContainer            0xF004
#define OfficeArtRecTypeOfficeArtFDggBlock              0xF006
#define OfficeArtRecTypeOfficeArtFBSE                   0xF007
#define OfficeArtRecTypeOfficeArtBlipEMF                0xF01A
#define OfficeArtRecTypeOfficeArtBlipWMF                0xF01B
#define OfficeArtRecTypeOfficeArtBlipPICT               0xF01C
#define OfficeArtRecTypeOfficeArtBlipJPEG               0xF01D
#define OfficeArtRecTypeOfficeArtBlipPNG                0xF01E
#define OfficeArtRecTypeOfficeArtBlipDIB                0xF01F
#define OfficeArtRecTypeOfficeArtBlipTIFF               0xF029
#define OfficeArtRecTypeOfficeArtBlipJPEG_              0xF02A
#define OfficeArtRecTypeOfficeArtColorMRUContainer      0xF11A
#define OfficeArtRecTypeOfficeArtSplitMenuClorContainer 0xF11E

#define OfficeArtRecordHeaderSize 8
struct OfficeArtRecordHeader {
	SHORT recVer_recInstance; //recVer
														//(4 bits): An unsigned
														//integer that specifies the
														//version if the record is an
														//atom. If the record is a
														//container, this field MUST
														//contain 0xF
														
														//recInstance 
														//(12 bits): An unsigned
														//integer that differentiates
														//an atom from the other
														//atoms that are contained in
														//the record.
	USHORT recType;           //(2 bytes): An unsigned
														//integer that specifies the
														//type of the record. This
														//value MUST be from 0xF000
														//through 0xFFFF, inclusive.
	
	ULONG recLen;             //(4 bytes): An unsigned
														//integer that specifies the
														//length, in bytes, of the
														//record. If the record is an
														//atom, this value specifies
														//the length of the atom,
														//excluding the header. If the
														//record is a container, this
														//value specifies the sum of
														//the lengths of the atoms that
														//the record contains, plus the
														//length of the record header
														//for each atom.
};

static USHORT 
OfficeArtRecordHeaderRecInstance(
		struct OfficeArtRecordHeader *rh) 
{
	USHORT x = ((rh->recVer_recInstance & 0xFFF0) >> 4);
	return x;
}

struct OfficeArtSpContainer {
	struct OfficeArtRecordHeader rh;
	BYTE shape[];
};

struct OfficeArtInlineSpContainer {
	struct OfficeArtSpContainer *shape;
	BYTE *data;
};

enum MM {
	MM_SHAPE     = 0x0064,
	MM_SHAPEFILE = 0x0066,
};

/* 2.9.156 MFPF
 * The MFPF structure specifies the type of picture data
 * that is stored. */
struct MFPF {
	SHORT mm;             //(2 bytes): A signed integer that
												//specifies the format of the
												//picture data. This MUST be one of
												//the following values.
												//Name Value Meaning
												//MM_SHAPE 0x0064 Shape object
												//MM_SHAPEFILE 0x0066 Shape file
	SHORT xExt;           //(2 bytes): This field is unused and MUST be ignored.
	SHORT yExt;           //(2 bytes): This field is unused and MUST be ignored.
	SHORT swHMF;          //(2 bytes): This field MUST be zero and MUST be ignored.
};

/* 2.9.191 PICF_Shape
 * The PICF_Shape structure specifies additional header
 * information for pictures of type MM_SHAPE or
 * MM_SHAPEFILE. */
struct PICF_Shape {
	LONG grf;             //(4 bytes): This field MUST be ignored.
	LONG padding1;        //(4 bytes): This value MUST be zero and MUST be ignored.
	SHORT mmPM;           //(2 bytes): This field MUST be ignored.
	LONG padding2;        //(4 bytes): This field MUST be zero and MUST be ignored.
};

/* 2.9.193 PICMID
 * The PICMID structure specifies the size and border
 * information for a picture.*/
struct PICMID {
	SHORT dxaGoal;        //(2 bytes): A signed integer that
												//specifies the initial width of the
												//picture, in twips, before
												//cropping or scaling occurs. 
												//This value MUST be greater than zero.
	SHORT dyaGoal;        //(2 bytes): A signed integer that
												//specifies the initial height of
												//the picture, in twips, before
												//cropping or scaling occurs. 
												//This value MUST be greater than zero.
	USHORT mx;            //(2 bytes): An unsigned integer
												//that specifies the ratio, measured
												//in tenths of a percent, between
												//the final display width and the
												//initial picture width that is
												//specified by dxaGoal. If the
												//picture is
												//not cropped, mx values that are
												//greater than 1000 cause the
												//picture to stretch in width, while
												//values that are less than 1000
												//cause the picture to shrink in
												//width. If the picture is
												//horizontally cropped and the mx
												//value is not adjusted accordingly,
												//the picture is scaled. To
												//counteract the new dimensions of a
												//cropped image and avoid scaling,
												//set mx to the value of ((dxaGoal –
												//(left-crop + right-crop)) /
												//dxaGoal. The final display width
												//MUST be at least 15 twips and MUST
												//NOT exceed 31680 twips (22 inches)
												//after cropping and scaling.
	USHORT my;            //(2 bytes): An unsigned integer
												//that specifies the ratio, measured
												//in tenths of a percent, between
												//the final display height and the
												//initial picture height that was
												//specified by dyaGoal. If the
												//picture is not cropped, my values
												//that are greater than 1000 cause
												//the picture to stretch in height,
												//while values of less than 1000
												//cause the picture to shrink.
												//If the picture is vertically
												//cropped and the my value is not
												//adjusted accordingly, the picture
												//is scaled. To counteract the new
												//dimensions of a cropped image and
												//avoid scaling, set the my value to
												//the value of ((dyaGoal – (top-crop
												//+ bottom-crop)) / dyaGoal. The
												//final display height MUST be at
												//least 15 twips and MUST NOT exceed
												//31680 twips (22 inches) after
												//cropping and scaling.
	SHORT dxaReserved1;   //(2 bytes): This value MUST be zero
												//and MUST be ignored.
	SHORT dyaReserved1;   //(2 bytes): This value MUST be zero
												//and MUST be ignored.
	SHORT dxaReserved2;   //(2 bytes): This value MUST be zero
												//and MUST be ignored.
	SHORT dyaReserved2;   //(2 bytes): This value MUST be zero
												//and MUST be ignored.
	BYTE fReserved;       //(8 bits): This value MUST be zero
												//and MUST be ignored.
	BYTE bpp;             //(8 bits): This field is unused and
												//MUST be ignored.
	struct Brc80 brcTop80;//(4 bytes): A Brc80 structure that
												//specifies what border to render
												//above the picture.
	struct Brc80 brcLeft80; 
												//(4 bytes): A Brc80 structure that
												//specifies what border to render to
												//the left of the picture.
	struct Brc80 brcBottom80; 
												//(4 bytes): A Brc80 structure that
												//specifies what border to render
												//below the picture.
	struct Brc80 brcRight80; 
												//(4 bytes): A Brc80 structure that
												//specifies what border to render to
												//the right of the picture.
	SHORT dxaReserved3;   //(2 bytes): This value MUST be zero
												//and MUST be ignored.
	SHORT dyaReserved3;   //(2 bytes): This value MUST be zero
												//and MUST be ignored.
};

/* 2.9.190 PICF
 * The PICF structure specifies the type of a picture, as
 * well as the size of the picture and information
 * about its border.*/
struct PICF {
	LONG lcb;             // (4 bytes): A signed integer that
												// specifies the size, in bytes, of
												// this PICF structure and the
												// subsequent data.
	USHORT cbHeader;      //(2 bytes): An unsigned integer
												//that specifies the size, in bytes,
												//of this PICF structure. This value
												//MUST be 0x44.
	struct MFPF mfpf;     //(8 bytes): An MFPF structure that
												//specifies the storage format of
												//the picture.
	struct PICF_Shape innerHeader;
												//(14 bytes): A PICF_Shape structure
												//that specifies additional header
												//information.
	struct PICMID picmid; //(38 bytes): A PICMID structure
												//that specifies the size and border
												//information of the picture.
	SHORT cProps;         //(2 bytes): This value MUST be 0
												//and MUST be ignored.
};

/* The OfficeArtBlipEMF record specifies BLIP file data for
 * the enhanced metafile format (EMF).*/
struct OfficeArtBlipEMF {
	struct OfficeArtRecordHeader rh; 
												//(8 bytes): An
												//OfficeArtRecordHeader structure,
												//as defined in section 2.2.1, that
												//specifies the header for this
												//record. The following table
												//specifies the subfields.
												//rh.recVer 
												//A value that MUST be 0x0.
												//rh.recInstance A value of 0x3D4 
												//to specify one Unique ID (UID), 
												//or a value of 0x3D5 to specify 
												//two UIDs.
												//rh.recType  A value that MUST 
												//be 0xF01A.
												//rh.recLen      An unsigned integer
												//that specifies the number of bytes
												//following the header. This value
												//MUST be the size of the
												//BLIPFileData field plus 50 if
												//recInstance equals 0x3D4, or
												//the size of BLIPFileData plus 66
												//if recInstance equals 0x3D5.
	BYTE rgbUid1[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
	BYTE rgbUid2[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
												//This field only exists if
												//recInstance equals 0x3D5. If 
												//this value is not 0, rgbUid1 MUST
												//be ignored.
	BYTE metafileHeader[34]; 
												//(34 bytes): An
												//OfficeArtMetafileHeader record, as
												//defined in section 2.2.31, that
												//specifies how to process the
												//metafile in BLIPFileData.

	BYTE *BLIPFileData;   //(variable): A variable-length
												//field that specifies the EMF data.

};

/* The OfficeArtBlipWMF record specifies BLIP file data for
 * the Windows Metafile Format (WMF).*/
struct OfficeArtBlipWMF {
	struct OfficeArtRecordHeader rh; 
												//(8 bytes): An
												//OfficeArtRecordHeader structure,
												//as defined in section 2.2.1, that
												//specifies the header for this
												//record. The following table
												//specifies the subfields.
												//rh.recVer      
												//A value that MUST be 0x0.
												//rh.recInstance A value of 0x216 to
												//specify one UID, or a value of
												//0x217 to specify two UIDs.
												//rh.recType     
												//A value that MUST be 0xF01B.
												//rh.recLen      
												//An unsigned integer that specifies 
												//the number of bytes following 
												//the header. This value MUST be 
												//the size of BLIPFileData plus 50 
												//if recInstance equals 0x216, 
												//or the size of BLIPFileData 
												//plus 66 if recInstance 
												//equals 0x217.

	BYTE rgbUid1[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.

	BYTE rgbUid2[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
												//This field only exists if
												//recInstance equals 0x217. 
												//If this value exists, rgbUid1 
												//MUST be ignored.

	BYTE metafileHeader[34]; 
												//(34 bytes): An
												//OfficeArtMetafileHeader record, as
												//defined in section 2.2.31, that
												//specifies how to process the
												//metafile in BLIPFileData.

	BYTE *BLIPFileData;   //(variable): A variable-length
												//field that specifies the WMF data.

};

/* The OfficeArtBlipPICT record specifies the BLIP file data
 * for the Macintosh PICT format.*/
struct OfficeArtBlipPICT {
	struct OfficeArtRecordHeader rh; 
												//(8 bytes): An
												//OfficeArtRecordHeader structure,
												//as defined in section 2.2.1, that
												//specifies the header for this
												//record. The following table
												//specifies the subfields.
												//rh.recVer      
												//A value that MUST be 0x0.
												//rh.recInstance A value of 0x542 
												//to specify one UID, or a value of
												//0x543 to specify two UIDs.
												//rh.recType     
												//A value that MUST be 0xF01C.
												//rh.recLen An unsigned integer 
												//that specifies the number of 
												//bytes following the header. 
												//This value MUST be the size 
												//of BLIPFileData plus 50 
												//if recInstance equals 0x542, 
												//or the size of BLIPFileData 
												//plus 66 if recInstance 
												//equals 0x543.
	BYTE rgbUid1[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
	BYTE rgbUid2[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
												//This field only exists if
												//recInstance equals
												//0x543. If this value exists, 
												//rgbUid1 MUST be ignored.
	BYTE metafileHeader[34]; 
												//(34 bytes): An
												//OfficeArtMetafileHeader record, as
												//defined in section 2.2.31, that
												//specifies how to process the
												//metafile in BLIPFileData.

	BYTE *BLIPFileData;   //(variable): A variable-length
												//field that specifies the Macintosh
												//PICT data.

};

/* The OfficeArtBlipJPEG record specifies BLIP file data for
 * the Joint Photographic Experts Group (JPEG) format.*/
struct OfficeArtBlipJPEG{
	struct OfficeArtRecordHeader rh; 
												//(8 bytes): An
												//OfficeArtRecordHeader structure,
												//as defined in section 2.2.1, that
												//specifies the header for this
												//record. The following table
												//specifies the subfields.
												//rh.recVer      
												//A value that MUST be 0x0.
												//rh.recInstance A value that is 
												//specified in the following table.
												//rh.recType     
												//A value that MUST be 0xF01D.
												//rh.recLen      
												//An unsigned integer that specifies
												//the number of bytes following the
												//header. This value MUST be the
												//size of BLIPFileData plus 17 if
												//recInstance equals either 0x46A or
												//0x6E2, or the size of BLIPFileData 
												//plus 33 if recInstance equals 
												//either 0x46B or 0x6E3.
												//Value of recInstance         
												//Meaning Number of unique identifiers

												//0x46A JPEG in RGB color space  1
												//0x46B JPEG in RGB color space  2
												//0x6E2 JPEG in CMYK color space 1
												//0x6E3 JPEG in CMYK color space 2

	BYTE rgbUid1[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.

	BYTE rgbUid2[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
												//This field only exists if
												//recInstance equals
												//either 0x46B or 0x6E3. 
												//If this value is specified, 
												//rgbUid1 MUST be ignored.
	BYTE tag;             //(1 byte): An unsigned integer that
												//specifies an application-defined
												//internal resource tag. This value
												//MUST be 0xFF for external files.

	BYTE *BLIPFileData;   //(variable): A variable-length
												//field that specifies the JPEG
												//data.
};

/* The OfficeArtBlipPNG record specifies BLIP file data for
 * the Portable Network Graphics (PNG) format.*/
struct OfficeArtBlipPNG {
	struct OfficeArtRecordHeader rh; 
												//(8 bytes): An
												//OfficeArtRecordHeader structure,
												//as defined in section 2.2.1, that
												//specifies the header for this
												//record. The following table
												//specifies the subfields.
												// rh.recVer      
												// A value that MUST be 0x0.
												//rh.recInstance A value of 0x6E0 to
												//specify one UID, or a value of
												//0x6E1 to specify two UIDs.
												//rh.recType     A value that MUST
												//be 0xF01E.
												//rh.recLen      An unsigned integer
												//that specifies the number of bytes
												//following the header. This value
												//MUST be the size of BLIPFileData
												//plus 17 if recInstance equals
												//0x6E0, or the size of
												//BLIPFileData plus 33 if 
												//recInstance equals 0x6E1.
	BYTE rgbUid1[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
	BYTE rgbUid2[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
												//This field only exists if
												//recInstance equals
												//0x6E1. If this value exists, 
												//rgbUid1 MUST be ignored.
	BYTE tag;             //(1 byte): An unsigned integer that
												//specifies an application-defined
												//internal resource tag. This value
												//MUST be 0xFF for external files.
	BYTE *BLIPFileData;   //(variable): A variable-length
												//field that specifies the PNG data.

};

/* The OfficeArtBlipDIB record specifies BLIP file data for
 * the device-independent bitmap (DIB) format.*/
struct OfficeArtBlipDIB {
	struct OfficeArtRecordHeader rh; 
												//(8 bytes): An
												//OfficeArtRecordHeader structure,
												//as defined in section 2.2.1, that
												//specifies the header for this
												//record. The following table
												//specifies the subfields.
												//rh.recVer A value that MUST be 0x0.
												//rh.recInstance A value of 0x7A8 to
												//specify one UID, or a value of
												//0x7A9 to specify two UIDs.
												//rh.recType     A value that MUST be 0xF01F.
												//rh.recLen      An unsigned integer
												//that specifies the number of bytes
												//following the header. This value
												//MUST be the size of BLIPFileData
												//plus 17 if recInstance equals
												//0x7A8, or the size of
												//BLIPFileData plus 33 if 
												//recInstance equals 0x7A9.
	BYTE rgbUid1[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
	BYTE rgbUid2[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
												//This field only exists if
												//recInstance equals
												//0x7A9. If this value exists, 
												//rgbUid1 MUST be ignored.
	BYTE tag;							//(1 byte): An unsigned integer that
												//specifies an application-defined
												//internal resource tag. This value
												//MUST be 0xFF for external files.
	BYTE *BLIPFileData;   //(variable): A variable-length
												//field that specifies the DIB data.
};

/* The OfficeArtBlipTIFF record specifies BLIP file data for
 * the TIFF format. */
struct OfficeArtBlipTIFF {
	struct OfficeArtRecordHeader rh; 
												//(8 bytes): An
												//OfficeArtRecordHeader structure,
												//as defined in section 2.2.1, that
												//specifies the header for this
												//record. The following table
												//specifies the subfields.
												//rh.recVer      
												//A value that MUST be 0x0.
												//rh.recInstance A value of 0x6E4 to
												//specify one UID, or a value of
												//0x6E5 to specify two UIDs.
												//rh.recType     A value that MUST
												//be 0xF029.
												//rh.recLen      
												//An unsigned integer that 
												//specifies the number of bytes 
												//following the header. 
												//This value MUST be the size of 
												//BLIPFileData plus 17 if 
												//recInstance equals 0x6E4, or 
												//the size of BLIPFileData plus 
												//33 if recInstance equals 0x6E5.
	BYTE rgbUid1[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
	BYTE rgbUid2[16];     //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the uncompressed BLIPFileData.
												//This field only exists if
												//recInstance equals
												//0x6E5. If this value exists, 
												//rgbUid1 MUST be ignored.
	BYTE tag;             //(1 byte): An unsigned integer that
												//specifies an application-defined
												//internal resource tag. This value
												//MUST be 0xFF for external files.
	BYTE *BLIPFileData;   //(variable): A variable-length
												//field that specifies the TIFF
												//data.

};

/*The OfficeArtFBSE record specifies a File BLIP Store Entry
 * (FBSE) that contains information about the BLIP.*/
struct OfficeArtFBSE {
	struct OfficeArtRecordHeader rh; 
												//(8 bytes): An
												//OfficeArtRecordHeader structure,
												//as defined in section 2.2.1, that
												//specifies the header for this
												//record. The following table
												//specifies the subfields.
												//rh.recVer
												//A value that MUST be 0x2.
												//rh.recInstance An MSOBLIPTYPE
												//enumeration value, as defined in
												//section 2.4.1, that specifies the
												//BLIP type and MUST match either
												//btWin32 or btMacOS.
												//rh.recType     A value that MUST
												//be 0xF007.
												//rh.recLen      An unsigned integer
												//that specifies the number of bytes
												//following the header. This value
												//MUST be the size of nameData plus
												//36 if the BLIP is not embedded in
												//this record, or
												//the size of nameData plus size
												//plus 36 if the BLIP is embedded in
												//this record.

	BYTE btWin32;         //(1 byte): An MSOBLIPTYPE
												//enumeration value, as defined in
												//section 2.4.1, that specifies the
												//Windows BLIP type. If the btMacOS
												//value is supported by the Windows
												//operating system, this
												//value MUST match btMacOS. If the
												//values of btWin32 and btMacOS are
												//different, the BLIP that matches
												//rh.recInstance MUST be present and
												//the other MAY be present.

	BYTE btMacOS;         //(1 byte): An MSOBLIPTYPE
												//enumeration value, as defined in
												//section 2.4.1, that specifies the
												//Macintosh BLIP type. If the
												//btWin32 value is supported by the
												//Macintosh operating system,
												//this value MUST match btWin32. If
												//the values of btWin32 and btMacOS
												//are different, the BLIP that
												//matches rh.recInstance MUST be
												//present and the other MAY be
												//present.

	BYTE rgbUid[16];      //(16 bytes): An MD4 message digest,
												//as specified in [RFC1320], that
												//specifies the unique identifier of
												//the pixel data in the BLIP.

	USHORT tag;           //(2 bytes): An unsigned integer
												//that specifies an
												//application-defined internal
												//resource tag. This value MUST be
												//0xFF for external files.

	ULONG size;           //(4 bytes): An unsigned integer
												//that specifies the size, in bytes,
												//of the BLIP in the stream.

	ULONG cRef;           //(4 bytes): An unsigned integer
												//that specifies the number of
												//references to the BLIP. A value of
												//0x00000000 specifies an empty slot
												//in the OfficeArtBStoreContainer
												//record, as defined in
												//section 2.2.20.

	ULONG foDelay;        //(4 bytes): An MSOFO structure, as
												//defined in section 2.1.4, that
												//specifies the file offset into the
												//associated OfficeArtBStoreDelay
												//record, as defined in section
												//2.2.21, (delay stream). A value of
												//0xFFFFFFFF specifies that the file
												//is not in the delay stream, and in
												//this case, cRef MUST be
												//0x00000000.

	BYTE unused1;         //(1 byte): A value that is
												//undefined and MUST be ignored.

	BYTE cbName;          //(1 byte): An unsigned integer that
												//specifies the length, in bytes, of
												//the nameData field, including the
												//terminating NULL character. This
												//value MUST be an even number and
												//less than or equal to 0xFE. If the
												//value is 0x00, nameData will not
												//be written.

	BYTE unused2;         //(1 byte): A value that is
												//undefined and MUST be ignored.

	BYTE unused3;         //(1 byte):  A value that is
												//undefined and MUST be ignored.

	BYTE *nameData;       //(variable): A Unicode
												//null-terminated string that
												//specifies the name of the BLIP.

	BYTE *embeddedBlip;   //(variable): An OfficeArtBlip
												//record, as defined in section
												//2.2.23, specifying the BLIP file
												//data that is embedded in this
												//record. If this value is not 0,
												//foDelay MUST be ignored.
};

/* 2.9.192 PICFAndOfficeArtData
 * The PICFAndOfficeArtData structure specifies header
 * information and binary data for a picture.
 * These structures MUST be stored in the Data Stream at
 * locations that are specified by the
 * sprmCPicLocation value. The range of text that is
 * described by the Chpx structure which contains the
 * sprmCPicLocation value MUST contain the picture character
 * (U+0001). */
struct PICFAndOfficeArtData {
	struct PICF picf;    //(68 bytes): A PICF structure that
											 //specifies the type of the picture,
											 //as well as the picture size
											 //and border information.
	BYTE cchPicName;     //(1 byte): An optional unsigned
											 //integer that specifies the size of
											 //stPicName. This value
											 //MUST exist if and only if
											 //picf.mfpf.mm is MM_SHAPEFILE
											 //(0x0066).
	BYTE *stPicName;     //(variable): An optional string of
											 //ANSI characters that specifies the
											 //full path and file name of the 
											 //picture. This value MUST exist 
											 //if and only if picf.mfpf.mm is 
											 //MM_SHAPEFILE
	BYTE *picture;
												//(variable): An 
												//OfficeArtInlineSpContainer, 
												//as specified in [MS-ODRAW] 
												//section 2.2.15, that specifies 
												//the image.
};

/* 2.9.158 NilPICFAndBinData
 * The NilPICFAndBinData structure that holds header
 * information and binary data for a hyperlink,
 * form field, or add-in field. The NilPICFAndBinData
 * structure MUST be stored in the Data Stream. */
struct NilPICFAndBinData {
	LONG lcb;             //(4 bytes): A signed integer that
												//specifies the size, in bytes, of
												//this structure.
	USHORT cbHeader;      //(2 bytes): An unsigned integer
												//that specifies the number of bytes
												//from the beginning of
												//this structure to the beginning 
												//of binData. This value MUST 
												//be 0x44.
	BYTE ignored[62];     //(62 bytes): This field MUST be 0
												//and MUST be ignored.
	BYTE *binData;        //(variable): The interpretation of
												//the binData element depends on the
												//field type of the field containing 
												//the picture character and is 
												//given by the following.
												//Field Type   Data Type
												//REF          HFD
												//PAGEREF      HFD
												//FORMTEXT     FFData
												//FORMCHECKBOX FFData
												//NOTEREF      HFD
												//PRIVATE			 Custom binary data 
												//             that is specified 
												//             by the add-in that 
												//             inserted this field.
												//ADDIN        Custom binary data 
												//             that is specified by the
												//             add-in that inserted this field.
												//FORMDROPDOWN FFData
												//HYPERLINK    HFD
												//
 //The NilPICFAndBinData structure is invalid if it
 //describes a picture character that is not inside a field
 //or is inside a field with a field type other than those
 //specified in the preceding table. The size of binData is
 //lcb –cbHeader. The data MAY<227> be invalid. If the data
 //is invalid, it MUST be ignored.
};

/* SECTION STRUCTURES */

/* 2.9.243 Sed
 * The Sed structure specifies the location of the section
 * properties.*/
struct Sed {
	SHORT fn;            //(2 bytes): This value is undefined
											 //and MUST be ignored.
	LONG fcSepx;         //(4 bytes): A signed integer value
											 //that specifies the position in the
											 //WordDocument Stream at which a Sepx
											 //structure is located.
	SHORT fnMpr;         //(2 bytes): This value is undefined
											 //and MUST be ignored.
	LONG fcMpr;          //(4 bytes): This value is undefined
											 //and MUST be ignored.
};


/* 2.8.26 PlcfSed
 * The PlcfSed structure is a PLC structure where the data
 * elements are Sed structures (12 bytes
 * each).*/
struct PlcfSed {
	CP *aCP;             //(variable): An array of CPs. Each
											 //CP specifies the beginning of a
											 //range of text in the main document
											 //that constitutes a section. The
											 //range of text ends immediately
											 //prior to the next CP. A PlcfSed
											 //MUST NOT contain duplicate CPs.
											 //There MUST also be an
											 //end-of-section character (0x0C) as
											 //the final character in the text
											 //range of all but the last section.
											 //An end-of-section character (0x0C)
											 //which occurs at a CP and which is
											 //not the last character in a section
											 //specifies a manual page break. The
											 //last CP does not begin a new
											 //section. It MUST be at or beyond
											 //the end of the main document.
											 //Sections only contain text from the
											 //main document, so even when the
											 //last CP comes after text in other
											 //document parts, that text is not
											 //part of the last section.
	struct Sed *aSed;    //(variable): An array of 12-byte Sed
											 //structures. Each Sed structure
											 //contains the location of properties
											 //pertaining to the section that
											 //begins at the corresponding CP.
};


/*
 * MS-DOC Structure.
 */

typedef struct cfb_doc 
{
	FILE *WordDocument;   //document stream
	FILE *Table;          //table stream
	FILE *Data;           //data stream
	
	Fib  fib;             //File information block
	struct Clx clx;       //clx data
	bool biteOrder;				//need to change byte order
	struct PlcBtePapx *plcbtePapx;
	int plcbtePapxNaFc;   // number of aFc in plcbteChpx
	struct PlcBteChpx *plcbteChpx;
	int plcbteChpxNaFc;   // number of aFc in plcbteChpx
	struct PlcfSpa *plcfspa;
	int plcfspaNaCP;      // number of aCP in plcfspa;
	struct PlcfSed *plcfSed;
	int plcfSedNaCP;      // number of aCP in plcfSed;
	struct STSH *STSH;    // style sheet 
	int lrglpstd;         // len of rglpstd
	ldp_t prop;           // properties
} cfb_doc_t;


// open streams and read doc struct
int  doc_read( cfb_doc_t *doc, struct cfb *cfb);

// free memory and close streams
void doc_close(cfb_doc_t *doc);
	
#ifdef __cplusplus
}
#endif

#endif //DOC_H_

// vim:ft=c	
