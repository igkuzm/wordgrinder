#ifndef SPRM_H
#define SPRM_H

#include "doc.h"

enum {
	sgcPar   = 0x1, // paragraph
	sgcCha   = 0x2, // character property
	sgcPic   = 0x3, // picture
	sgcSec   = 0x4, // section
	sgcTab   = 0x5, // table
};

/* 2.6.1 Character Properties
 * A Prl with a sprm.sgc of 2 modifies a character
 * property.*/ 
enum {
	sprmCFRMarkDel        = 0x00,
	sprmCFRMarkIns        = 0x01,
	sprmCFFldVanish       = 0x02,
	sprmCPicLocation      = 0x03, //A signed 32-bit integer
																//that specifies either the
																//position in the Data
																//Stream
																//of a picture or binary
																//data or the name of an OLE
																//object storage.
																//Text with sprmCPicLocation
																//applied MUST also have
																//sprmCFSpec applied with
																//a value of 1. The text
																//range MUST contain only
																//characters from the
																//special
																//characters specified in
																//sprmCFSpec.
																//The value of
																//sprmCPicLocation is
																//evaluated for each
																//character in the text
																//range. The value is
																//evaluated differently
																//depending on the character
																//code, as
																//shown following:
																//If the character is
																//U+0001:
																// The operand of
																//sprmCPicLocation is a
																//position in the
																//DataStream. If
																//sprmCFData is also present
																//and set to 1, the value
																//specifies the position
																//of a NilPICFAndBinData and
																//describes binary data;
																//otherwise the
																//value specifies the
																//position of a
																//PICFAndOfficeArtData and
																//describes a
																//picture.
																//If the character is
																//U+0014:
																// If sprmCFOle2 is also
																//present and set to "true"
																//and the associated field
																//does not have
																//grffldEnd.fZombieEmbed
																//set, the operand of
																//sprmCPicLocation specifies
																//the location of an OLE
																//object storage. If the
																//file is not encrypted with
																//Office Binary Document RC4
																//CryptoAPI
																//Encryption (section
																//2.2.6.3), the value
																//specifies the name of an
																//OLE
																//object storage in the
																//ObjectPool of the
																//document.
																//Specifically, the decimal
																//value is converted to a
																//string, and
																//prefixed with an
																//underscore. The resultant
																//string MUST be the
																//name of a valid OLE
																//storage in the ObjectPool
																//of the
																//document. If the file is
																//encrypted with Office
																//Binary Document
																//RC4 CryptoAPI Encryption,
																//the value specifies an
																//offset in the
																//data stream which contains
																//an FOBJH followed by an
																//OLE
																//object storage.
																//When used in this fashion,
																//the text range on which
																//sprmCPicLocation is
																//applied MUST contain
																//exactly one
																//character.
																//If sprmCFOle2 is absent or
																//set to "false" or the
																//associated field
																//has grffldEnd.fZombieEmbed
																//set, sprmCPicLocation is
																//unused Release: May 17,
																//2022
																//Sprm ispmd operand
																//and MUST be ignored.
																//If there is another
																//character,
																//sprmCPicLocation MUST be
																//ignored.
																//sprmCPicLocation MUST be
																//present for characters
																//that indicate a picture,
																//binary data, or OLE object
																//storage.
	sprmCIbstRMark        = 0x04,
	sprmCDttmRMark        = 0x05,
	sprmCFData            = 0x06, //A Bool8 that specifies
																//whether the picture
																//character in the text
																//represents
																//binary data. If set to
																//true, the text range
																//MUST contain exactly 1
																//character
																//that is the picture
																//character (U+0001) and
																//sprmCPicLocation MUST be
																//present to specify the
																//location of the binary
																//data. By default, a
																//picture
																//character specifies a
																//picture and does not
																//specify binary data.
	sprmCIdslRMark        = 0x07,
	sprmCSymbol           = 0x09,
	sprmCFOle2            = 0x0A, //A Bool8 value that
																//specifies whether the
																//character is a
																//placeholder for an
																//OLE object. When
																//sprmCFOle2 is true,
																//sprmCFObj MUST also be
																//true, and
																//sprmCPicLocation MUST
																//also be set with the OLE
																//storage name. The
																//character representing
																//the OLE object MUST be
																//the field separator
																//(U+00014) of an EMBED
																//field (0x3A), LINK field
																//(0x38), or CONTROL field
																//(0x57). By default,
																//characters are not
																//placeholders for OLE
																//objects.
	sprmCHighlight        = 0x0C,
	sprmCFWebHidden       = 0x11,
	sprmCRsidProp         = 0x15,
	sprmCRsidText         = 0x16,
	sprmCRsidRMDel        = 0x17,
	sprmCFSpecVanish      = 0x18,
	sprmCFMathPr          = 0x1A,
	sprmCIstd             = 0x30,
	sprmCIstdPermute      = 0x31,
	sprmCPlain            = 0x33,
	sprmCKcd              = 0x34,

	sprmCFBold            = 0x35, //0x35 A ToggleOperand value
																//that specifies whether the
																//text is bold. By default,
																//text is not bold.

	sprmCFItalic          = 0x36, //0x36 A ToggleOperand value
																//that specifies whether the
																//text is italicized. By
																//default, text is not
																//italicized.

	sprmCFStrike          = 0x37, //0x37 A ToggleOperand value
																//that specifies whether the
																//text is formatted with
																//strikethrough. By default,
																//text is not struck
																//through.

	sprmCFOutline         = 0x38, //0x38 A ToggleOperand value
																//that specifies whether
																//only the outline contour
																//of
																//the characters in the text
																//is rendered, with the
																//inside of each character
																//left
																//empty. By default, text is
																//rendered in normal solid
																//characters. If
																//sprmCFEmboss, or
																//sprmCFImprint is true,
																//then sprmCFOutline MUST
																//be false.

	sprmCFShadow          = 0x39,

	sprmCFSmallCaps       = 0x3A, //0x3A A ToggleOperand value
																//that specifies whether the
																//text characters are
																//displayed as their capital
																//letter equivalents, in a
																//font size that is smaller
																//than
																//the actual font size that
																//is specified for this
																//text. It does not affect
																//any
																//nonalphabetic character.
																//By default, the characters
																//are displayed in their
																//original character form.

	sprmCFCaps            = 0x3B,
	sprmCFVanish          = 0x3C,
	sprmCKul              = 0x3E,
	sprmCDxaSpace         = 0x40,
	sprmCIco              = 0x42,
	sprmCHps              = 0x43,
	sprmCHpsPos           = 0x45,
	sprmCMajority         = 0x47,
	sprmCIss              = 0x48,
  sprmCHpsKern          = 0x4B,
	sprmCHresi            = 0x4E,
	sprmCRgFtc0           = 0x4F,
	sprmCRgFtc1           = 0x50,
	sprmCRgFtc2           = 0x51,
	sprmCCharScale        = 0x52,
	sprmCFDStrike         = 0x53,
	sprmCFImprint         = 0x54,
	sprmCFSpec            = 0x55, //A ToggleOperand value that
																//specifies whether the
																//current text has a
																//meaning that differs or
																//displays differently than
																//the underlying character
																//to
																//which it is applied. This
																//value SHOULD<146> be
																//applied only to the
																//following
																//characters.
																//U+0001 - A picture
																//location that is used in
																//conjunction with
																//sprmCPicLocation.
																//U+0002 - An auto-numbered
																//footnote reference. See
																//plcffndRef.
																//U+0003 - A short
																//horizontal line.
																//U+0004 - A long horizontal
																//line that is the width of
																//the content area
																//of the page.
																//U+0005 - An annotation
																//reference character. See
																//PlcfandRef.
																//U+0008 - A drawn object.
																//See plcfSpa.
																//U+0013 - A field begin
																//character. See Plcfld.
																//U+0014 - A field separator
																//character. See Plcfld.
																//U+0015 - A field end
																//character. See Plcfld.
																//U+0028 - A symbol. See
																//sprmCSymbol.
																//U+003C - The start of a
																//structured document tag
																//bookmark range.
																//See
																//FibRgFcLcb2003.fcPlcfBkfSdt.
																//U+003E - The end of a
																//structured document tag
																//bookmark range.
																//See
																//FibRgFcLcb2003.fcPlcfBklSdt.
																//U+2002 - An en space.
																//U+2003 - An em space.
																//By default, characters
																//have no special meaning
																//beyond their underlying
																//glyph.
	sprmCFObj             = 0x56, //A Bool8 value that
																//specifies whether the
																//current text represents
																//an
																//embedded object. If
																//sprmCFObj is "true",
																//sprmCFOle2 MUST also be
																//"true".
																//By default, text is not
																//an embedded object.
	sprmCPropRMark90      = 0x57,
	sprmCFEmboss          = 0x58,
	sprmCSfxText          = 0x59,
	sprmCFBiDi            = 0x5A,
	sprmCFBoldBi          = 0x5C,
	sprmCFItalicBi        = 0x5D,
	sprmCFtcBi            = 0x5E,
	sprmCLidBi            = 0x5F,
	sprmCIcoBi            = 0x60,

	sprmCHpsBi            = 0x61, //0x61 An unsigned 2-byte
																//integer value that
																//specifies the size of the
																//text, for text
																//that is displayed
																//right-to-left or text that
																//is a complex script. This
																//value is
																//specified in half-points.
																//The specified value MUST
																//be between 0 and 3276. By
																//default, text of the
																//following Unicode
																//subranges uses the
																//associated size, in
																//half points, as specified
																//in [MC-USB].
																// Thai, Mongolian, and
																//Bangla use a font size of
																//28.
																// Tibetan uses a font size
																//of 32.
																// Devanagari uses a font
																//size of 20.
																// Khmer uses a font size
																//of 36.
																//Text of other Unicode
																//subranges uses a font size
																//of 24 half points.

	sprmCDispFldRMark     = 0x62,
	sprmCIbstRMarkDel     = 0x63,
	sprmCDttmRMarkDel     = 0x64,
	sprmCBrc80            = 0x65,
	sprmCShd80            = 0x66,
	sprmCIdslRMarkDel     = 0x67,
	sprmCFUsePgsuSettings = 0x68,
	sprmCRgLid0_80        = 0x6D,
	sprmCRgLid1_80        = 0x6E,
	sprmCIdctHint         = 0x6F,
	sprmCCv               = 0x70,
	sprmCShd              = 0x71,
	sprmCBrc              = 0x72,
	sprmCRgLid0           = 0x73,
	sprmCRgLid1           = 0x74,
	sprmCFNoProof         = 0x75,
	sprmCFitText          = 0x76,
	sprmCCvUl             = 0x77,
	sprmCFELayout         = 0x78,
	sprmCLbcCRJ           = 0x79,
	sprmCFComplexScripts  = 0x82,
	sprmCWall             = 0x83,
	sprmCCnf              = 0x85,
	sprmCNeedFontFixup    = 0x86,
	sprmCPbiIBullet       = 0x87,
	sprmCPbiGrf           = 0x88,
	sprmCPropRMark        = 0x89,
	sprmCFSdtVanish       = 0x90,
};

/* 2.6.2 Paragraph Properties
 * A Prl with a sprm.sgc of 1 modifies a paragraph
 * property*/
enum {
	sprmPIstd               = 0x00,
	sprmPIstdPermute        = 0x01,
	sprmPIncLvl             = 0x02,

	sprmPJc80               = 0x03, //0x03 An unsigned 8-bit
																	//integer that specifies
																	//the physical
																	//justification of the
																	//paragraph. This MUST be
																	//one of the following
																	//values.
																	//0 - Paragraph is
																	//physically left
																	//justified.
																	//1 - Paragraph is
																	//centered.
																	//2 - Paragraph is
																	//physically right
																	//justified.
																	//3 - Paragraph is
																	//justified to both right
																	//and left with a low
																	//character compression
																	//ratio.
																	//4 - Paragraph is
																	//justified to both right
																	//and left with a medium
																	//character compression
																	//ratio.
																	//5 - Paragraph is
																	//justified to both right
																	//and left with a high
																	//character compression
																	//ratio.
																	//By default, paragraphs
																	//are physically
																	//left-justified.

	sprmPFKeep              = 0x05,
	sprmPFKeepFollow        = 0x06,
	sprmPFPageBreakBefore   = 0x07,
	sprmPIlvl               = 0x0A,
	sprmPIlfo               = 0x0B,
	sprmPFNoLineNumb        = 0x0C,
	sprmPChgTabsPapx        = 0x0D,
	sprmPDxaRight80         = 0x0E,
	sprmPDxaLeft80          = 0x0F,
	sprmPNest80             = 0x10,
	sprmPDxaLeft180         = 0x11,
	sprmPDyaLine            = 0x12, //An LSPD value that
																	//specifies the spacing
																	//between lines in this
																	//paragraph. By default,
																	//paragraphs use single
																	//spacing
	sprmPDyaBefore          = 0x13, //0x13 A two-byte unsigned
																	//integer value that
																	//specifies the size, in
																	//twips, of
																	//the spacing before this
																	//paragraph. The value
																	//MUST be a number
																	//between 0x0000 and
																	//0x7BC0, inclusive. When
																	//auto-spacing is
																	//supported and the value
																	//of sprmPFDyaBeforeAuto
																	//is 1, this property is
																	//ignored. By default, the
																	//space before a paragraph
																	//is zero twips.

	sprmPDyaAfter           = 0x14, //0x14 A two-byte unsigned
																	//integer value that
																	//specifies the size, in
																	//twips, of
																	//the spacing after this
																	//paragraph. The value
																	//MUST be between 0x0000
																	//and 0x7BC0, inclusive.
																	//When auto spacing is
																	//supported and the value
																	//of sprmPFDyaAfterAuto is
																	//1, this property is
																	//ignored. By default, the
																	//space after a paragraph
																	//is zero twips.

	sprmPChgTabs            = 0x15,
	sprmPFInTable           = 0x16,
	sprmPFTtp               = 0x17, // terminating paragraph
																	// mark
	sprmPDxaAbs             = 0x18,
	sprmPDyaAbs             = 0x19,
	sprmPDxaWidth           = 0x1A,
	sprmPPc                 = 0x1B,
	sprmPWr                 = 0x23,
	sprmPBrcTop80           = 0x24,
	sprmPBrcLeft80          = 0x25,
	sprmPBrcBottom80        = 0x26,
	sprmPBrcRight80         = 0x27,
	sprmPBrcBetween80       = 0x28,
	sprmPBrcBar80           = 0x29,
	sprmPFNoAutoHyph        = 0x2A,
	sprmPWHeightAbs         = 0x2B,
	sprmPDcs                = 0x2C,
	sprmPShd80              = 0x2D,
	sprmPDyaFromText        = 0x2E,
	sprmPDxaFromText        = 0x2F,
	sprmPFLocked            = 0x30,
	sprmPFWidowControl      = 0x31,
	sprmPFKinsoku           = 0x33,
	sprmPFWordWrap          = 0x34,
	sprmPFOverflowPunct     = 0x35,
	sprmPFTopLinePunct      = 0x36,
	sprmPFAutoSpaceDE       = 0x37,
	sprmPFAutoSpaceDN       = 0x38,
	sprmPWAlignFont         = 0x39,
	sprmPFrameTextFlow      = 0x3A,
	sprmPOutLvl             = 0x40,
	sprmPFBiDi              = 0x41,
	sprmPNumRM              = 0x45,
	sprmPHugePapx           = 0x46,
	sprmPFUsePgsuSettings   = 0x47,
	sprmPFAdjustRight       = 0x48,
	sprmPItap               = 0x49, //An integer value that
																	//specifies the table
																	//depth of this paragraph.
																	//See
																	//the Overview of Tables
																	//(section 2.4.3) for the
																	//rules that this value
																	//follows. This value,
																	//when present, MUST be a
																	//non-negative number. By
																	//default, paragraphs are
																	//not in tables
	sprmPDtap               = 0x4A, //A signed integer that
																	//specifies an addition or
																	//subtraction to the
																	//existing
																	//table depth of this
																	//paragraph. It provides
																	//an alternate way of
																	//specifying table depth
																	//to sprmPItap or a way to
																	//increment or
																	//decrement any value that
																	//was already set by
																	//sprmPItap or
																	//sprmPDtap
	sprmPFInnerTableCell    = 0x4B,
	sprmPFInnerTtp          = 0x4C,
	sprmPShd                = 0x4D,
	sprmPBrcTop             = 0x4E,
	sprmPBrcLeft            = 0x4F,
	sprmPBrcBottom          = 0x50,
	sprmPBrcRight           = 0x51,
	sprmPBrcBetween         = 0x52,
	sprmPBrcBar             = 0x53,
	sprmPDxcRight           = 0x55,
	sprmPDxcLeft            = 0x56,
	sprmPDxcLeft1           = 0x57,
	sprmPDylBefore          = 0x58,
	sprmPDylAfter           = 0x59,
	sprmPFOpenTch           = 0x5A,
	sprmPFDyaBeforeAuto     = 0x5B,

	sprmPFDyaAfterAuto      = 0x5C, //0x5C A Bool8 value that
																	//specifies whether the
																	//space displayed after
																	//this
																	//paragraph uses auto
																	//spacing. A value of 1
																	//specifies that
																	//sprmPDyaAfter
																	//MUST be ignored if the
																	//application supports
																	//auto spacing. By
																	//default,
																	//auto spacing is disabled
																	//for paragraphs.

	sprmPDxaRight           = 0x5D,
	sprmPDxaLeft            = 0x5E,
	sprmPNest               = 0x5F,
	sprmPDxaLeft1           = 0x60,

	sprmPJc                 = 0x61, //0x61 An unsigned 8-bit
																	//integer value that
																	//specifies the logical
																	//justification of
																	//the paragraph. The value
																	//MUST be one of those
																	//listed following. Some
																	//of the values also
																	//correspond to the ST_Jc
																	//enumeration values that
																	//are specified in
																	//[ECMA-376] Part 4,
																	//Section 2.18.50 ST_Jc
																	//(Horizontal
																	//Alignment Type).
																	//0
																	//St_Jc: left
																	//Paragraph is logical
																	//left justified
																	//1
																	//St_Jc: center
																	//Paragraph is centered
																	//2
																	//St_Jc: right
																	//Paragraph is logical
																	//right justified
																	//3
																	//St_Jc: both
																	//Paragraph is justified
																	//to both right and left
																	//4
																	//St_Jc:distribute
																	//Paragraph characters are
																	//distributed to fill
																	//the
																	//entire width of
																	//the paragraph
																	//5
																	//St_Jc: mediumKashida
																	//If the language is
																	//Arabic, the paragraph
																	//uses medium-length
																	//Kashida. In other
																	//languages, text is
																	//justified with a 
																	//medium
																	//character compression
																	//ratio.
																	//6
																	//Paragraph is indented
																	//7
																	//St_Jc: highKashida
																	//If the language is
																	//Arabic, the paragraph
																	//uses longer length
																	//Kashida. In other
																	//languages, text is
																	//justified with a high
																	//character compression
																	//ratio.
																	//8
																	//St_Jc: lowKashida
																	//If the language is
																	//Arabic, the paragraph
																	//uses small length
																	//Kashida. In other
																	//languages, text is
																	//justified with a high
																	//character compression
																	//ratio.
																	//9
																	//St_Jc:thaiDistribute
																	//If the language of
																	//the
																	//paragraph is Thai,
																	//the
																	//text is justified
																	//with Thai distributed
																	//justification. In
																	//other
																	//languages, text is
																	//justified with a low
																	//character compression
																	//ratio.
																	//The default is
																	//logical
																	//left justification.
	
	sprmPFNoAllowOverlap    = 0x62,
	sprmPWall               = 0x64,
	sprmPIpgp               = 0x65,
	sprmPCnf                = 0x66,
	sprmPRsid               = 0x67,
	sprmPIstdListPermute    = 0x69,
	sprmPTableProps         = 0x6B,
	sprmPTIstdInfo          = 0x6C,
	sprmPFContextualSpacing = 0x6D,
	sprmPPropRMark          = 0x6F,
	sprmPFMirrorIndents     = 0x70,
	sprmPTtwo               = 0x71,
};

/* 2.6.3 Table Properties
 * A Prl with a sprm.sgc of 5 modifies a table property.*/
enum {
	sprmTJc90                = 0x00, //0x00 An unsigned 16-bit
																	 //integer value that
																	 //specifies the physical
																	 //justification of
																	 //the table. The valid
																	 //values and their
																	 //meanings are as
																	 //follows.
																	 //0 - The table is
																	 //physical left
																	 //justified.
																	 //1 - The table is
																	 //centered.
																	 //2 - The table is
																	 //physical right
																	 //justified.
																	 //Tables do not have a
																	 //default physical
																	 //justification. Their
																	 //default
																	 //justification is
																	 //logical left.
	
	sprmTDxaLeft             = 0x01,
	sprmTDxaGapHalf          = 0x02,
	sprmTFCantSplit90        = 0x03,
	sprmTTableHeader         = 0x04, //A Bool8 value that
																	 //specifies that the
																	 //current table row is a
																	 //header row. If
																	 //the value is 0x01 but
																	 //sprmTTableHeader is not
																	 //applied with a value of
																	 //0x01 for a previous row
																	 //in the same table, then
																	 //this property MUST be
																	 //ignored.
																	 //By default, a table row
																	 //is not a header row
	sprmTTableBorders80      = 0x05, //A TableBordersOperand80
																	 //value that specifies
																	 //border information for
																	 //the
																	 //cells in a table row.
																	 //By default, table rows
																	 //have no borders
	sprmTDyaRowHeight        = 0x07,
	sprmTDefTable            = 0x08, //A TDefTableOperand
																	 //value that specifies
																	 //the number of columns
																	 //in the
																	 //table row, the width
																	 //of each column, border
																	 //attributes, and a
																	 //variety of
																	 //other settings.
																	 //By default, a table
																	 //row has zero columns.
																	 //In order for a table
																	 //to have
																	 //columns, the file MUST
																	 //provide a
																	 //sprmTDefTable or a
																	 //sprmTInsert for each
																	 //table row.
	sprmTDefTableShd80       = 0x09,
	sprmTTlp                 = 0x0A,
	sprmTFBiDi               = 0x0B,
	sprmTDefTableShd3rd      = 0x0C,
	sprmTPc                  = 0x0D,
	sprmTDxaAbs              = 0x0E,
	sprmTDyaAbs              = 0x0F,
	sprmTDxaFromText         = 0x10,
	sprmTDyaFromText         = 0x11,
	sprmTDefTableShd         = 0x12,
	sprmTTableBorders        = 0x13, //A TableBordersOperand
																	 //value that specifies
																	 //the borders for this
																	 //row unless
																	 //modified by other Sprms
																	 //applied to the cells.
																	 //By default, table rows
																	 //have
																	 //no borders
	sprmTTableWidth          = 0x14,
	sprmTFAutofit            = 0x15,
	sprmTDefTableShd2nd      = 0x16,
	sprmTWidthBefore         = 0x17,
	sprmTWidthAfter          = 0x18,
	sprmTFKeepFollow         = 0x19,
	sprmTBrcTopCv            = 0x1A,
	sprmTBrcLeftCv           = 0x1B,
	sprmTBrcBottomCv         = 0x1C,
	sprmTBrcRightCv          = 0x1D,
	sprmTDxaFromTextRight    = 0x1E,
	sprmTDyaFromTextBottom   = 0x1F,
	sprmTSetBrc80            = 0x20, //A TableBrc80Operand
																	 //value that specifies
																	 //the borders of a set of
																	 //cells in
																	 //the table row. By
																	 //default, cells have no
																	 //borders
	sprmTInsert              = 0x21,
	sprmTDelete              = 0x22,
	sprmTDxaCol              = 0x23,
	sprmTMerge               = 0x24,
	sprmTSplit               = 0x25,
	sprmTTextFlow            = 0x29,
	sprmTVertMerge           = 0x2B,
	sprmTVertAlign           = 0x2C,
	sprmTSetShd              = 0x2D,
	sprmTSetShdOdd           = 0x2E,
	sprmTSetBrc              = 0x2F, //A TableBrcOperand value
																	 //that specifies the
																	 //border type of a set of
																	 //cells in
																	 //a table row. By
																	 //default, the border
																	 //type is inherited from
																	 //the table border
																	 //properties
	sprmTCellPadding         = 0x32,

	sprmTCellSpacingDefault  = 0x33, //0x33 A CSSAOperand
																	 //that specifies the
																	 //cell spacing for
																	 //each
																	 //cell in the entire
																	 //row. cssa.itc.
																	 //itcFirst
																	 //MUST be 0,
																	 //cssa.itc.itcLim MUST
																	 //be 1, cssa.grfbrc
																	 //MUST
																	 //be fbrcSidesOnly
																	 //(0x0F), 
																	 //cssa.ftsWidth
																	 //MUST be ftsNil
																	 //(0x00)
																	 //or ftsDxa (0x03) 
																	 //or
																	 //ftsDxaSys (0x13),
																	 //and
																	 //cssa.wWidth MUST be
																	 //nonnegative and MUST
																	 //NOT exceed 15840
																	 //(11"). By default,
																	 //cells do not have
																	 //cell
																	 //spacing.")

	sprmTCellPaddingDefault  = 0x34,
	sprmTCellWidth           = 0x35,
	sprmTFitText             = 0x36,
	sprmTFCellNoWrap         = 0x39,
	sprmTIstd                = 0x3A,
	sprmTCellPaddingStyle    = 0x3E,
	sprmTCellFHideMark       = 0x42,
	sprmTSetShdTable         = 0x60,
	sprmTWidthIndent         = 0x61,
	sprmTCellBrcType         = 0x62, //A TCellBrcTypeOperand
																	 //value that specifies
																	 //the border type for
																	 //the first
																	 //several consecutive
																	 //cells in a table row.
																	 //By default, the border
																	 //type is
																	 //inherited from the
																	 //table style of the
																	 //whole table.
																	 //
	sprmTFBiDi90             = 0x64,
	sprmTFNoAllowOverlap     = 0x65,
	sprmTFCantSplit          = 0x66,
	sprmTPropRMark           = 0x67,
	sprmTWall                = 0x68,
	sprmTIpgp                = 0x69,
	sprmTCnf                 = 0x6A,
	sprmTDefTableShdRaw      = 0x70,
	sprmTDefTableShdRaw2nd   = 0x71,
	sprmTDefTableShdRaw3rd   = 0x72,
	sprmTRsid                = 0x79,
	sprmTCellVertAlignStyle  = 0x7C,
	sprmTCellNoWrapStyle     = 0x7D,
	sprmTCellBrcTopStyle     = 0x7F, //A BrcOperand value
																	 //that specifies the top
																	 //border for cells that
																	 //are affected
																	 //by a CNFOperand value.
																	 //This Sprm MUST NOT
																	 //appear outside of the
																	 //grpprl array of a
																	 //CNFOperand value. By
																	 //default, cells have no
																	 //top border.
	sprmTCellBrcBottomStyle  = 0x80, //A BrcOperand value
																	 //that specifies the
																	 //bottom border for
																	 //cells that are
																	 //affected by a
																	 //CNFOperand value. This
																	 //Sprm MUST NOT appear
																	 //outside of
																	 //the grpprl array of a
																	 //CNFOperand. By
																	 //default, cells have no
																	 //bottom
																	 //border
	sprmTCellBrcLeftStyle    = 0x81, //A BrcOperand value
																	 //that specifies the
																	 //logical left border
																	 //for cells that are
																	 //affected by a
																	 //CNFOperand value. This
																	 //Sprm MUST NOT appear
																	 //outside of
																	 //the grpprl array of a
																	 //CNFOperand. By
																	 //default, cells have no
																	 //logical left
																	 //border
	sprmTCellBrcRightStyle   = 0x82, //A BrcOperand value
																	 //that specifies the
																	 //logical right border
																	 //for cells that
																	 //are affected by a
																	 //CNFOperand value. This
																	 //Sprm MUST NOT appear
																	 //outside
																	 //of the grpprl array of
																	 //a CNFOperand. By
																	 //default, cells have no
																	 //logical
																	 //right border
	sprmTCellBrcInsideHStyle = 0x83,
	sprmTCellBrcInsideVStyle = 0x84,
	sprmTCellBrcTL2BRStyle   = 0x85,
	sprmTCellBrcTR2BLStyle   = 0x86,
	sprmTCellShdStyle        = 0x87,
	sprmTCHorzBands          = 0x88,
	sprmTCVertBands          = 0x89,
	sprmTJc                  = 0x8A, // An unsigned 16-bit
																	 // integer value that
																	 // specifies the physical
																	 // justification of
																	 // the table. The valid
																	 // values and their
																	 // meanings are as
																	 // follows.
																	 // 0 - The table is
																	 // physical left
																	 // justified.
																	 // 1 - The table is
																	 // centered.
																	 // 2 - The table is
																	 // physical right
																	 // justified.
																	 // Tables do not have a
																	 // default physical
																	 // justification. Their
																	 // default
																	 // justification is
																	 // logical left
};

/* 2.6.4 Section Properties
 * A Prl structure with a sprm.sgc of 4 modifies a section
 * property.*/
enum {
	sprmScnsPgn        = 0x00,
	sprmSiHeadingPgn   = 0x01,
	sprmSDxaColWidth   = 0x03,
	sprmSDxaColSpacing = 0x04,
	sprmSFEvenlySpaced = 0x05,
	sprmSFProtected    = 0x06,
	sprmSDmBinFirst    = 0x07,
	sprmSDmBinOther    = 0x08,
	sprmSBkc           = 0x09,
	sprmSFTitlePage    = 0x0A,
	sprmSCcolumns      = 0x0B,
	sprmSDxaColumns    = 0x0C,
	sprmSNfcPgn        = 0x0E,
	sprmSFPgnRestart   = 0x11,
	sprmSFEndnote      = 0x12,
	sprmSLnc           = 0x13,
	sprmSNLnnMod       = 0x15,
	sprmSDxaLnn        = 0x16,
	sprmSDyaHdrTop     = 0x17,
	sprmSDyaHdrBottom  = 0x18,
	sprmSLBetween      = 0x19,
	sprmSVjc           = 0x1A,
	sprmSLnnMin        = 0x1B,
	sprmSPgnStart97    = 0x1C,
	sprmSBOrientation  = 0x1D,
	sprmSXaPage        = 0x1F, //An unsigned 16-bit integer
														 //that specifies the page width
														 //of the section in twips.
														 //The value of the operand MUST
														 //be in the interval [144,
														 //31680].
														 //By default, the page width is
														 //215.9 mm (8.5 inches, or
														 //12240 twips)

	sprmSYaPage        = 0x20, //0x20 An unsigned 16-bit
														 //integer that specifies the
														 //page height of the
														 //section,
														 //in
														 //twips. The value of the
														 //operand MUST be in the
														 //interval [144, 31680].
														 //By default, the page
														 //height
														 //is 279.4 mm (11 inches, or
														 //15840 twips).

	sprmSDxaLeft       = 0x21, //An XAS_nonNeg that specifies
														 //the width, in twips, of the
														 //left margin.
														 //By default, the width of the
														 //left margin varies depending
														 //on the implementation
														 //and the system settings, so
														 //implementations MUST use this
														 //SPRM to specify the
														 //left margin of each section
														 
														 // 2.9.350 XAS_nonNeg
														 // The XAS_nonNeg value is a
														 // 16-bit unsigned integer that
														 // specifies horizontal
														 // distance, in twips. This
														 // value MUST be less than or
														 // equal to 31680
	sprmSDxaRight      = 0x22,
	sprmSDyaTop        = 0x23,
	sprmSDyaBottom     = 0x24,
	sprmSDzaGutter     = 0x25,
	sprmSDmPaperReq    = 0x26,
	sprmSFBiDi         = 0x28,
	sprmSFRTLGutter    = 0x2A,
	sprmSBrcTop80      = 0x2B,
	sprmSBrcLeft80     = 0x2C,
	sprmSBrcBottom80   = 0x2D,
	sprmSPgbProp       = 0x2F,
	sprmSDxtCharSpace  = 0x30,

	sprmSDyaLinePitch  = 0x31, //0x31 A YAS that specifies,
														 //in twips, the line height
														 //that is used for document
														 //grid, if enabled (see
														 //sprmSClm). This line
														 //height
														 //does not apply to lines
														 //within table cells in case
														 //the
														 //fDontAdjustLineHeightInTable
														 //flag is set in the
														 //document
														 //Dop2000.  If the document
														 //grid is enabled (see
														 //sprmSClm), a section MUST
														 //specify the line height
														 //that
														 //is used for the document
														 //grid.  This value MUST be
														 //greater than or equal 
														 //to 1,
														 //and MUST be less than or
														 //equal to 31680.

	sprmSClm           = 0x32,
	sprmSTextFlow      = 0x33,
	sprmSBrcTop        = 0x34,
	sprmSBrcLeft       = 0x35,
	sprmSBrcBottom     = 0x36,
	sprmSBrcRight      = 0x37,
	sprmSWall          = 0x39,
	sprmSRsid          = 0x3A,
	sprmSFpc           = 0x3B,
	sprmSRncFtn        = 0x3C,
	sprmSRncEdn        = 0x3E,
	sprmSNFtn          = 0x3F,
	sprmSNfcFtnRef     = 0x40,
	sprmSNEdn          = 0x41,
	sprmSNfcEdnRef     = 0x42,
	sprmSPropRMark     = 0x43,
	sprmSPgnStart      = 0x44,
};

/* 2.6.5 Picture Properties
 * A Prl with a sprm.sgc of 3 modifies a picture
 * property.*/
enum {
	sprmPicBrcTop80    = 0x02, //A Brc80 that specifies the
														 //top border of the inline
														 //picture. The Brc80.brcType
														 //field MUST be less than or
														 //equal to 0x19. By default,
														 //inline pictures do not have
														 //borders
	sprmPicBrcLeft80   = 0x03,
	sprmPicBrcBottom80 = 0x04,
	sprmPicBrcRight80  = 0x05,
	sprmPicBrcTop      = 0x08, //A BrcOperand that specifies
														 //the top border of the inline
														 //picture. The
														 //BrcOperand.Brc.brcType field
														 //MUST be less than or equal to
														 //0x1B. By default,
														 //inline pictures do not have
														 //borders
	sprmPicBrcLeft     = 0x09,
	sprmPicBrcBottom   = 0x0A,
	sprmPicBrcRight    = 0x0B,
};

#endif /* ifndef SPRM_H */
