/**
 * File              : direct_paragraph_formatting.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 24.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
*/

#include "../include/libdoc/direct_paragraph_formatting.h"
#include "../include/libdoc/apply_properties.h"
#include "../include/libdoc/style_properties.h"
#include "../include/mswordtype.h"
#include "memread.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void set_to_default(cfb_doc_t *doc){
	
	PAP *pap = &(doc->prop.pap);
	memset(pap, 0, sizeof(PAP));

	CHP *chp = &(doc->prop.pap_chp);
	memset(chp, 0, sizeof(CHP));
	
}

static int callback(void *userdata, struct Prl *prl);
/* 2.4.6.1 Direct Paragraph Formatting
 * This section explains how to find the properties applied
 * directly (as opposed to through a style, for
 * example) to a paragraph, given a character position cp
 * within it. The properties are found as an array
 * of Prl elements. */
void direct_paragraph_formatting(
		cfb_doc_t *doc, int k, struct PapxFkp *papxFkp,
		ULONG of, struct Pcd *pcd)
{
#ifdef DEBUG
	LOG("start for PapxFkp at offset: %d for k: %d",
			of, k);
#endif

	set_to_default(doc);

/* 1. Follow the algorithm from Determining Paragraph
 * Boundaries for finding the character position of
 * the last character in the paragraph to completion. From
 * step 5, remember the PapxFkp and k.
 * From step 4, remember the offset in the WordDocument
 * Stream at which PapxFkp was read. Let
 * this offset be called of. From step 2 remember the Pcd.
 * If the algorithm from Determining
 * Paragraph Boundaries specifies that cp is invalid, leave
 * the algorithm. */

/* 2. Find a BxPap at PapxFkp.rgbx[k]. Find a PapxInFkp at
 * offset of + 2*BxPap.bOffset */
	ULONG offset = of + (2 * papxFkp->rgbx[k].bOffset);
#ifdef DEBUG
	LOG("papxFkp->rgbx[k].bOffset: %d", 
			papxFkp->rgbx[k].bOffset);
	LOG("PapxInFkp at offset: %d", offset);
#endif
	fseek(doc->WordDocument, offset,
			SEEK_SET);
	
/* 3. Find a GrpprlAndIstd in the PapxInFkp from step 2.
 * The offset and size of the GrpprlAndIstd
 * is instructed by the first byte of the PapxInFkp, as
 * detailed at PapxInFkp. */

	int size = 0;
	BYTE cb;
	fread(&cb, 1, 1,doc->WordDocument);
#ifdef DEBUG
	LOG("PapxInFkp cb: %d", cb);
#endif
	if (cb){
		// cb is not 0
		size += 2*cb - 1;
	} else {
		// cb is 0
		BYTE cb_;
		fread(&cb_, 1, 1,
				doc->WordDocument);
#ifdef DEBUG
	LOG("PapxInFkp cb': %d", cb_);
#endif
		if (cb_ < 1) {
			// error
			ERR("cb' MUST be at least 1");
			return;
		}
		size += 2*cb_; 
	}

	struct GrpPrlAndIstd *grpPrlAndIstd = ALLOC(size, 
			ERR("malloc"); return);
	fread(grpPrlAndIstd, size, 1,
			doc->WordDocument);

#ifdef DEBUG
	LOG("Istd: %d", grpPrlAndIstd->istd);
	char str[BUFSIZ] = "grpprl data: ";
	for (int i = 0; i < size-2; ++i) {
		STRFCAT(str, "%d ", grpPrlAndIstd->grpprl[i]);	
	}
	LOG("%s", str);
#endif
	
	// apply style properties
	apply_style_properties(doc, grpPrlAndIstd->istd);	

/* 4. Find the grpprl within the GrpprlAndIstd. This is an
 * array of Prl elements that specifies the
 * direct properties of this paragraph. */
	parse_grpprl(
			grpPrlAndIstd->grpprl, 
			size-2, 
			doc, callback);

/* 5. Finally Pcd.Prm specifies further property
 * modifications that apply to this paragraph. If Pcd.Prm
 * is a Prm0 and the Sprm specified within Prm0 modifies a
 * paragraph property, append to the
 * array of Prl elements from the previous step a single Prl
 * made of the Sprm and value in Prm0. if
 * Pcd.Prm is a Prm1, append to the array of Prl elements
 * from the previous step any Sprm
 * structures that modify paragraph properties within the
 * array of Prl elements specified by Prm1. */

	free(grpPrlAndIstd);
}
int callback(void *userdata, struct Prl *prl){
	// parse properties
	cfb_doc_t *doc = userdata;
	apply_property(doc, 1, prl);
	return 0;
}
