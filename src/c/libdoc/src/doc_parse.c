/**
 * File              : doc_parse.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 28.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "../include/libdoc.h"
#include "../include/libdoc/paragraph_boundaries.h"
#include "../include/libdoc/retrieving_text.h"
#include "../include/libdoc/direct_character_formatting.h"
#include "../include/libdoc/direct_paragraph_formatting.h"

int callback(void *d, struct Prl *prl){
	printf("PRL with sprm: %d \n", prl->sprm);	
	return 0;
}

CP parse_range_cp(cfb_doc_t *doc, CP cp, CP lcp,
		void *user_data,
		int (*callback)(void *user_data, ldp_t *p, int ch))
{
	while (cp <= lcp && cp < doc->fib.rgLw97->ccpText){
		get_char_for_cp(doc, cp, user_data,
				callback);
		cp++;
	}
	return cp;
}

CP parse_table_row(cfb_doc_t *doc, CP cp, CP lcp,
		void *user_data,
		int (*callback)(void *user_data, ldp_t *p, int ch))
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
			cp = parse_range_cp(doc, cp, lcp, user_data, 
					callback);
		}
	}
	return cp;
}

int doc_parse(const char *filename, void *user_data,
		int (*main_document)(void *user_data, ldp_t *p, int ch),
		int (*footnotes)(void *user_data, ldp_t *p, int ch),
		int (*headers)(void *user_data, ldp_t *p, int ch))
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

/* 2.3.1 Main Document
 * The main document contains all content outside any of 
 * the specialized document parts, including
 * anchors that specify where content from the other 
 * document parts appears.
 * The main document begins at CP zero, and is 
 * FibRgLw97.ccpText characters long.
 * The last character in the main document MUST be a 
 * paragraph mark (Unicode 0x000D).*/
for (cp = 0; cp < doc.fib.rgLw97->ccpText; ) {
	
	// get table row and cell boundaries and apply props
	CP lcp = last_cp_in_row(&doc, cp);
	if (lcp != CPERROR){
		// this CP is in table
		cp = parse_table_row(&doc, cp, lcp, user_data, 
				main_document);

	} else {
		// get paragraph boundaries and apply props
		CP lcp = last_cp_in_paragraph(&doc, cp); 
		
		// iterate cp
		cp = parse_range_cp(&doc, cp, lcp, user_data, 
				main_document);
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
for (;cp < doc.fib.rgLw97->ccpFtn; ++cp) {
	get_char_for_cp(&doc, cp, user_data,
			footnotes);
}

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
for (;cp < doc.fib.rgLw97->ccpHdd; ++cp) {
	get_char_for_cp(&doc, cp, user_data,
			headers);
}

doc_close(&doc);

#ifdef DEBUG
	LOG("done");
#endif
	return 0;
}
