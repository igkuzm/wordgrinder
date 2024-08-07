/**
 * File              : doc2txt.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 27.05.2024
 * Last Modified Date: 07.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <stdio.h>
#include "../include/libdoc.h"
#include "../ms-cfb/log.h"

/* read MS-DOC and print it's content */

int styles(void *, STYLE *s);
int text(void *, DOC_PART,  ldp_t*, int);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s file.doc\n\n", argv[0]);
		return 0;
	}	

	int ret = doc_parse(
			argv[1], 
			NULL, 
			styles,
			text);

	return ret;
}

static void picture(struct picture *pic, void *d){
	fprintf(stderr, "PIC len: %d\n", pic->len);
}

int text(void *d, DOC_PART part, ldp_t *p, int ch){

/* Following symbols below 32 are allowed inside paragraph:
0x0002 - footnote mark
0x0007 - table separator (converted to tabmode)
0x0009 - Horizontal tab ( printed as is)
0x000B - hard return
0x000C - page break
0x000D - return - marks an end of paragraph
0x001E - IS2 for some reason means short defis in Word.
0x001F - soft hyphen in Word
0x0013 - start embedded hyperlink
0x0014 - separate hyperlink URL from text
0x0015 - end embedded hyperlink
*/

	switch (ch) {
		case INLINE_PICTURE:
			{
				doc_get_picture(
						ch, 
						p, 
						NULL, 
						picture);
				printf("%c", ' '); break;
				break;
			}
			
		case FLOATING_PICTURE:
			{
				doc_get_picture(
						ch, 
						p, 
						NULL, 
						picture);
				printf("%c", ' '); break;
				break;
			}
		case CELL_MARK:
			{
				if (p->pap.TTP){
					printf("\n____________________________\n");

				} else {
					printf(" | ");	
				}
				break;
			}
		case PARAGRAPH_MARK:
			{
				printf("\n");
				break;
			}

		case 0x1E: printf("%c", '-' ); break;
		case 0x09: printf("%c", '\t'); break;
		case 0x13: printf("%c", ' ' ); break;
		case 0x15: printf("%c", ' ' ); break;
		case 0x0C: printf("%c", ch)  ; break;
		case 0x1F: printf("%c", 0xAD); break;
		case 0x0B: printf("%c", 0x0A); break;
		default: putchar(ch);
	}

	return 0;
}

int styles(void *d, STYLE *s){
	//fprintf(stderr, 
			//"STYLE: %d, %s, fs: %d, b: %d, u: %d, i: %d\n", 
			//s->s, s->name, s->chp.size, s->chp.fBold, s->chp.fUnderline, s->chp.fItalic);
	//fprintf(stderr, 
			//"STYLE: %d, %s, fs: %d, b: %d, u: %d, i: %d\n", 
			//s->s, s->name, s->pap_chp.size, s->pap_chp.fBold, s->pap_chp.fUnderline, s->pap_chp.fItalic);
	return 0;
}
