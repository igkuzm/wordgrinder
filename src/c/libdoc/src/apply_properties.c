/**
 * File              : apply_properties.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 28.05.2024
 * Last Modified Date: 06.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/apply_properties.h"
#include "../include/libdoc/style_properties.h"
#include "../include/libdoc/operands.h"
#include <stdint.h>

static int apply_char_property(cfb_doc_t *doc, int l, struct Prl *prl);
static int apply_paragraph_property(cfb_doc_t *doc, struct Prl *prl);
static int apply_picture_property(cfb_doc_t *doc, struct Prl *prl);
static int apply_section_property(cfb_doc_t *doc, struct Prl *prl);
static int apply_table_property(cfb_doc_t *doc, struct Prl *prl);

int apply_property(cfb_doc_t *doc, int l, struct Prl *prl)
{
	BYTE sgc = SprmSgc(prl->sprm);
#ifdef DEBUG
	LOG("sgc: 0x%X", sgc);
#endif
	switch (sgc) {
		case sgcCha:
			return apply_char_property(doc, l, prl);
		case sgcPar:
			return apply_paragraph_property(doc, prl);
		case sgcSec:
			return apply_section_property(doc, prl);
		case sgcTab:
			return apply_table_property(doc, prl);
		case sgcPic:
			return apply_picture_property(doc, prl);
		
		default:
			break;
	}

	return 0;
}

int apply_char_property(
		cfb_doc_t *doc, int l, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	BYTE sgc = SprmSgc(prl->sprm);
	LOG("sgc: 0x%x, ismpd: 0x%02x", sgc, ismpd); 
#endif

	CHP *chp = &(doc->prop.chp);
	switch (l) {
		case 1: chp = &(doc->prop.pap_chp); break;	
		default: break;
	}

	// set bold
	if (ismpd == sprmCFBold){
		chp->fBold = 
			ToggleOperand(doc, prl->operand[0]); 
		return 0;
	}
	
	// set italic
	if (ismpd == sprmCFItalic){
		chp->fItalic = 
			ToggleOperand(doc, prl->operand[0]); 
		return 0;
	}
	
	// set outline
	if (ismpd == sprmCFOutline){
		chp->fUnderline = 
			ToggleOperand(doc, prl->operand[0]); 
		return 0;
	}

	// background color
	if (ismpd == sprmCHighlight){
		const COLOR *c = Ico(prl->operand[0]);
		if (c){
			int rgb;
			int r = c->red;
			int g = c->green;
			int b = c->blue;
			rgb = (r << 24) + (g << 16) + (b << 8);
			chp->bcolor = rgb; 
		}
		return 0;
	}

	// font size
	if (ismpd == sprmCHpsBi){
		USHORT *n = (USHORT *)(prl->operand);
		chp->size = *n;
		return 0;
	}

	// cpital letters
	if (ismpd == sprmCFSmallCaps){
		chp->allCaps = 
			ToggleOperand(doc, prl->operand[0]); 
		return 0;
	}

	// special chars 
	if (ismpd == sprmCFSpec){
		doc->prop.chp.sprmCFSpec = 
			ToggleOperand(doc, prl->operand[0]); 
		return 0;
	}

	if (ismpd == sprmCFOle2){
		BYTE *n = prl->operand;
		doc->prop.chp.sprmCFOle2 = *n;
		return 0;
	}
	
	if (ismpd == sprmCFObj){
		BYTE *n = prl->operand;
		doc->prop.chp.sprmCFObj = *n;
		return 0;
	}

	if (ismpd == sprmCFData){
		BYTE *n = prl->operand;
		doc->prop.chp.sprmCFData = *n;
		return 0;
	}

	// picture location 
	if (ismpd == sprmCPicLocation){
		LONG *n = (LONG *)prl->operand;
		doc->prop.chp.sprmCPicLocation = *n; 
		return 0;
	}



#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%02x", ismpd); 
#endif
	return 1;
}

int apply_paragraph_property(
		cfb_doc_t *doc, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	BYTE sgc = SprmSgc(prl->sprm);
	LOG("sgc: 0x%x, ismpd: 0x%02x", sgc, ismpd); 
#endif

	if (ismpd == sprmPIstd){
		USHORT *istd = (USHORT *)prl->operand;
#ifdef DEBUG
	LOG("paragraph istd: %d", *istd); 
#endif
		apply_style_properties(doc, *istd);
		return 0;
	}
	
	if (ismpd == sprmPDyaBefore){
		USHORT *n = (USHORT *)(prl->operand);
		doc->prop.pap.before = *n;
		return 0;;
	}

	if (ismpd == sprmPDyaAfter){
		USHORT *n = (USHORT *)(prl->operand);
		doc->prop.pap.after = *n;
		return 0;;
	}

	// paragraph justification
	if (ismpd == sprmPJc80 || ismpd == sprmPJc){
		BYTE *n = prl->operand;
		switch (*n) {
			case 0:  doc->prop.pap.just = justL; break;
			case 1:  doc->prop.pap.just = justC; break;
			case 2:  doc->prop.pap.just = justR; break;
			default: doc->prop.pap.just = justF; break;
		}
		return 0;
	}

	// table terminating paragraph mark
	if (ismpd == sprmPFTtp){
		BYTE *n = prl->operand;	
		if (*n)
			doc->prop.pap.TTP = fTrue;
		else
			doc->prop.pap.TTP = fFalse;
		return 0;
	}

	// inner table terminating paragraph mark
	if (ismpd == sprmPFInnerTtp){
		BYTE *n = prl->operand;	
		if (*n)
			doc->prop.pap.ITTP = fTrue;
		else
			doc->prop.pap.ITTP = fFalse;
		return 0;
	}

	// inner table cell mark
	if (ismpd == sprmPFInnerTableCell){
		BYTE *n = prl->operand;	
		if (*n)
			doc->prop.pap.ITC = fTrue;
		else
			doc->prop.pap.ITC = fFalse;
		return 0;
	}
	
	// in table depth
	if (ismpd == sprmPItap || ismpd == sprmPDtap){
		LONG *n = (LONG* )(prl->operand);
		if (n > 0)
			doc->prop.pap.Itap = *n;
		else
			doc->prop.pap.Itap = 0;

		return 0;
	}

	// spacing between lines
	if (ismpd == sprmPDyaLine){
		/* TODO:  spacing */
		return 0;
	};

#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%02x", ismpd); 
#endif
	return 1;
}

int apply_section_property(
		cfb_doc_t *doc, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	BYTE sgc = SprmSgc(prl->sprm);
	LOG("sgc: 0x%x, ismpd: 0x%02x", sgc, ismpd); 
#endif

	if (ismpd == sprmSXaPage){
		SHORT *n = (SHORT *)(prl->operand);
		doc->prop.sep.xaPage = *n;

		return 0;
	}

	if (ismpd == sprmSYaPage){
		SHORT *n = (SHORT *)(prl->operand);
		doc->prop.sep.yaPage = *n;

		return 0;
	}
	
	if (ismpd == sprmSDxaLeft){
		USHORT *n = (USHORT *)(prl->operand);
		doc->prop.sep.xaLeft = *n;

		return 0;
	}

	if (ismpd == sprmSDxaRight){
		USHORT *n = (USHORT *)(prl->operand);
		doc->prop.sep.xaRight = *n;

		return 0;
	}

	if (ismpd == sprmSDyaTop){
		USHORT *n = (USHORT *)(prl->operand);
		doc->prop.sep.yaTop = *n;

		return 0;
	}

	if (ismpd == sprmSDyaBottom){
		USHORT *n = (USHORT *)(prl->operand);
		doc->prop.sep.yaBottom = *n;

		return 0;
	}


#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%02x", ismpd); 
#endif
	return 1;
}

int apply_table_property(
		cfb_doc_t *doc, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	BYTE sgc = SprmSgc(prl->sprm);
	LOG("sgc: 0x%x, ismpd: 0x%02x", sgc, ismpd); 
#endif
	
	// table justification
	if (
			ismpd == sprmTJc90 ||
			ismpd == sprmTJc)
	{
		USHORT *n = (USHORT*)(prl->operand);
		switch (*n) {
			case 0:  doc->prop.trp.just = justL; break;
			case 1:  doc->prop.trp.just = justC; break;
			case 2:  doc->prop.trp.just = justR; break;
			default: doc->prop.trp.just = justL; break;
		}
		return 0;
	}

	// table header
	if (ismpd == sprmTTableHeader){
		BYTE *n = prl->operand;	
		if (*n)
			doc->prop.trp.header = fTrue;
		else
			doc->prop.trp.header = fFalse;
		return 0;
	}

	// table borders
	if (ismpd == sprmTTableBorders){
		struct TableBordersOperand *n = 
			(struct TableBordersOperand *)prl->operand;

		if (n->brcBottom.brcType)
			doc->prop.trp.bordB = fTrue;
		else
			doc->prop.trp.bordB = fFalse;
		
		if (n->brcTop.brcType)
			doc->prop.trp.bordT = fTrue;
		else
			doc->prop.trp.bordT = fFalse;
		
		if (n->brcLeft.brcType)
			doc->prop.trp.bordL = fTrue;
		else
			doc->prop.trp.bordL = fFalse;
		
		if (n->brcRight.brcType)
			doc->prop.trp.bordR = fTrue;
		else
			doc->prop.trp.bordR = fFalse;

		if (n->brcHorizontalInside.brcType)
			doc->prop.trp.bordH = fTrue;
		else
			doc->prop.trp.bordH = fFalse;

		if (n->brcVerticalInside.brcType)
			doc->prop.trp.bordV = fTrue;
		else
			doc->prop.trp.bordV = fFalse;
		
		return 0;
	}

	if (ismpd == sprmTTableBorders80){
		struct TableBordersOperand80 *n = 
			(struct TableBordersOperand80 *)prl->operand;

		if (n->cb != 0xFF &&
				n->brcBottom.brcType)
			doc->prop.trp.bordB = fTrue;
		else
			doc->prop.trp.bordB = fFalse;
		
		if (n->cb != 0xFF &&
				n->brcTop.brcType)
			doc->prop.trp.bordT = fTrue;
		else
			doc->prop.trp.bordT = fFalse;
		
		if (n->cb != 0xFF &&
				n->brcLeft.brcType)
			doc->prop.trp.bordL = fTrue;
		else
			doc->prop.trp.bordL = fFalse;
		
		if (n->cb != 0xFF &&
				n->brcRight.brcType)
			doc->prop.trp.bordR = fTrue;
		else
			doc->prop.trp.bordR = fFalse;

		if (n->cb != 0xFF &&
				n->brcHorizontalInside.brcType)
			doc->prop.trp.bordH = fTrue;
		else
			doc->prop.trp.bordH = fFalse;

		if (n->cb != 0xFF &&
				n->brcVerticalInside.brcType)
			doc->prop.trp.bordV = fTrue;
		else
			doc->prop.trp.bordV = fFalse;
		
		return 0;
	}

	// table cell borders
	if (ismpd == sprmTSetBrc||
			ismpd == sprmTSetBrc80)
	{
#ifdef DEBUG
	LOG("Table Cell Borders, ismpd: 0x%02x", ismpd); 
#endif
		struct TableBrc80Operand *n = 
			(struct TableBrc80Operand *)prl->operand;

		if (
				n->brc.brcType != 0xFF &&
				n->brc.brcType )
		{
				doc->prop.tcp.bordT = fFalse;
				doc->prop.tcp.bordL = fFalse;
				doc->prop.tcp.bordB = fFalse;
				doc->prop.tcp.bordR = fFalse;
				
				if ((n->bordersToApply & BordersToApplyTop) ==
						BordersToApplyTop)
					doc->prop.tcp.bordT = fTrue;

				if ((n->bordersToApply & BordersToApplyLeft) ==
						BordersToApplyLeft)
					doc->prop.tcp.bordL = fTrue;
				
				if ((n->bordersToApply & BordersToApplyBottom) ==
						BordersToApplyBottom)
					doc->prop.tcp.bordB = fTrue;

				if ((n->bordersToApply & BordersToApplyRight) ==
						BordersToApplyRight)
					doc->prop.tcp.bordR = fTrue;
		}
		
		return 0;
	}


	if (ismpd == sprmTCellBrcType){
#ifdef DEBUG
	LOG("Table Cell Borders: sprmTCellBrcType"); 
#endif
		struct TCellBrcTypeOperand *n = 
			(struct TCellBrcTypeOperand *)prl->operand;
		
		int cells = n->cb / 4; 
		int i;
		for (i = 0; i < cells; ++i) {
			/* TODO: handle cells */	
			if (n->rgBrcType[i])
					doc->prop.tcp.bordT = fTrue;
			if (n->rgBrcType[i+1])
					doc->prop.tcp.bordL = fTrue;
			if (n->rgBrcType[i+2])
					doc->prop.tcp.bordB = fTrue;
			if (n->rgBrcType[i+3])
					doc->prop.tcp.bordR = fTrue;
		}

		return 0;
	};

	if (ismpd == sprmTCellBrcTopStyle)
	{
#ifdef DEBUG
	LOG("Table Cell Borders: sprmTCellBrcTopStyle"); 
#endif
		struct BrcOperand *n = 
			(struct BrcOperand *)prl->operand;
		
			if (n->brc.brcType)
					doc->prop.tcp.bordT = fTrue;

		return 0;
	};

	if (ismpd == sprmTCellBrcBottomStyle)
	{
#ifdef DEBUG
	LOG("Table Cell Borders: sprmTCellBrcBottomStyle"); 
#endif
		struct BrcOperand *n = 
			(struct BrcOperand *)prl->operand;
		
			if (n->brc.brcType)
					doc->prop.tcp.bordB = fTrue;

		return 0;
	};

	if (ismpd == sprmTCellBrcLeftStyle)
	{
#ifdef DEBUG
	LOG("Table Cell Borders: sprmTCellBrcLeftStyle"); 
#endif
		struct BrcOperand *n = 
			(struct BrcOperand *)prl->operand;
		
			if (n->brc.brcType)
					doc->prop.tcp.bordL = fTrue;

		return 0;
	};

	if (ismpd == sprmTCellBrcRightStyle)
	{
#ifdef DEBUG
	LOG("Table Cell Borders: sprmTCellBrcRightStyle"); 
#endif
		struct BrcOperand *n = 
			(struct BrcOperand *)prl->operand;
		
			if (n->brc.brcType)
					doc->prop.tcp.bordR = fTrue;

		return 0;
	};

	// table defaults
	if (ismpd == sprmTDefTable){
#ifdef DEBUG
	LOG("Size of TDefTableOperand: %d", *((SHORT *)(prl->operand))); 
	LOG("NumberOfColumns: %d", prl->operand[2]); 
#endif
		struct TDefTableOperand *t = TDefTableOperandInit(prl);	
		if (t){
			struct TC80 *rgTc80 = 
				(struct TC80 *)t->rgTc80;
			doc->prop.trp.ncellx = t->NumberOfColumns;
			XAS *axas = (SHORT *)(t->rgdxaCenter);
			// first cell left indent = axas[0];
			/*! TODO: first cell left indent */
			int i;
			for (i = 0; i < t->NumberOfColumns; ++i) {
				XAS xas = axas[i+1];
				doc->prop.trp.cellx[i] = xas;

				// set borders
				bool bT = fFalse;
				bool bL = fFalse;
				bool bB= fFalse;
				bool bR = fFalse;
				if (t->rgTc80){	
					if (rgTc80[i].brcTop.brcType != 0xFF &&
					    rgTc80[i].brcTop.brcType > 0)
						bT = fTrue;
					if (rgTc80[i].brcLeft.brcType != 0xFF &&
					    rgTc80[i].brcLeft.brcType > 0)
						bL = fTrue;
					if (rgTc80[i].brcBottom.brcType != 0xFF &&
					    rgTc80[i].brcBottom.brcType > 0)
						bB = fTrue;
					if (rgTc80[i].brcRight.brcType != 0xFF &&
					    rgTc80[i].brcRight.brcType > 0)
						bR = fTrue;
				}
				doc->prop.trp.cbordT[i] = bT;
				doc->prop.trp.cbordL[i] = bL;
				doc->prop.trp.cbordB[i] = bB;
				doc->prop.trp.cbordR[i] = bR;

			//TDefTableOperandFree(t);
#ifdef DEBUG
	LOG("Column %d has XAS: %d, borders: %d:%d:%d:%d", i-1, xas, bT, bL, bB, bR); 
#endif
			}
		}

		
		return 0;
	}


#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%02x", ismpd); 
#endif
	return 1;
}

int apply_picture_property(
		cfb_doc_t *doc, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	LOG("ismpd: 0x%02x", ismpd); 
#endif
	
	if (
			ismpd == sprmPicBrcTop80 ||
			ismpd == sprmPicBrcLeft80 ||
			ismpd == sprmPicBrcBottom80 ||
			ismpd == sprmPicBrcRight80
			)
	{
		/* TODO: set no borders as default */
		
		struct Brc80 *t =
			(struct Brc80 *)prl->operand;

		if (t->brcType != 0xFF && t->brcType){
			switch (ismpd) {
				case sprmPicBrcTop80:
					/* TODO: set borders */
					break;
				case sprmPicBrcLeft80:
					/* TODO: set borders */
					break;
				case sprmPicBrcBottom80:
					/* TODO: set borders */
					break;
				case sprmPicBrcRight80:
					/* TODO: set borders */
					break;

				default:
					break;
			
			}
		}

		return 0;
	}
	
	if (
			ismpd == sprmPicBrcTop ||
			ismpd == sprmPicBrcLeft ||
			ismpd == sprmPicBrcBottom ||
			ismpd == sprmPicBrcRight
			)
	{
		/* TODO: set no borders as default */

		struct BrcOperand *t =
			(struct BrcOperand *)prl->operand;

		if (t->brc.brcType){
			switch (ismpd) {
				case sprmPicBrcTop:
					/* TODO: set borders */
					break;
				case sprmPicBrcLeft:
					/* TODO: set borders */
					break;
				case sprmPicBrcBottom:
					/* TODO: set borders */
					break;
				case sprmPicBrcRight:
					/* TODO: set borders */
					break;

				default:
					break;
			
			}
		}

		return 0;
	}


#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%02x", ismpd); 
#endif
	return 1;
}
