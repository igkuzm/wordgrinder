/**
 * File              : row_boundaries.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 17.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/cell_boundaries.h"
#include "../include/libdoc/direct_paragraph_formatting.h"
#include "../include/libdoc/paragraph_boundaries.h"

/* 2.4.5 Determining Row Boundaries */
CP last_cp_in_row(cfb_doc_t *doc, CP cp)
{
	cp = last_cp_in_paragraph(doc, cp);

	if (doc->prop.pap.Itap > 0) {
		
		// check if TTP
		if (doc->prop.pap.TTP)
			return cp;

		LONG itapOrig = doc->prop.pap.Itap;
		
		// get next paragraph last CP 
		while (cp < doc->fib.rgLw97->ccpText){
			
			memset(&doc->prop.trp, 0, sizeof(TRP));
			
			cp = last_cp_in_paragraph(doc, cp+1);
		
			// check if TTP
			if (doc->prop.pap.TTP)
				return cp;
			
			if (itapOrig == doc->prop.pap.Itap &&
					doc->prop.pap.ITTP)
				return cp;
		}
	}

	return CPERROR;
}

