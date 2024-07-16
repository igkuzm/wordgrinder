/**
 * File              : prl.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 16.07.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/doc.h"
#include "memread.h"
#include <stdint.h>
#include "../include/libdoc/sprm.h"

static struct Prl * prl_new(MEM *mem)
{
	struct Prl *prl = NULL;
	Sprm sprm;
	memread(&sprm, sizeof(Sprm), 1, mem);
	sprm = ctohs(sprm);	
	int spra  = SprmSpra(sprm);

	// get operand
	uint8_t *operand = NULL;
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
					USHORT cb = *(USHORT *)(mem->buffer);
					bytes = cb + 1;
					alloc += 2;
					break;
				}
				if (SprmSgc(sprm) == sgcPar &&
						SprmIspmd(sprm) == sprmPChgTabs)
				{
					BYTE cb = *(mem->buffer);
					if (cb >1 && cb < 255)
						bytes = cb;
					else if (cb == 255){
						/* TODO: PChgTabsOperand */
					}
					else {
						ERR("PChgTabsOperand");
					}
						
					break;
				}
				memread(&bytes, 1, 1, mem);
				alloc++;
			}
			break;
		default:
			break;
			
	}
	if (bytes){
		prl = MALLOC(bytes + sizeof(sprm) + alloc, 
				ERR("malloc"); return NULL);
		prl->sprm = sprm;
		memread(prl->operand, bytes, 1, mem);
	}
	return prl;
}

void parse_grpprl(
		uint8_t *grpprl, int len, void *userdata,
		int (*callback)(void *userdata, struct Prl *prl))
{
#ifdef DEBUG
	LOG("start");
#endif
	MEM *mem = memopen(grpprl, len);
	while (mem->p < mem->size) {
		struct Prl *prl = prl_new(mem);
		if (!prl)
			break;
		if (callback)
			if(callback(userdata, prl))
				break;
		if (prl)
			free(prl);
	}
}
