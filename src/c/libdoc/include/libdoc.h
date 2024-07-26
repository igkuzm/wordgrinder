/**
 * File              : libdoc.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 27.05.2024
 * Last Modified Date: 26.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/**
 * include/libdoc.h
 * Copyright (c) 2024 Igor V. Sementsov <ig.kuzm@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBDOC_H
#define LIBDOC_H

#ifdef __cplusplus
extern "C"{
#endif

#include "mswordtype.h"

#define INLINE_PICTURE		 0x0001 //
#define FLOATING_PICTURE	 0x0008 //
#define FOOTNOTE_MARK			 0x0002 // - footnote mark
#define FOOTNOTE_MARK			 0x0002 // - footnote mark
#define CELL_MARK			     0x0007 // - table separator
#define HORIZONTALTAB			 0x0009 // - Horizontal tab
#define LINEBREAK			     0x000B // - hard return
#define PAGEBREAK			     0x000C // - page break
#define PARAGRAPH_MARK		 0x000D // - marks an end of paragraph
#define HYPHEN			       0x001E // (-) separates the
																	// chapter number and
																	// caption number
																	// IS2 for some reason
																	// means short defis in
																	// Word
#define PERIOD			       0x002E // (.) separates the
																	// chapter number and the
																	// caption number
#define COLON			         0x003A // (:) separates the
																	// chapter number and the
																	// caption number
#define EN_DASH			       0x2013 // (–) separates the
																	// chapter number and the
																	// caption number
#define EM_DASH			       0x2014 // (–) separates the
																	// chapter number and the
																	// caption number
#define SOFT_HYPEN			   0x001F // - soft hyphen in Word
#define HYPERLINK_START		 0x0013 // - start embedded hyperlink
#define HYPERLINK_SEPARATE 0x0014 // - separate hyperlink
																	// URL from text
#define HYPERLINK_END			 0x0015 // - end embedded hyperlink

/* structure to save properties */
typedef struct libdoc_prop {
	/* data */
	DOP dop;
	SEP sep;
	PAP pap;
	CHP pap_chp;
	CHP chp;
	TRP trp;
	TCP tcp;
	void *data;
} ldp_t;

/* open MS-DOC file and run callbacks for characters in 
 * main document, footnotes and headers */
int doc_parse(const char *filename, void *user_data,
		int (*main_document)(void *user_data, ldp_t *p, int ch),
		int (*footnotes)(void *user_data, ldp_t *p, int ch),
		int (*headers)(void *user_data, ldp_t *p, int ch));

void doc_get_picture(
		int ch, ldp_t *p, void *userdata,
		void (*callback)(struct picture *pic, void *userdata));

#ifdef __cplusplus
}
#endif

#endif /* ifndef LIBDOC_H */
// vim:ft=c
