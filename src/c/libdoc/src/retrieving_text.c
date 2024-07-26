/**
 * File              : retrieving_text.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 26.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "../include/libdoc/retrieving_text.h"
#include "../include/libdoc/direct_character_formatting.h"

static void check_marks(cfb_doc_t *doc, int ch)
{
	// check if CELL_MARK - drop props
	if (ch == CELL_MARK){
		//if (doc->prop.trp.TTP || doc->prop.trp.ITTP)
			//memset(&(doc->prop.trp), 0, sizeof(TRP));
		//memset(&(doc->prop.tcp), 0, sizeof(TCP));
		//memset(&(doc->prop.pap), 0, sizeof(PAP));
	}
}

void get_char_for_cp(cfb_doc_t *doc, CP cp,
		void *user_data,
		int (*callback)(void *user_data, ldp_t *p, int ch)		
		)
{
	struct PlcPcd *PlcPcd = &(doc->clx.Pcdt->PlcPcd);
	DWORD off; // ofset of WordDocument where text is located

/* The Clx contains a Pcdt, and the Pcdt contains a PlcPcd.
 * Find the largest i such that PlcPcd.aCp[i] ≤ cp. As with
 * all Plcs, the elements of PlcPcd.aCp are sorted in
 * ascending order.  Recall from the definition of a Plc
 * that the aCp array has one more element than the aPcd
 * array.  Thus, if the last element of PlcPcd.aCp is less
 * than or equal to cp, cp is outside the range of valid
 * character positions in this document
 */
	int i;
	for (i=0; PlcPcd->aCp[i] <= cp;)
		i++;
	i--;	

/*
 * PlcPcd.aPcd[i] is a Pcd. Pcd.fc is an FcCompressed that
 * specifies the location in the WordDocument Stream of the
 * text at character position PlcPcd.aCp[i].
 */
	struct Pcd *pcd = &(PlcPcd->aPcd[i]);
	struct FcCompressed fc = PlcPcd->aPcd[i].fc;	
	if (FcCompressed(fc)){
/*
 * If FcCompressed.fCompressed is 1, the character at
 * position cp is an 8-bit ANSI character at offset
 * (FcCompressed.fc / 2) + (cp - PlcPcd.aCp[i]) in the
 * WordDocument Stream, unless it is one of the special
 * values in the table defined in the description of
 * FcCompressed.fc. This is 
 * to say that the text at character position PlcPcd.aCP[i]
 * begins at offset FcCompressed.fc / 2 in the WordDocument
 * Stream and each character occupies one byte.
*/			
		//ANSI
		off = (FcValue(fc) / 2) + (cp - PlcPcd->aCp[i]);

		// get properties
		direct_character_formatting(doc, off, pcd);
		doc->prop.chp.cp = cp;

		fseek(doc->WordDocument, off, SEEK_SET);	
		int ch;
		fread(&ch, 1, 1, 
					doc->WordDocument);
		
		// check special chars
		int sch = FcCompressedSpecialChar_get(ch);
		if (sch)
			callback(user_data, &doc->prop, sch);
		else
			callback(user_data, &doc->prop, ch);
		
		//check_marks(doc, ch);

	} else {
/*
 * If FcCompressed.fCompressed is zero, the character at
 * position cp is a 16-bit Unicode character at offset
 * FcCompressed.fc + 2(cp - PlcPcd.aCp[i]) in the
 * WordDocument Stream. This is to say that the text at
 * character position PlcPcd.aCP[i] begins at offset
 * FcCompressed.fc in the 
 * WordDocument Stream and each character occupies two
 * bytes.
*/			
		//UNICODE 16
		off = FcValue(fc) + 2*(cp - PlcPcd->aCp[i]);

		// get properties
		direct_character_formatting(doc, off, pcd);
		doc->prop.chp.cp = cp;
		
		fseek(doc->WordDocument, off, SEEK_SET);	
		WORD u;
		fread(&u, 2, 1, 
					doc->WordDocument);
		if (doc->biteOrder){
			u = bswap_16(u);
		}
		char utf8[4]={0};
		//sprintf(utf8, "0x%x ", u);
		_utf16_to_utf8(&u, 1, utf8);
		//if (doc->biteOrder){
				//u = bswap_16(u);
		//}
		if (u < 0x010){
			// first byte in uint16 is 00
			if (u > 0x1f && u < 0x7f) {
				//simple ANSI
				int ch;
				fread(&ch, 1, 1,
						doc->WordDocument);
		
				callback(user_data, &doc->prop, ch);
			} else {
				//this is a mark
				callback(user_data, &doc->prop, u);
				//check_marks(doc, u);
			}
		} else if (u != 0xfeff) {
			char utf8[4]={0};
			_utf16_to_utf8(&u, 1, utf8);
			for (i = 0; i < 4; ++i) {
				callback(user_data, &doc->prop, utf8[i]);
			}
		}
	}
}
