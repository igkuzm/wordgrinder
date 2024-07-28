/**
 * File              : prl.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 28.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/doc.h"
#include "memread.h"
#include <stdint.h>
#include "../include/libdoc/sprm.h"

static struct Prl * prl_parse(BYTE *grpprl, int *read)
{
#ifdef DEBUG
	LOG("start");
#endif
	Sprm sprm = *(Sprm *)(&grpprl[*read]);

	sprm = ctohs(sprm);	
	int spra  = SprmSpra(sprm);

#ifdef DEBUG
	LOG("sprm: 0x%X", sprm);
#endif
	// get operand
	int bytes = 0, alloc = 0;
	switch (spra) {
		case 0: case 1:
			// 1 byte
			bytes = 1;
			break;
		case 2: case 4: case 5:
			// 2 byte
			bytes = 2;
			break;
		case 7:
			// 3 byte
			bytes = 3;
			break;
		case 3:
			// 4 byte
			bytes = 4;
			break;
		case 6:
			// The first byte of the operand indicates the size of
			// the rest of the operand, except in the cases of 
			// sprmTDefTable and sprmPChgTabs
			{
				if (SprmSgc(sprm) == sgcTab &&
						SprmIspmd(sprm) == sprmTDefTable)
				{
					USHORT cb = *(USHORT *)(&grpprl[read[0]+2]);
					bytes = cb + 1;
					break;
				}
				if (SprmSgc(sprm) == sgcPar &&
						SprmIspmd(sprm) == sprmPChgTabs)
				{
					BYTE cb = *(&grpprl[read[0]+2]);
					if (cb >1 && cb < 255)
						bytes = cb;
					else if (cb == 255){
						/* TODO: PChgTabsOperand */
						ERR("TODO PChgTabsOperand");
					}
					else {
						ERR("PChgTabsOperand");
					}
						
					break;
				}
				bytes = *(&grpprl[read[0]+2]);
			}
			break;
		default:
			break;
			
	}
	if (bytes){
		struct Prl *prl = (struct Prl *)(&grpprl[*read]);	
		*read += bytes + sizeof(Sprm);
		return prl;
	}
	// some error
	return NULL;
}

void parse_grpprl(
		uint8_t *grpprl, int len, void *userdata,
		int (*callback)(void *userdata, struct Prl *prl))
{
#ifdef DEBUG
	LOG("start");
#endif
	int read = 0;
	while (read < len) {
		struct Prl *prl = prl_parse(grpprl, &read);
		
		if (!prl) //stop all grpprl parsing on error
			break;
		
		if (callback)
			if(callback(userdata, prl))
				break;
	}
#ifdef DEBUG
	LOG("done");
#endif
}
