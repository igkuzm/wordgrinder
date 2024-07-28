/**
 * File              : direct_character_formatting.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 27.05.2024
 * Last Modified Date: 28.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/doc.h"
#include "../include/libdoc/prl.h"
#include "../include/libdoc/apply_properties.h"
#include "../include/libdoc/direct_character_formatting.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_to_default(cfb_doc_t *doc){
	CHP *chp = &(doc->prop.chp);
	memset(chp, 0, sizeof(CHP));
	
	chp->fBold      = doc->prop.pap_chp.fBold;
	chp->fUnderline = doc->prop.pap_chp.fUnderline;
	chp->fItalic    = doc->prop.pap_chp.fItalic;
	chp->font       = doc->prop.pap_chp.font;
	chp->size       = doc->prop.pap_chp.size;
	chp->fcolor     = doc->prop.pap_chp.fcolor;
	chp->bcolor     = doc->prop.pap_chp.bcolor;
	chp->allCaps    = doc->prop.pap_chp.allCaps;
}

static int callback(void *userdata, struct Prl *prl);
/* 2.4.6.2 Direct Character Formatting
 * This section specifies how to find the properties applied
 * directly to a given character position cp. The
 * result will be an array of Prl elements that specify the
 * property modifications to be applied.
 * Additional formatting and properties can affect that cp
 * as well, if a style is applied. To determine the
 * full set of properties, including those from styles, see
 * section 2.4.6.6 Determining Formatting
 * Properties. */
void direct_character_formatting(
		cfb_doc_t *doc, ULONG fc, struct Pcd *pcd)
{
#ifdef DEBUG
	LOG("start");
#endif

	set_to_default(doc);

/* 1. Follow the algorithm from Retrieving Text. From step 5
 * or 6, determine the offset in the
 * WordDocument Stream where text was found. Call this
 * offset fc. Also remember from step 4, the
 * Pcd. If the algorithm from Retrieving Text specifies cp
 * is invalid, leave the algorithm. */
	
/* 2. Read a PlcBteChpx at offset FibRgFcLcb97.fcPlcfBteChpx
 * in the Table Stream, and of size
 * FibRgFcLcb97.lcbPlcfBteChpx.*/
	struct PlcBteChpx *plcbteChpx = doc->plcbteChpx;

/* 3. Find the largest i such that plcbteChpx.aFc[i] ≤ fc.
 * If the last element of plcbteChpx.aFc is less
 * than or equal to fc, then cp is outside the range of
 * character positions in this document, and is
 * not valid. Read a ChpxFkp at offset aPnBteChpx[i].pn *512
 * in the WordDocument Stream. */
	int i;
	for (i=0; plcbteChpx->aFc[i] <= fc;)
		i++;
	i--;

#ifdef DEBUG
	LOG("plcbteChpx->aFc[%d]: %d", 
			i, plcbteChpx->aFc[i]);
#endif

	if (plcbteChpx->aFc[doc->plcbteChpxNaFc - 1] <= fc){
		ERR("plcbteChpx->aFc[%d]: %d - cp is outside the range "
				"of character positions in this document, and is "
				"not valid", 
				doc->plcbtePapxNaFc - 1,
				plcbteChpx->aFc[doc->plcbteChpxNaFc - 1]);
		return;
	}

	ULONG chpxFkp_fc = pnFkpChpx_pn(
					doc->plcbteChpx->aPnBteChpx[i]) * 512;
#ifdef DEBUG
	LOG("chpxFkp offset: %d", chpxFkp_fc);
#endif

	struct ChpxFkp chpxFkp;
	BYTE buf[512];	
	chpxFkp_init(&chpxFkp, buf, doc->WordDocument, 
				chpxFkp_fc);

/* 4. Find the largest j such that ChpxFkp.rgfc[j] ≤ fc. If
 * the last element of ChpxFkp.rgfc is less than
 * or equal to fc, then cp is outside the range of character
 * positions in this document, and is not
 * valid. Find a Chpx at offset ChpxFkp.rgb[i] in ChpxFkp.*/
	int j;
	for (j = 0; chpxFkp.rgfc[j] <= fc;)
		j++;
	j--;

	if (chpxFkp.rgfc[chpxFkp.crun] <= fc){
		ERR("chpxFkp->rgfc[%d]: %d - cp is outside the range "
				"of character positions in this document, and is "
				"not valid", 
				chpxFkp.crun,
				chpxFkp.rgfc[chpxFkp.crun]);
		return;
	}

	ULONG offset = chpxFkp.rgb[j] * 2 + chpxFkp_fc;
	BYTE cb;
	fseek(doc->WordDocument, offset,
		 	SEEK_SET);
	fread(&cb, 1, 1,
		 	doc->WordDocument);
#ifdef DEBUG
	LOG("cb: %d", cb);
#endif

	/* GrpPrl has size of chpx.cb */
	BYTE grpprl[cb];	
	fread(grpprl, cb, 1, 
			doc->WordDocument);

#ifdef DEBUG
	char str[BUFSIZ] = "grpprl: ";
	for (int i = 0; i < cb; ++i) {
		STRFCAT(str, "%d ", grpprl[i]);	
	}
	LOG("%s", str);
#endif

/* 5. The grpprl within the Chpx is an array of Prls that
 * specifies the direct properties of this character.*/
	parse_grpprl(
			grpprl, 
			cb, 
			doc, callback);

/* 6. Additionally, apply Pcd.Prm which specifies additional
 * properties for this text. If Pcd.Prm is a Prm0
 * and the Sprm specified within Prm0 modifies a character
 * property (a Sprm with an sgc value of 2), append a single 
 * Prl made of the Sprm and value in
 * that Prm0 to the array of Prls from the
 * previous step. If Pcd.Prm is a Prm1, append any Sprms
 * that modify character properties from the
 * array of Prls specified by Prm1.
 */

#ifdef DEBUG
	LOG("done");
#endif
}	

int callback(void *userdata, struct Prl *prl){
	// parse properties
	cfb_doc_t *doc = userdata;
	apply_property(doc, 0, prl);
	return 0;
}
