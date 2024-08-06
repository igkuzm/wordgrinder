/**
 * File              : doc_parse.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 07.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "../include/libdoc.h"
#include "../include/libdoc/paragraph_boundaries.h"
#include "../include/libdoc/retrieving_text.h"
#include "../include/libdoc/direct_section_formatting.h"
#include "../include/libdoc/style_properties.h"
#include "../include/libdoc/section_boundaries.h"
#include "../include/libdoc/direct_character_formatting.h"
#include "../include/libdoc/direct_paragraph_formatting.h"
#include <stdio.h>

int callback(void *d, struct Prl *prl){
	printf("PRL with sprm: %d \n", prl->sprm);	
	return 0;
}

CP parse_range_cp(cfb_doc_t *doc, CP cp, CP lcp,
		void *user_data,
		DOC_PART part,
		int (*callback)(void *user_data, DOC_PART part, ldp_t *p, int ch))
{
	while (cp <= lcp && cp < doc->fib.rgLw97->ccpText){
		get_char_for_cp(doc, cp, user_data, part,
				callback);
		cp++;
	}
	return cp;
}

CP parse_table_row(cfb_doc_t *doc, CP cp, CP lcp,
		void *user_data,
		DOC_PART part,
		int (*callback)(void *user_data, DOC_PART part, ldp_t *p, int ch))
{
	while (cp <= lcp && cp < doc->fib.rgLw97->ccpText){
		//get cell
		CP clcp = last_cp_in_row(doc, cp);
		if (clcp == CPERROR)
			return cp;
		
		// parse cell
		while (cp <= clcp && cp < doc->fib.rgLw97->ccpText){

			// parse paragraph
			CP lcp = last_cp_in_paragraph(doc, cp); 
			cp = parse_range_cp(doc, cp, lcp, user_data, part, 
					callback);
		}
	}
	return cp;
}

static void _parse_styles(cfb_doc_t *doc, void *user_data, 
		int (*styles)(void *user_data, STYLE *s))
{
	// parse styles
	USHORT cstd = doc->STSH->lpstshi->stshi->stshif.cstd;
	
	int i, index = 0;
	for (i = 0; i < doc->lrglpstd && index < cstd;) {

		void *ptr = &(doc->STSH->rglpstd[i]); 
		// read cbStd
		USHORT *cbStd = (USHORT *)ptr;
#ifdef DEBUG
		LOG("SDT at index %d size: %d", index, *cbStd);
#endif
		
		struct LPStd *LPStd = 
			(struct LPStd *)&(doc->STSH->rglpstd[i]);

		apply_style_properties(doc, index);
				
		if (LPStd->cbStd == 0){
			i += 2;
			index++;
			continue;
		}

		struct STD *STD = (struct STD *)LPStd->STD;
		
		STYLE s;
		memset(&s, 0, sizeof(STYLE));
		s.s = index;
		s.chp = doc->prop.chp;
		s.pap_chp = doc->prop.pap_chp;

		USHORT *p = NULL;

		// check if STD->Stdf has StdfPost2000;
		struct STSH *STSH = doc->STSH;
		struct STSHI *STSHI = STSH->lpstshi->stshi;
		USHORT cbSTDBaseInFile = STSHI->stshif.cbSTDBaseInFile;
		
		if (cbSTDBaseInFile == 0x000A){
			// no StdfPost2000
			p = (USHORT *)((struct STD_noStdfPost2000 *)STD)->xstzName_grLPUpxSw;

		} else if (cbSTDBaseInFile == 0x0012){
			// has StdfPost2000
			p = (USHORT *)STD->xstzName_grLPUpxSw;
		
		} else {
			ERR("cbSTDBaseInFile");
			i += *cbStd + 2;
			index++;
			continue;
		}

		// get name
		char str[BUFSIZ];
		memset(str, 0, BUFSIZ);
		
		if (*p){
			USHORT *xstz = p+1;
			_utf16_to_utf8(xstz, *p, str);
#ifdef DEBUG
	LOG("%s", str);
#endif
			strncpy(s.name, str, sizeof(s.name) - 1);
			s.lname = strlen(s.name);
		}

		struct StdfBase *stdfBase = (struct StdfBase *)STD;
		USHORT istdBase = StdfBaseIstdBase(stdfBase);

		s.sbedeon = istdBase;

		styles(user_data, &s);

		// iterate
		// skeep next cbStd bytes and  2 bytes of cbStd itself
		i += *cbStd + 2;
		index++;
	}
}

int doc_parse(const char *filename, void *user_data,
		int (*styles)(void *user_data, STYLE *s),
		int (*text)(void *user_data, DOC_PART part, ldp_t *p, int ch))
{
#ifdef DEBUG
	LOG("start");
#endif
	int ret, cp, i;

	// get CFB
	struct cfb cfb;
	ret = cfb_open(&cfb, filename);
	if (ret)
		return ret;
	
	// Read the DOC Streams
	cfb_doc_t doc;
	ret = doc_read(&doc, &cfb);
	if (ret)
		return ret;

	doc.prop.data = &doc;
	FibRgFcLcb97 *rgFcLcb97 = (FibRgFcLcb97 *)(doc.fib.rgFcLcb);

	// parse styles
	_parse_styles(&doc, user_data, styles);

/* 2.3.1 Main Document
 * The main document contains all content outside any of 
 * the specialized document parts, including
 * anchors that specify where content from the other 
 * document parts appears.
 * The main document begins at CP zero, and is 
 * FibRgLw97.ccpText characters long.
 * The last character in the main document MUST be a 
 * paragraph mark (Unicode 0x000D).*/

	// for each section in word document
	for (i=0; i < doc.plcfSedNaCP; ++i){
		CP first = doc.plcfSed->aCP[i];
		CP last;
		if (i < doc.plcfSedNaCP)
			last = doc.plcfSed->aCP[i+1];
		else
			last = doc.fib.rgLw97->ccpText;
		
		// apply section prop
		direct_section_formatting(&doc, i);
		
		// parse section
		for (cp = first; cp < last; ) {
			
			// get table row and cell boundaries and apply props
			CP lcp = last_cp_in_row(&doc, cp);
			if (lcp != CPERROR){
				// this CP is in table
				cp = parse_table_row(&doc, cp, lcp, user_data, MAIN_DOCUMENT, 
						text);

			} else {
				// get paragraph boundaries and apply props
				CP lcp = last_cp_in_paragraph(&doc, cp); 
				
				// iterate cp
				cp = parse_range_cp(&doc, cp, lcp, user_data, MAIN_DOCUMENT, 
						text);
			}
		}	
	}

/* 2.3.2 Footnotes
 * The footnote document contains all of the content in the
 * footnotes. It begins at the CP immediately
 * following the Main Document, and is FibRgLw97.ccpFtn
 * characters long.
 * The locations of individual footnotes within the footnote
 * document are specified by a PlcffndTxt whose
 * location is specified by the fcPlcffndTxt member of
 * FibRgFcLcb97. The locations of the footnote
 * reference characters in the Main Document are specified
 * by a PlcffndRef whose location is specified
 * by the fcPlcffndRef member of FibRgFcLcb97. */

/*for (;cp < doc.fib.rgLw97->ccpFtn; ++cp) {*/
	/*get_char_for_cp(&doc, cp, user_data, FOOTNOTES,*/
			/*text);*/
/*}*/

/* by the fcPlcffndRef member of FibRgFcLcb97.
 * 2.3.3 Headers
 * The header document contains all content in headers and
 * footers as well as the footnote and endnote
 * separators. It begins immediately after the footnote
 * document and is FibRgLw97.ccpHdd characters
 * long.
 * The header document is split into text ranges called
 * stories, as specified by PlcfHdd. Each story
 * specifies the contents of a single header, footer, or
 * footnote/endnote separator. If a story is non-
 * empty, it MUST end with a paragraph mark that serves as a
 * guard between stories. This paragraph
 * mark is not considered part of the story contents (that
 * is, if the story contents require a paragraph
 * mark themselves, a second paragraph mark MUST be used).
 * Stories are considered empty if they have no contents and
 * no guard paragraph mark. Thus, an empty
 * story is indicated by the beginning CP, as specified in
 * PlcfHdd, being the same as the next CP in PlcfHdd */
/*for (;cp < doc.fib.rgLw97->ccpHdd; ++cp) {*/
	/*get_char_for_cp(&doc, cp, user_data,*/
			/*HEADERS, text);*/
/*}*/

doc_close(&doc);

#ifdef DEBUG
	LOG("done");
#endif
	return 0;
}
