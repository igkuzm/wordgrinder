/**
 * File              : debug.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 07.11.2022
 * Last Modified Date: 20.02.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "cfb.h"

void print_cfb_header(struct cfb * cfb){
	int i;
	printf("********************************************\n");
	printf("read CFB...\n");
	printf("********************************************\n");
	
	printf("_abSig: ");
	for (i = 0; i < 8; ++i)
		printf("0x%x ", cfb->header._abSig[i]);
	printf("\n");
	
	printf("_clid: ");
	printf("0x%x ", cfb->header._clid.a);
	printf("0x%x ", cfb->header._clid.b);
	printf("0x%x ", cfb->header._clid.c);
	printf("0x%x ", cfb->header._clid.d);
	printf("\n");
	
	printf("_uMinorVersion: %u\n", cfb->header._uMinorVersion);
	printf("_uDllVersion: %u\n", cfb->header._uDllVersion);
	printf("_uSectorShift: %u\n", cfb->header._uSectorShift);
	printf("_uMiniSectorShift: %u\n", cfb->header._uMiniSectorShift);
	printf("_usReserved: %u\n", cfb->header._usReserved);
	printf("_ulReserved1: %u\n", cfb->header._ulReserved1);
	printf("_ulReserved2: %u\n", cfb->header._ulReserved2);
	printf("_csectFat: %u\n", cfb->header._csectFat);
	printf("_sectDirStart: 0x%x\n", cfb->header._sectDirStart);
	printf("_signature: 0x%x\n", cfb->header._signature);
	printf("_ulMiniSectorCutoff: %u\n", cfb->header._ulMiniSectorCutoff);
	printf("_sectMiniFatStart: 0x%x\n", cfb->header._sectMiniFatStart);
	printf("_csectMiniFat: %u\n", cfb->header._csectMiniFat);
	printf("_sectDifStart: 0x%x\n", cfb->header._sectDifStart);
	printf("_csectDif: %u\n", cfb->header._csectDif);
	printf("********************************************\n");
}

void print_dir(cfb_dir * dir){
	int i;
	printf("********************************************\n");
	char dirname[BUFSIZ];
	cfb_dir_name(dir, dirname);
	printf("DIR %s\n", dirname);
	printf("********************************************\n");	
	printf("_ab: ");
	for (i = 0; i < dir->_cb; ++i) 
		printf("0x%x ", dir->_ab[i]);	
	printf("\n");
	printf("_cb: %u\n", dir->_cb);
	printf("_mse: %u\n", dir->_mse);
	printf("_bflags: %u\n", dir->_bflags);
	printf("_sidLeftSib: %d\n", dir->_sidLeftSib);
	printf("_sidRightSib: %d\n", dir->_sidRightSib);
	printf("_sidChild: %d\n", dir->_sidChild);
	printf("_clsId: ");
	printf("0x%x ", dir->_clsId.a);
	printf("0x%x ", dir->_clsId.b);
	printf("0x%x ", dir->_clsId.c);
	printf("0x%x ", dir->_clsId.d);
	printf("\n");	
	printf("_dwUserFlags: %u\n", dir->_dwUserFlags);
	printf("_time create: ");
	printf("%u ", dir->_time[0].dwLowDateTime);
	printf("%u ", dir->_time[0].dwHighDateTime);
	printf("\n");		
	printf("_time modify: ");
	printf("%u ", dir->_time[1].dwLowDateTime);
	printf("%u ", dir->_time[1].dwHighDateTime);
	printf("\n");		
	printf("_sectStart: 0x%x\n", dir->_sectStart);
	printf("_ulSize: %u\n", dir->_ulSize);
	printf("_dptPropType: 0x%x\n", dir->_dptPropType);
	printf("********************************************\n");
}

// vim:ft=c
