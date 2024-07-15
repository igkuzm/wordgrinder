/**
 * File              : apply_properties.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 28.05.2024
 * Last Modified Date: 30.05.2024
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
	LOG("sgc: 0x%02x", sgc); 
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
	LOG("ismpd: 0x%04x", ismpd); 
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

#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%04x", ismpd); 
#endif
	return 1;
}

int apply_paragraph_property(
		cfb_doc_t *doc, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	LOG("ismpd: 0x%04x", ismpd); 
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


#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%04x", ismpd); 
#endif
	return 1;
}

int apply_section_property(
		cfb_doc_t *doc, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	LOG("ismpd: 0x%04x", ismpd); 
#endif


#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%04x", ismpd); 
#endif
	return 1;
}

int apply_table_property(
		cfb_doc_t *doc, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	LOG("ismpd: 0x%04x", ismpd); 
#endif
	
	// table justification
	if (ismpd == sprmTJc90){
		USHORT *n = (USHORT*)(prl->operand);
		switch (*n) {
			case 0:  doc->prop.trp.just = justL; break;
			case 1:  doc->prop.trp.just = justC; break;
			case 2:  doc->prop.trp.just = justR; break;
			default: doc->prop.trp.just = justL; break;
		}
		return 0;
	}


#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%04x", ismpd); 
#endif
	return 1;
}

int apply_picture_property(
		cfb_doc_t *doc, struct Prl *prl)
{
	USHORT ismpd = SprmIspmd(prl->sprm);
#ifdef DEBUG
	LOG("ismpd: 0x%04x", ismpd); 
#endif


#ifdef DEBUG
	LOG("no rule to parse ismpd: 0x%04x", ismpd); 
#endif
	return 1;
}
