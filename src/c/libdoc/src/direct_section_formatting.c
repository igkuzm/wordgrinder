/**
 * File              : direct_section_formatting.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.08.2024
 * Last Modified Date: 05.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/direct_section_formatting.h"
#include "../include/libdoc/prl.h"
#include "../include/libdoc/apply_properties.h"
#include <string.h>
#include <stdio.h>

static int callback(void *userdata, struct Prl *prl);

void direct_section_formatting(cfb_doc_t *doc, int index)
{
#ifdef DEBUG
	LOG("start");
#endif

	if (index >= doc->plcfSedNaCP){
		ERR("no section with index: %d", index);
		return;
	}

	memset(&doc->prop.sep, 0, sizeof(SEP));
	
	LONG off = doc->plcfSed->aSed[index].fcSepx;
	fseek(doc->WordDocument, off, SEEK_SET);
	
	// read size of grpprl
	SHORT cb;
	/*LOG("cb: %d", cb);*/
	fread(&cb, 2, 1,
			doc->WordDocument);
	if (cb <= 0){
		ERR("unknown error");
		return;
	}

	// get grpprl
	BYTE grpprl[cb];
	fread(grpprl, 1, cb,
			doc->WordDocument);

	// parse grpprl
	parse_grpprl(
			grpprl, 
			cb, 
			doc, callback);
}

int callback(void *userdata, struct Prl *prl){
	// parse properties
	cfb_doc_t *doc = userdata;
	
	USHORT ismpd = SprmIspmd(prl->sprm);
	apply_property(doc, 2, prl);
	return 0;
}
