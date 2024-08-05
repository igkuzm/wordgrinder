/**
 * File              : doc.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 05.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/doc.h"
#include <stdio.h>

/* How to read the FIB
 * The Fib structure is located at offset 0 of the
 * WordDocument Stream. Given the variable size of
 * the Fib, the proper way to load it is the following:
 * 1.  Set all bytes of the in-memory version of the Fib
 * being used to 0. It is recommended to use 
 *     the largest version of the Fib structure as the
 *     in-memory version.
 * 2.  Read the entire FibBase, which MUST be present and
 * has fixed size.
 * 3.  Read Fib.csw.
 * 4.  Read the minimum of Fib.csw * 2 bytes and the size,
 * in bytes, of the in-memory version of 
 *     FibRgW97 into FibRgW97.
 * 5.  If the application expects fewer bytes than indicated
 * by Fib.csw, advance by the difference
 *     thereby skipping the unknown portion of FibRgW97.
 * 6.  Read Fib.cslw.
 * 7.  Read the minimum of Fib.cslw * 4 bytes and the size,
 * in bytes, of the in-memory version of
 *     FibRgLw97 into FibRgLw97.
 * 8.  If the application expects fewer bytes than indicated
 * by Fib.cslw, advance by the 
 *     difference thereby skipping the unknown portion of
 *     FibRgLw97.
 * 9.  Read Fib.cbRgFcLcb.
 * 10. Read the minimum of Fib.cbRgFcLcb * 8 bytes and the
 * size, in bytes, of the in-memory 
 *     version of FibRgFcLcb into FibRgFcLcb.
 * 11. If the application expects fewer bytes than indicated
 * by Fib.cbRgFcLcb, advance by the 
 *     difference, thereby skipping the unknown portion of
 *     FibRgFcLcb.
 * 12. Read Fib.cswNew.
 * 13. Read the minimum of Fib.cswNew * 2 bytes and the
 * size, in bytes, of the in-memory version 
 *     of FibRgCswNew into FibRgCswNew.*/
static int _doc_fib_init(Fib *fib, FILE *fp, struct cfb *cfb){
#ifdef DEBUG
	LOG("start");
#endif
	memset(fib, 0, sizeof(struct Fib));

	//fib->base = NULL;
	//fib->csw = 0;
	//fib->rgW97 = NULL;
	//fib->cslw = 0;
	//fib->rgLw97 = NULL;
	//fib->cbRgFcLcb = 0;
	//fib->rgFcLcb = NULL;
	//fib->cswNew = 0;
	//fib->rgCswNew = NULL;

	//allocate fibbase
	fib->base = (FibBase *)ALLOC(32,
		ERR("malloc");
		return DOC_ERR_ALLOC);

	//read fibbase
#ifdef DEBUG
	LOG("read fibbase");
#endif
	
	if (fread(fib->base, 32, 1,
				fp) != 1)
	{
		ERR("fread");
		free(fib->base);
		return DOC_ERR_FILE;
	}
	if (cfb->biteOrder){
		fib->base->wIdent        = bswap_16(fib->base->wIdent);
		fib->base->nFib          = bswap_16(fib->base->nFib);
		fib->base->lid           = bswap_16(fib->base->lid);
		fib->base->pnNext        = bswap_16(fib->base->pnNext);
		fib->base->ABCDEFGHIJKLM = bswap_16(fib->base->ABCDEFGHIJKLM);
		fib->base->nFibBack      = bswap_16(fib->base->nFibBack);
		fib->base->lKey          = bswap_32(fib->base->lKey);
	}
	
	//check wIdent
#ifdef DEBUG
	LOG("check wIdent: 0x%x", fib->base->wIdent);
#endif	
	if (fib->base->wIdent != 0xA5EC){
		free(fib->base);
		return DOC_ERR_HEADER;
	}	

#ifdef DEBUG
	LOG("read csw");
#endif	
	//read Fib.csw
	if (fread(&(fib->csw), 2, 1,
				fp) != 1)
	{
		ERR("fread");
		free(fib->base);
		return DOC_ERR_FILE;
	}
	if (cfb->biteOrder){
		fib->csw = bswap_16(fib->csw);
	}

	//check csw
#ifdef DEBUG
	LOG("check csw: 0x%x", fib->csw);
#endif		
	if (fib->csw != 14) {
		free(fib->base);
		return DOC_ERR_HEADER;
	}

	//allocate FibRgW97
	fib->rgW97 = (FibRgW97 *)ALLOC(28,
		ERR("malloc");
		free(fib->base);
		return DOC_ERR_ALLOC);

	//read FibRgW97
#ifdef DEBUG
	LOG("read FibRgW97");
#endif
	if (fread(fib->rgW97, 28, 1,
				fp) != 1)
	{
		ERR("fread");
		free(fib->base);
		free(fib->rgW97);
		return DOC_ERR_FILE;
	}
	if (cfb->biteOrder){
		fib->rgW97->lidFE = bswap_16(fib->rgW97->lidFE);
	}

#ifdef DEBUG
	LOG("read Fib.cslw");
#endif	
	//read Fib.cslw
	if (fread(&(fib->cslw), 2, 1, fp) != 1){
		free(fib->base);
		free(fib->rgW97);
		return DOC_ERR_FILE;
	}
	if (cfb->biteOrder){
		fib->cslw = bswap_16(fib->cslw);
	}

#ifdef DEBUG
	LOG("check cslw: 0x%x", fib->cslw);
#endif	
	//check cslw
	if (fib->cslw != 22) {
		free(fib->base);
		free(fib->rgW97);
		return DOC_ERR_HEADER;
	}	

	//allocate FibRgLw97
	fib->rgLw97 = (FibRgLw97 *)ALLOC(88,
		ERR("malloc");
		free(fib->base);
		free(fib->rgW97);
		return DOC_ERR_ALLOC);
	
#ifdef DEBUG
	LOG("read Fib.FibRgLw97");
#endif	
	//read FibRgLw97
	if (fread(fib->rgLw97, 88, 1,
				fp) != 1)
	{
		ERR("fread");
		free(fib->base);
		free(fib->rgW97);
		free(fib->rgLw97);
		return DOC_ERR_FILE;
	}	
	if (cfb->biteOrder){
		fib->rgLw97->cbMac      = bswap_32(fib->rgLw97->cbMac);
		fib->rgLw97->ccpText    = bswap_32(fib->rgLw97->ccpText);
		fib->rgLw97->ccpFtn     = bswap_32(fib->rgLw97->ccpFtn);
		fib->rgLw97->ccpHdd     = bswap_32(fib->rgLw97->ccpHdd);
		fib->rgLw97->ccpAtn     = bswap_32(fib->rgLw97->ccpAtn);
		fib->rgLw97->ccpEdn     = bswap_32(fib->rgLw97->ccpEdn);
		fib->rgLw97->ccpTxbx    = bswap_32(fib->rgLw97->ccpTxbx);
		fib->rgLw97->ccpHdrTxbx = bswap_32(fib->rgLw97->ccpHdrTxbx);
	}
	
#ifdef DEBUG
	LOG("read Fib.cbRgFcLcb");
#endif	
	//read Fib.cbRgFcLcb
	if (fread(&(fib->cbRgFcLcb), 2, 1,
				fp) != 1)
	{
		ERR("fread");
		free(fib->base);
		free(fib->rgW97);
		free(fib->rgLw97);
		return DOC_ERR_FILE;
	}
	if (cfb->biteOrder){
		fib->cbRgFcLcb = bswap_16(fib->cbRgFcLcb);
	}
	
#ifdef DEBUG
	LOG("cbRgFcLcb: 0x%x", fib->cbRgFcLcb);
#endif	

	//allocate rgFcLcb
	fib->rgFcLcb = (uint32_t *)ALLOC(fib->cbRgFcLcb*8,
		ERR("malloc");
		free(fib->base);
		free(fib->rgW97);
		free(fib->rgLw97);
		return DOC_ERR_ALLOC);

#ifdef DEBUG
	LOG("read Fib.rgFcLcb");
#endif	
	//read rgFcLcb
	if (fread(fib->rgFcLcb, 8, fib->cbRgFcLcb,
				fp) != fib->cbRgFcLcb)
	{
		ERR("fread");
		free(fib->base);
		free(fib->rgW97);
		free(fib->rgLw97);
		free(fib->rgFcLcb);
		return DOC_ERR_FILE;
	}	
	if (cfb->biteOrder){
		int i;
		for (i = 0; i < fib->cbRgFcLcb/4; ++i) {
			fib->rgFcLcb[i] = bswap_32(fib->rgFcLcb[i]);	
		}
	}

#ifdef DEBUG
	LOG("read Fib.cswNew");
#endif	
	//read Fib.cswNew
	if (fread(&(fib->cswNew), 2, 1,
				fp) != 1)
	{
		ERR("fread");
		return -1;
	}

#ifdef DEBUG
	LOG("cswNew: 0x%x", fib->cswNew);
#endif	
	if (cfb->biteOrder){
		fib->cswNew = bswap_16(fib->cswNew);
	}

	if (fib->cswNew > 0){
		//allocate FibRgCswNew
		fib->rgCswNew = 
			(FibRgCswNew *)ALLOC(fib->cswNew * 2,
		  ERR("malloc");
			free(fib->base);
			free(fib->rgW97);
			free(fib->rgLw97);
			free(fib->rgFcLcb);
			return DOC_ERR_ALLOC);

#ifdef DEBUG
	LOG("read FibRgCswNew");
#endif		
		//read FibRgCswNew
		if (fread(fib->rgCswNew, 2, fib->cswNew,
					fp) != fib->cswNew)
		{
			ERR("fread");
			free(fib->base);
			free(fib->rgW97);
			free(fib->rgLw97);
			free(fib->rgFcLcb);
			return DOC_ERR_FILE;
		}	
		if (cfb->biteOrder){
			fib->rgCswNew->nFibNew = bswap_16(fib->rgCswNew->nFibNew);
			int i;
			for (i = 0; i < 4; ++i) {
				fib->rgCswNew->rgCswNewData[i] = bswap_16(fib->rgCswNew->rgCswNewData[i]);
			}
		}
	}
#ifdef DEBUG
	LOG("done");
#endif	
	return 0;
};

static FILE *_table_stream(cfb_doc_t *doc, struct cfb *cfb){
#ifdef DEBUG
	LOG("start");
#endif	
	char *table = (char *)"0Table";
	Fib *fib = &doc[0].fib;
	if (FibBaseG(fib[0].base))
		table = (char *)"1Table";
#ifdef DEBUG
	LOG("table name: %s", table);
#endif	
	return cfb_get_stream(cfb, table);
}

int _doc_plcfspa_init(cfb_doc_t *doc){
#ifdef DEBUG
	LOG("start");
#endif	

	FibRgFcLcb97 *rgFcLcb97 = (FibRgFcLcb97 *)(doc->fib.rgFcLcb);
	ULONG off = rgFcLcb97->fcPlcSpaMom;
	ULONG len = rgFcLcb97->lcbPlcSpaMom;
	ULONG ccpText =doc->fib.rgLw97->ccpText;

	if (len <= 0 || off <= 0) // there is no shapes in 
														// main document
		return 0;

	// read cp's
	doc->plcfspa = 
		NEW(struct PlcfSpa, 
				ERR("NEW"); 
				return -1);
	doc->plcfspa->aCP = 
		NEW(CP, return -1);

	fseek(doc->Table, off, SEEK_SET);
	
	int i;
	CP cp = 0;
	for (i=0; cp >= 0 && cp <= ccpText; ++i){
		if (fread(&cp, sizeof(CP), 1,
				doc->Table) < 1)
			break;
		doc->plcfspa->aCP[i] = cp;
		doc->plcfspa->aCP = 
			REALLOC(doc->plcfspa->aCP, (i+1)*sizeof(CP), 
					ERR("realloc"); 
					return -1);
	}
	doc->plcfspaNaCP = i-1;

	// read aSpa
	doc->plcfspa->aSpa = 
		ALLOC(doc->plcfspaNaCP * sizeof(struct Spa),
				ERR("alloc"); 
				return -1);

	struct Spa spa;
	for (i = 0; i < doc->plcfspaNaCP; ++i) {
		if (fread(&spa, 26, 1,
				doc->Table) < 1)
			break;
		doc->plcfspa->aSpa[i] = spa;
	}
	
#ifdef DEBUG
	LOG("PlcfSpa:");
	for (i = 0; i < doc->plcfspaNaCP; ++i) {
		LOG("CP: %d, LID: %d", 
				doc->plcfspa->aCP[i], doc->plcfspa->aSpa[i].lid);
	}
#endif	

	return 0;
}

int _doc_plcfSed_init(cfb_doc_t *doc){
#ifdef DEBUG
	LOG("start");
#endif	

	FibRgFcLcb97 *rgFcLcb97 = (FibRgFcLcb97 *)(doc->fib.rgFcLcb);
	ULONG off = rgFcLcb97->fcPlcfSed;
	ULONG len = rgFcLcb97->lcbPlcfSed;

	if (len <= 0 || off <= 0){
		// some error 
		ERR("unknown error");
		return 1;
	}

	// number of cp is len/(12 + 4) + 1 CP
	doc->plcfSedNaCP = len / 16 + 1;

	// read cp's
	doc->plcfSed = 
		NEW(struct PlcfSed, 
				ERR("NEW"); 
				return -1);
	doc->plcfSed->aCP = (CP *) 
		ALLOC(sizeof(CP) * doc->plcfSedNaCP, return -1);
	doc->plcfSed->aSed = (struct Sed *) 
		ALLOC(sizeof(struct Sed) * (doc->plcfSedNaCP - 1), return -1);

	fseek(doc->Table, off, SEEK_SET);
	fread(doc->plcfSed->aCP, sizeof(CP),
			doc->plcfSedNaCP, doc->Table);

	
	// read aSpa
	int i;
	for (i = 0; i < doc->plcfSedNaCP; ++i) {
		// skeep fn
		fseek(doc->Table, 2, SEEK_CUR);
		LONG fcSepx;
		fread(&fcSepx, 4,
				1, doc->Table);
		doc->plcfSed->aSed[i].fcSepx = fcSepx;
		// skeep fnMpr
		fseek(doc->Table, 2, SEEK_CUR);
		// skeep fcMpr
		fseek(doc->Table, 4, SEEK_CUR);
	}
	
#ifdef DEBUG
	LOG("PlcfSed with NaCP: %d", doc->plcfSedNaCP);
	for (i = 0; i < doc->plcfSedNaCP; ++i) {
		LOG("CP: %d, fcSepx: %d", 
				doc->plcfSed->aCP[i], doc->plcfSed->aSed[i].fcSepx);
	}
#endif	

	return 0;
}


int _plcpcd_init(struct PlcPcd * PlcPcd, uint32_t len, cfb_doc_t *doc){
#ifdef DEBUG
	LOG("start");
#endif	
	
	int i;

	//get lastCP
	uint32_t lastCp = 
			doc->fib.rgLw97->ccpFtn +
			doc->fib.rgLw97->ccpHdd +
			doc->fib.rgLw97->reserved3 + //Mcr
			doc->fib.rgLw97->ccpAtn +
			doc->fib.rgLw97->ccpEdn +
			doc->fib.rgLw97->ccpTxbx +
			doc->fib.rgLw97->ccpHdrTxbx;
	
	if (lastCp)
		lastCp += 1 + doc->fib.rgLw97->ccpText;
	else
		lastCp = doc->fib.rgLw97->ccpText;

#ifdef DEBUG
	LOG("lastCp: %d", lastCp);
#endif	

	//allocate aCP
	PlcPcd->aCp = (uint32_t *)ALLOC(4,
			ERR("malloc");
			free(PlcPcd);	
			return -1);

	//read aCP
	i=0;
	uint32_t ch;
	while(fread(&ch, 4, 1,
				doc->Table) == 1)
	{
		if (doc->biteOrder){
			ch = bswap_32(ch);
		}
		PlcPcd->aCp[i] = ch;
#ifdef DEBUG
	LOG("aCp[%d]: %u", i, PlcPcd->aCp[i]);
#endif		
		i++;
		if (ch == lastCp)
			break;

		//realloc aCp
#ifdef DEBUG
	LOG("realloc aCP with size: %d", (i+1)*4);
#endif
		void *ptr = REALLOC(PlcPcd->aCp, (i+1)*4,
				ERR("realloc");
				break);
	}
#ifdef DEBUG
	LOG("number of cp in array: %d", i);
#endif	
	//number of cp in array
	PlcPcd->aCPl = i;

	//read PCD - has 64bit
	uint32_t size = len - i*4;
	
	//number of Pcd in array
	PlcPcd->aPcdl = size / 8;
#ifdef DEBUG
	LOG("number of Pcd in array: %d", PlcPcd->aPcdl);
#endif	
	
	PlcPcd->aPcd = (struct Pcd *)ALLOC(size,
			ERR("malloc");
			free(PlcPcd->aCp);	
			free(PlcPcd);	
			return -1);

	// get Pcd array
	for (i = 0; i < PlcPcd->aPcdl; ++i) {
		uint64_t ch;
		struct Pcd Pcd;
		if (fread(&Pcd.ABCfR2, 2, 1,
					doc->Table) != 1)
		{
			ERR("fread");
			return -1;
		}
		if (fread(&Pcd.fc.fc, 4, 1,
					doc->Table) != 1)
		{
			ERR("fread");
			return -1;
		}
		if (fread(&Pcd.prm, 2, 1,
					doc->Table) != 1)
		{
			ERR("fread");
			return -1;
		}
		if (doc->biteOrder){
			Pcd.ABCfR2 = bswap_16(Pcd.ABCfR2);
			Pcd.fc.fc = bswap_32(Pcd.fc.fc);
			Pcd.prm = bswap_16(Pcd.prm);
		}

#ifdef DEBUG
		LOG("PlcPcd->aPcd[%d]: ABCfR2: 0x%x, FC: %d, PRM: 0x%x", 
				i, Pcd.ABCfR2, Pcd.fc.fc, Pcd.prm);
#endif	
		PlcPcd->aPcd[i] = Pcd;
	}

	
#ifdef DEBUG
	LOG("done");
#endif	

	return 0;
}

/* init CLX structure */
static int _clx_init(cfb_doc_t *doc)
{

#ifdef DEBUG
	LOG("start");
#endif

	//get CLX
	//All versions of the FIB contain exactly one FibRgFcLcb97 
	FibRgFcLcb97 *rgFcLcb97 = (FibRgFcLcb97 *)(doc->fib.rgFcLcb);
	
	//FibRgFcLcb97.fcClx specifies the offset in the Table Stream of a Clx
	uint32_t fcClx = rgFcLcb97->fcClx;
	if (doc->biteOrder)
		fcClx = bswap_32(fcClx);
#ifdef DEBUG
	LOG("fcClx: %d", fcClx);
#endif
	
	//FibRgFcLcb97.lcbClx specifies the size, in bytes, of that Clx
	uint32_t lcbClx = rgFcLcb97->lcbClx;
	if (doc->biteOrder)
		lcbClx = bswap_32(lcbClx);
#ifdef DEBUG
	LOG("lcbClx: %d", lcbClx);
#endif	

	struct Clx *clx = &doc->clx;
	
	//get clx
	uint8_t ch;
	fseek(doc->Table, fcClx, SEEK_SET);
	if (fread(&ch, 1, 1, 
				doc->Table) != 1)
	{
		ERR("fread");
		return -1;
	}
#ifdef DEBUG
	LOG("first bite of CLX: 0x%x", ch);
#endif

	if (ch == 0x01){ //we have RgPrc (Prc array)
#ifdef DEBUG
	LOG("we have RgPrc (Prc array)");
#endif		
		//allocate RgPrc
		clx->RgPrc = NEW(struct Prc,
				ERR("new");
				return DOC_ERR_ALLOC);
		
		int16_t cbGrpprl; //the first 2 bite of PrcData - signed integer
		if (fread(&cbGrpprl, 2, 1, 
					doc->Table) != 1)
		{
			ERR("fread");
			return -1;
		}
		if (doc->biteOrder){
			cbGrpprl = bswap_16(cbGrpprl);
		}
#ifdef DEBUG
	LOG("the first 2 bite of PrcData is cbGrpprl: 0x%x", cbGrpprl);
#endif		
		if (cbGrpprl > 0x3FA2) //error
			return DOC_ERR_FILE;		
		//allocate RgPrc->data 
		clx->RgPrc->data = NEW(struct PrcData,
			ERR("new");
			return DOC_ERR_ALLOC);

		clx->RgPrc->data->cbGrpprl = cbGrpprl;

		//allocate GrpPrl
		clx->RgPrc->data->GrpPrl = (struct Prl *)ALLOC(cbGrpprl,
				ERR("malloc");
				return DOC_ERR_ALLOC);
		
		//read GrpPrl
#ifdef DEBUG
	LOG("read GrpPrl");
#endif		
		if (fread(clx->RgPrc->data->GrpPrl,
			 	cbGrpprl, 1, doc->Table) != 1)
		{
			ERR("fread");
			return -1;
		}
		/* TODO:  parse GrpPrl + byteOrder */

		//read ch again
#ifdef DEBUG
	LOG("again first bite of CLX: 0x%x", ch);
#endif		
		if (fread(&ch, 1, 1, 
					doc->Table) != 1)
		{
			ERR("fread");
			return -1;
		}
	}	

	//get PlcPcd
	clx->Pcdt = NEW(struct Pcdt,
			ERR("new");
			return DOC_ERR_ALLOC);	

	//read Pcdt->clxt - this must be 0x02
	clx->Pcdt->clxt = ch;
#ifdef DEBUG
	LOG("Pcdt->clxt: 0x%x", clx->Pcdt->clxt);
#endif	
	if (clx->Pcdt->clxt != 0x02) { //some error
		ERR("some error");
		return DOC_ERR_FILE;		
	}

	//read lcb;
	if (fread(&(clx->Pcdt->lcb), 
			4, 1, doc->Table) != 1)
	{
		ERR("fread");
		return -1;
	}
	if (doc->biteOrder){
		clx->Pcdt->lcb = bswap_32(clx->Pcdt->lcb);
	}
#ifdef DEBUG
	LOG("Pcdt->lcb: %d", clx->Pcdt->lcb);
#endif	

	//get PlcPcd
	_plcpcd_init(&(clx->Pcdt->PlcPcd),
		 	clx->Pcdt->lcb, doc);
	
#ifdef DEBUG
	LOG("aCP: %d, PCD: %d", clx->Pcdt->PlcPcd.aCPl, 
			clx->Pcdt->PlcPcd.aPcdl);
#endif		

#ifdef DEBUG
	LOG("done");
#endif
	
	return 0;
}

static int _doc_plcBtePapx_init(cfb_doc_t *doc){
#ifdef DEBUG
	LOG("start");
#endif
	FibRgFcLcb97 *fibRgFcLcb97 = 
		(FibRgFcLcb97 *)(doc->fib.rgFcLcb);
	doc->plcbtePapx = 
			plcbtePapx_get(
					doc->Table, 
					fibRgFcLcb97->fcPlcfBtePapx,
					fibRgFcLcb97->lcbPlcfBtePapx, 
					&doc->plcbtePapxNaFc); 
	if (!doc->plcbtePapx){
		ERR("can't read PlcBtePapx");
		return -1;
	}
	return 0;
}

static int _doc_plcBteChpx_init(cfb_doc_t *doc){
#ifdef DEBUG
	LOG("start");
#endif
	FibRgFcLcb97 *fibRgFcLcb97 = 
		(FibRgFcLcb97 *)(doc->fib.rgFcLcb);
	doc->plcbteChpx = 
			plcbteChpx_get(
					doc->Table, 
					fibRgFcLcb97->fcPlcfBteChpx,
					fibRgFcLcb97->lcbPlcfBteChpx, 
					&doc->plcbteChpxNaFc); 
	if (!doc->plcbteChpx){
		ERR("can't read PlcBteChpx");
		return -1;
	}
#ifdef DEBUG
	LOG("plcbteChpx with naFc: %d", doc->plcbteChpxNaFc);
	char str[BUFSIZ] = "";
	for (int i = 0; i < doc->plcbteChpxNaFc*2 - 1; ++i) {
		char s[16];
		sprintf(s, "%d ", 
				doc->plcbteChpx->aFc[i]);
		strcat(str, s);
	}
	LOG("%s", str);
#endif
	return 0;
}

static int _doc_STSH_init(cfb_doc_t *doc){
#ifdef DEBUG
	LOG("start");
#endif
	FibRgFcLcb97 *fibRgFcLcb97 = 
		(FibRgFcLcb97 *)(doc->fib.rgFcLcb);
	doc->STSH = STSH_get(doc->Table, 
			fibRgFcLcb97->fcStshf, 
			fibRgFcLcb97->lcbStshf, 
			&doc->lrglpstd);
	
	if (!doc->STSH){
		ERR("can't read PlcBteChpx");
		return -1;
	}
	
	return 0;
}


int doc_read(cfb_doc_t *doc, struct cfb *cfb){
#ifdef DEBUG
	LOG("start");
#endif

	memset(doc, 0, sizeof(cfb_doc_t));
	
	int ret = 0;
	//get byte order
	doc->biteOrder = cfb->biteOrder;
	
	//get WordDocument
	FILE *fp = cfb_get_stream(cfb, (char*)"WordDocument");
	if (!fp)	
		return DOC_ERR_FILE;
	fseek(fp, 0, SEEK_SET);
	doc->WordDocument = fp;

	//init FIB
	_doc_fib_init(&(doc->fib), doc->WordDocument, cfb);

	//get table
	doc->Table = _table_stream(doc, cfb);
	if (!doc->Table){
		ERR("Can't get Table stream"); 
		return DOC_ERR_FILE;
	}

	//get Data
	doc->Data = cfb_get_stream(cfb, (char*)"Data");

	//Read the Clx from the Table Stream
	ret = _clx_init(doc);
	if (ret)
		return ret;	

	// get PlcBtePapx
	ret = _doc_plcBtePapx_init(doc);
	if (ret)
		return ret;	
	
	// get PlcBteChpx
	ret = _doc_plcBteChpx_init(doc);
	if (ret)
		return ret;	

	// get PlcfSpa
	ret = _doc_plcfspa_init(doc);
	if (ret)
		return ret;	

	// get PlcfSed
	ret = _doc_plcfSed_init(doc);
	if (ret)
		return ret;	
	

	// get STSH
	ret = _doc_STSH_init(doc);
	if (ret)
		return ret;	

#ifdef DEBUG
	LOG("done");
#endif	
	return 0;
}

void doc_close(cfb_doc_t *doc)
{
	if (doc){
		if (doc->fib.base)
			free(doc->fib.base);
		if (doc->fib.rgW97)
			free(doc->fib.rgW97);
		if (doc->fib.rgLw97)
			free(doc->fib.rgLw97);
		if (doc->fib.rgFcLcb)
			free(doc->fib.rgFcLcb);
		if (doc->fib.rgCswNew)
			free(doc->fib.rgCswNew);
		
		if (doc->plcbteChpx)
			free(doc->plcbteChpx);
		if (doc->plcbtePapx)
			free(doc->plcbtePapx);
		if (doc->STSH)
			STSH_free(doc->STSH);
		if (doc->plcfspa){
			if(doc->plcfspa->aCP);
				free(doc->plcfspa->aCP);
			if(doc->plcfspa->aSpa);
				free(doc->plcfspa->aSpa);
			free(doc->plcfspa);
		}
		if (doc->plcfSed){
			if(doc->plcfSed->aCP);
				free(doc->plcfSed->aCP);
			if(doc->plcfSed->aSed);
				free(doc->plcfSed->aSed);
			free(doc->plcfSed);
		}
		
		if (doc->clx.Pcdt){
			if (doc->clx.Pcdt->PlcPcd.aCp)
				free(doc->clx.Pcdt->PlcPcd.aCp);
			if (doc->clx.Pcdt->PlcPcd.aPcd)
				free(doc->clx.Pcdt->PlcPcd.aPcd);
			free (doc->clx.Pcdt);
		}
		
		if (doc->clx.RgPrc){
			if (doc->clx.RgPrc->data){
				if (doc->clx.RgPrc->data->GrpPrl)
					free(doc->clx.RgPrc->data->GrpPrl);
				free(doc->clx.RgPrc->data);
			}
			free(doc->clx.RgPrc);
		}
		
		if (doc->Table)
			fclose(doc->Table);
		if (doc->WordDocument)
			fclose(doc->WordDocument);
		if (doc->Data)
			fclose(doc->Data);
	}
/* TODO: free memory and close streams */
}

static void image_from_OfficeArtBlipJPEG(
		FILE *fp, 
		struct OfficeArtRecordHeader *rh, 
		struct picture *pic,
		void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	struct OfficeArtBlipJPEG t;
	memset(&t, 0, sizeof(struct OfficeArtBlipJPEG));

	t.rh = *rh;
	fread(t.rgbUid1, 1, 16, fp);

	USHORT recInstance = OfficeArtRecordHeaderRecInstance(rh);
	if (
			recInstance != 0x46A && recInstance != 0x6E2 &&
			recInstance != 0x46B && recInstance != 0x6E3
			)
	{
		ERR("this is not OfficeArtBlipJPEG - recInstance not match");
		return;
	}

	if (recInstance == 0x46B || recInstance == 0x6E3){
		fread(t.rgbUid2, 1, 16, fp);
	}
	
	fread(&t.tag, 1, 1, fp);

	if (rh->recLen){
		// read BLIP data
		BYTE BLIPFileData[rh->recLen];
		fread(&BLIPFileData, rh->recLen, 1, fp);
		t.BLIPFileData = BLIPFileData;

		pic->type = pict_jpg;
		pic->data = BLIPFileData;
		pic->len  = rh->recLen;

		if (callback)
			callback(pic, userdata);
	}
};

static void image_from_OfficeArtBlipTIFF(
		FILE *fp, 
		struct OfficeArtRecordHeader *rh, 
		struct picture *pic,
		void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	struct OfficeArtBlipTIFF t;
	memset(&t, 0, sizeof(struct OfficeArtBlipTIFF));

	t.rh = *rh;
	fread(t.rgbUid1, 1, 16, fp);

	USHORT recInstance = OfficeArtRecordHeaderRecInstance(rh);
	if (recInstance != 0x6E4 && recInstance != 0x6E5){
		ERR("this is not OfficeArtBlipTIFF - recInstance not match");
		return;
	}

	if (recInstance == 0x6E5){
		fread(t.rgbUid2, 1, 16, fp);
	}
	
	fread(&t.tag, 1, 1, fp);

	if (rh->recLen){
		// read BLIP data
		BYTE BLIPFileData[rh->recLen];
		fread(&BLIPFileData, rh->recLen, 1, fp);
		t.BLIPFileData = BLIPFileData;

		pic->type = pict_tiff;
		pic->data = BLIPFileData;
		pic->len  = rh->recLen;

		if (callback)
			callback(pic, userdata);
	}
};

static void image_from_OfficeArtBlipDIB(
		FILE *fp, 
		struct OfficeArtRecordHeader *rh, 
		struct picture *pic,
		void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	struct OfficeArtBlipDIB t;
	memset(&t, 0, sizeof(struct OfficeArtBlipDIB));

	t.rh = *rh;
	fread(t.rgbUid1, 1, 16, fp);

	USHORT recInstance = OfficeArtRecordHeaderRecInstance(rh);
	if (recInstance != 0x7A8 && recInstance != 0x7A9){
		ERR("this is not OfficeArtBlipDIB - recInstance not match");
		return;
	}

	if (recInstance == 0x7A9){
		fread(t.rgbUid2, 1, 16, fp);
	}
	
	fread(&t.tag, 1, 1, fp);

	if (rh->recLen){
		// read BLIP data
		BYTE BLIPFileData[rh->recLen];
		fread(&BLIPFileData, rh->recLen, 1, fp);
		t.BLIPFileData = BLIPFileData;

		pic->type = pict_dbitmap;
		pic->data = BLIPFileData;
		pic->len  = rh->recLen;

		if (callback)
			callback(pic, userdata);
	}
};

static void image_from_OfficeArtBlipPICT(
		FILE *fp, 
		struct OfficeArtRecordHeader *rh, 
		struct picture *pic,
		void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	struct OfficeArtBlipPICT t;
	memset(&t, 0, sizeof(struct OfficeArtBlipPICT));

	t.rh = *rh;
	fread(t.rgbUid1, 1, 16, fp);

	USHORT recInstance = OfficeArtRecordHeaderRecInstance(rh);
	if (recInstance != 0x542 && recInstance != 0x543){
		ERR("this is not OfficeArtBlipPICT - recInstance not match");
		return;
	}

	if (recInstance == 0x543){
		fread(t.rgbUid2, 1, 16, fp);
	}
	
	fread(&t.metafileHeader, 1, 34, fp);

	if (rh->recLen){
		// read BLIP data
		BYTE BLIPFileData[rh->recLen];
		fread(&BLIPFileData, rh->recLen, 1, fp);
		t.BLIPFileData = BLIPFileData;

		pic->type = pict_mac;
		pic->data = BLIPFileData;
		pic->len  = rh->recLen;

		if (callback)
			callback(pic, userdata);
	}
};

static void image_from_OfficeArtBlipWMF(
		FILE *fp, 
		struct OfficeArtRecordHeader *rh, 
		struct picture *pic,
		void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	struct OfficeArtBlipWMF t;
	memset(&t, 0, sizeof(struct OfficeArtBlipWMF));

	t.rh = *rh;
	fread(t.rgbUid1, 1, 16, fp);

	USHORT recInstance = OfficeArtRecordHeaderRecInstance(rh);
	if (recInstance != 0x216 && recInstance != 0x217){
		ERR("this is not OfficeArtBlipWMF - recInstance not match");
		return;
	}

	if (recInstance == 0x217){
		fread(t.rgbUid2, 1, 16, fp);
	}
	
	fread(&t.metafileHeader, 1, 34, fp);

	if (rh->recLen){
		// read BLIP data
		BYTE BLIPFileData[rh->recLen];
		fread(&BLIPFileData, rh->recLen, 1, fp);
		t.BLIPFileData = BLIPFileData;

		pic->type = pict_wmf;
		pic->data = BLIPFileData;
		pic->len  = rh->recLen;

		if (callback)
			callback(pic, userdata);
	}
};

static void image_from_OfficeArtBlipEMF(
		FILE *fp, 
		struct OfficeArtRecordHeader *rh, 
		struct picture *pic,
		void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	struct OfficeArtBlipEMF t;
	memset(&t, 0, sizeof(struct OfficeArtBlipEMF));

	t.rh = *rh;
	fread(t.rgbUid1, 1, 16, fp);

	USHORT recInstance = OfficeArtRecordHeaderRecInstance(rh);
	if (recInstance != 0x3D4 && recInstance != 0x3D5){
		ERR("this is not OfficeArtBlipEMF - recInstance not match");
		return;
	}

	if (recInstance == 0x3D5){
		fread(t.rgbUid2, 1, 16, fp);
	}
	
	fread(&t.metafileHeader, 1, 34, fp);

	if (rh->recLen){
		// read BLIP data
		BYTE BLIPFileData[rh->recLen];
		fread(&BLIPFileData, rh->recLen, 1, fp);
		t.BLIPFileData = BLIPFileData;

		pic->type = pict_wmf;
		pic->data = BLIPFileData;
		pic->len  = rh->recLen;

		if (callback)
			callback(pic, userdata);
	}
};

static void image_from_OfficeArtBlipPNG(
		FILE *fp, 
		struct OfficeArtRecordHeader *rh, 
		struct picture *pic,
		void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	struct OfficeArtBlipPNG t;
	memset(&t, 0, sizeof(struct OfficeArtBlipPNG));

	t.rh = *rh;
	fread(t.rgbUid1, 1, 16, fp);


	USHORT recInstance = OfficeArtRecordHeaderRecInstance(rh);
	if (recInstance != 0x6E0 && recInstance != 0x6E1){
		ERR("this is not OfficeArtBlipPNG - recInstance not match");
		return;
	}

	if (recInstance == 0x6E1){
		fread(t.rgbUid2, 1, 16, fp);
	}
	
	fread(&t.tag, 1, 1, fp);

	if (rh->recLen){
		// read BLIP data
		BYTE BLIPFileData[rh->recLen];
		fread(&BLIPFileData, rh->recLen, 1, fp);
		t.BLIPFileData = BLIPFileData;

		pic->type = pict_png;
		pic->data = BLIPFileData;
		pic->len  = rh->recLen;

		if (callback)
			callback(pic, userdata);
	}
};

static void image_from_OfficeArtFBSE(
		FILE *fp, 
		cfb_doc_t *doc,
		struct OfficeArtRecordHeader *rh, 
		struct picture *pic,
		void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	struct OfficeArtFBSE t;	
	memset(&t, 0, sizeof(struct OfficeArtFBSE));

	t.rh = *rh;
	fread(&t.btWin32, 1, 1,  fp);
	fread(&t.btMacOS, 1, 1,  fp);
	fread(t.rgbUid,   1, 16, fp);
	fread(&t.tag,     2, 1,  fp);
	fread(&t.size,    4, 1,  fp);
	fread(&t.cRef,    4, 1,  fp);
	fread(&t.foDelay, 4, 1,  fp);
	fread(&t.unused1, 1, 1,  fp);
	fread(&t.cbName,  1, 1,  fp);
	fread(&t.unused2, 1, 1,  fp);
	fread(&t.unused3, 1, 1,  fp);

	
	t.nameData = NULL;
	BYTE nameData[t.cbName + 1];
	if (t.cbName){
		fread(nameData, t.cbName, 1, fp);
		t.nameData = nameData;
	}
	
	if (t.foDelay > 0){
		// the image is in OfficeArtBStoreDelay
		fp = doc->WordDocument;
		fseek(fp, t.foDelay, SEEK_SET);
	}
	
	// read BLIP header
	struct OfficeArtRecordHeader header;
	fread(&header, OfficeArtRecordHeaderSize, 1, fp);

#ifdef DEBUG
	LOG("BLIP with type: 0x%X and len %d",
			header.recType, header.recLen);
#endif
	
	if (header.recType == OfficeArtRecTypeOfficeArtFBSE)
		return image_from_OfficeArtFBSE(
				fp, doc, &header, pic, userdata, callback);

	if (header.recType == OfficeArtRecTypeOfficeArtBlipEMF)
		return image_from_OfficeArtBlipEMF(
				fp, &header, pic, userdata, callback);

	if (header.recType == OfficeArtRecTypeOfficeArtBlipWMF)
		return image_from_OfficeArtBlipWMF(
				fp, &header, pic, userdata, callback);
	
	if (header.recType == OfficeArtRecTypeOfficeArtBlipPICT)
		return image_from_OfficeArtBlipPICT(
				fp, &header, pic, userdata, callback);

	if (
			header.recType == OfficeArtRecTypeOfficeArtBlipJPEG ||
			header.recType == OfficeArtRecTypeOfficeArtBlipJPEG_
			)
		return image_from_OfficeArtBlipJPEG(
				fp, &header, pic, userdata, callback);
	
	if (header.recType == OfficeArtRecTypeOfficeArtBlipPNG)
		return image_from_OfficeArtBlipPNG(
				fp, &header, pic, userdata, callback);
	
	if (header.recType == OfficeArtRecTypeOfficeArtBlipDIB)
		return image_from_OfficeArtBlipDIB(
				fp, &header, pic, userdata, callback);
	
	if (header.recType == OfficeArtRecTypeOfficeArtBlipTIFF)
		return image_from_OfficeArtBlipTIFF(
				fp, &header, pic, userdata, callback);

};

void doc_get_inline_picture(
		int ch, ldp_t *p, void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	cfb_doc_t *doc = p->data;
	if (ch == INLINE_PICTURE){
		if (doc->prop.chp.sprmCFData){
			/* TODO: NilPICFAndBinData */
		
		} else {
			//PICFAndOfficeArtData
			struct PICFAndOfficeArtData t;
			memset(&t, 0, 
					sizeof(struct PICFAndOfficeArtData));
			
			// read PICF from stream
			fseek(doc->Data, 
					doc->prop.chp.sprmCPicLocation,
					SEEK_SET);
			fread(&t, 68, 1, doc->Data);
	
			// read PicName if needed
			if (t.picf.mfpf.mm == MM_SHAPEFILE){
				fread(&t.cchPicName,
						1, 1, doc->Data);
				BYTE stPicName[t.cchPicName + 1];
				t.stPicName = NULL;
				if (t.cchPicName > 0){
					fread(stPicName,
							t.cchPicName, 1, doc->Data);
					t.stPicName = stPicName; 
				}
			}

			// read SpContainer header
			struct OfficeArtRecordHeader spHeader;
			fread(&spHeader,
					OfficeArtRecordHeaderSize,
					1, doc->Data);

			if (spHeader.recType != 
					OfficeArtRecTypeOfficeArtSpContainer)
			{
				ERR("This is not OfficeArtSpContainer");
				return;
			}

			// skip SpContainer shape data
			fseek(doc->Data,
					spHeader.recLen, SEEK_CUR);

			// read OfficeArtBStoreContainerFileBlock header
			struct OfficeArtRecordHeader rh;
			fread(&rh,
					OfficeArtRecordHeaderSize,
					1, doc->Data);

#ifdef DEBUG
			LOG("OfficeArtBStoreContainerFileBlock with type: 0x%X and len %d",
					rh.recType, rh.recLen);
#endif

			struct picture pic;
			memset(&pic, 0, sizeof(struct picture));
			
			pic.goalw  = t.picf.picmid.dxaGoal;
			pic.goalh  = t.picf.picmid.dyaGoal;
			pic.scalex = t.picf.picmid.mx;
			pic.scaley = t.picf.picmid.my;

			if (rh.recType == OfficeArtRecTypeOfficeArtFBSE)
				return image_from_OfficeArtFBSE(
						doc->Data, doc, &rh, &pic, userdata, callback);
		
			if (rh.recType == OfficeArtRecTypeOfficeArtBlipEMF)
				return image_from_OfficeArtBlipEMF(
						doc->Data, &rh, &pic, userdata, callback);

			if (rh.recType == OfficeArtRecTypeOfficeArtBlipWMF)
				return image_from_OfficeArtBlipWMF(
						doc->Data, &rh, &pic, userdata, callback);
			
			if (rh.recType == OfficeArtRecTypeOfficeArtBlipPICT)
				return image_from_OfficeArtBlipPICT(
						doc->Data, &rh, &pic, userdata, callback);
			
			if (
					rh.recType == OfficeArtRecTypeOfficeArtBlipJPEG ||
					rh.recType == OfficeArtRecTypeOfficeArtBlipJPEG_
					)
				return image_from_OfficeArtBlipJPEG(
						doc->Data, &rh, &pic, userdata, callback);

			if (rh.recType == OfficeArtRecTypeOfficeArtBlipPNG)
				return image_from_OfficeArtBlipPNG(
						doc->Data, &rh, &pic, userdata, callback);
			
			if (rh.recType == OfficeArtRecTypeOfficeArtBlipDIB)
				return image_from_OfficeArtBlipDIB(
						doc->Data, &rh, &pic, userdata, callback);
			
			if (rh.recType == OfficeArtRecTypeOfficeArtBlipTIFF)
				return image_from_OfficeArtBlipDIB(
						doc->Data, &rh, &pic, userdata, callback);
		}	
	}
}
void doc_get_floating_picture(
		int ch, ldp_t *p, void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	cfb_doc_t *doc = p->data;
	if (ch != FLOATING_PICTURE){
		ERR("not a FLOATING_PICTURE");
		return;
	}

	int i;
	for (i = 0; i < doc->plcfspaNaCP; ++i) {
		CP cp = doc->plcfspa->aCP[i];	
		if (cp == doc->prop.chp.cp)
			break;
	}
	int index = i;

	if (doc->plcfspa->aCP[index] != doc->prop.chp.cp){
		ERR("no floating picture for CP: %d", doc->prop.chp.cp);
		return;
	}

	struct picture pic;
	memset(&pic, 0, sizeof(struct picture));
			
	pic.goalw  = 
		doc->plcfspa->aSpa[index].rca.right - 
		doc->plcfspa->aSpa[index].rca.left; 
	
	pic.goalh  = 
		doc->plcfspa->aSpa[index].rca.bottom - 
		doc->plcfspa->aSpa[index].rca.top; 

	// get OfficeArtDggContainer
	FibRgFcLcb97 *rgFcLcb97 = (FibRgFcLcb97 *)(doc->fib.rgFcLcb);
	ULONG off = rgFcLcb97->fcDggInfo;
	ULONG len = rgFcLcb97->lcbDggInfo;

	fseek(doc->Table, off, SEEK_SET);
	
	// read OfficeArtDggContainer header
	struct OfficeArtRecordHeader rh;
	fread(&rh, OfficeArtRecordHeaderSize, 1,
			doc->Table);
#ifdef DEBUG
	LOG("OfficeArtDggContainer type: 0x%X, len: %d", rh.recType, rh.recLen);
#endif
	
	if (rh.recType != OfficeArtRecTypeOfficeArtDggContainer)
	{
		ERR(" this is not OfficeArtDggContainer");
		return;
	}
	// read OfficeArtFDGGBlock header
	fread(&rh, OfficeArtRecordHeaderSize, 1,
			doc->Table);
	
#ifdef DEBUG
	LOG("OfficeArtFDGGBlock type: 0x%X, len: %d", rh.recType, rh.recLen);
#endif

	if (rh.recType != OfficeArtRecTypeOfficeArtFDggBlock)
	{
		ERR(" this is not OfficeArtFDggBlock");
		return;
	}
	// skip block
	fseek(doc->Table, 
			rh.recLen, SEEK_CUR);

	// read BLip Store
	// read header OfficeArtBStoreContainer with index `i`
	fread(&rh, OfficeArtRecordHeaderSize, 1,
			doc->Table);
		
#ifdef DEBUG
	USHORT recInstance = OfficeArtRecordHeaderRecInstance(&rh);
	LOG("OfficeArtBStoreContainer type: 0x%X, len: %d, number of records: %d", rh.recType, rh.recLen, recInstance);
#endif

	if (rh.recType != OfficeArtRecTypeOfficeArtBStoreContainer)
	{
		ERR(" this is not OfficeArtBStoreContainer");
		return;
	}
	// length od OfficeArtBStoreContainer
	int lenOfficeArtBStoreContainer = rh.recLen;

	// read rgfb header
	fread(&rh, OfficeArtRecordHeaderSize, 1,
			doc->Table);
	
#ifdef DEBUG
	LOG("OfficeArtBStoreContainerFileBlock with type: 0x%X and len %d",
					rh.recType, rh.recLen);
#endif
	
	// skip to index
	i = 0;
	while (lenOfficeArtBStoreContainer > 0)
	{
		if (i++ == index)
			break;
		fseek(doc->Table, rh.recLen, SEEK_CUR);
		lenOfficeArtBStoreContainer -= rh.recLen;
		fread(&rh, OfficeArtRecordHeaderSize, 1,
				doc->Table);
	}

	if (rh.recType == OfficeArtRecTypeOfficeArtFBSE)
		return image_from_OfficeArtFBSE(
				doc->Table, doc, &rh, &pic, userdata, callback);

	if (rh.recType == OfficeArtRecTypeOfficeArtBlipEMF)
		return image_from_OfficeArtBlipEMF(
				doc->Table, &rh, &pic, userdata, callback);

	if (rh.recType == OfficeArtRecTypeOfficeArtBlipWMF)
		return image_from_OfficeArtBlipWMF(
				doc->Table, &rh, &pic, userdata, callback);
	
	if (rh.recType == OfficeArtRecTypeOfficeArtBlipPICT)
		return image_from_OfficeArtBlipPICT(
				doc->Table, &rh, &pic, userdata, callback);
	
	if (
			rh.recType == OfficeArtRecTypeOfficeArtBlipJPEG ||
			rh.recType == OfficeArtRecTypeOfficeArtBlipJPEG_
			)
		return image_from_OfficeArtBlipJPEG(
				doc->Table, &rh, &pic, userdata, callback);

	if (rh.recType == OfficeArtRecTypeOfficeArtBlipPNG)
		return image_from_OfficeArtBlipPNG(
				doc->Table, &rh, &pic, userdata, callback);
	
	if (rh.recType == OfficeArtRecTypeOfficeArtBlipDIB)
		return image_from_OfficeArtBlipDIB(
				doc->Table, &rh, &pic, userdata, callback);
	
	if (rh.recType == OfficeArtRecTypeOfficeArtBlipTIFF)
		return image_from_OfficeArtBlipDIB(
				doc->Table, &rh, &pic, userdata, callback);
}

void doc_get_picture(
		int ch, ldp_t *p, void *userdata,
		void (*callback)(struct picture *pic, void *userdata))
{
	if (ch == INLINE_PICTURE)
		return doc_get_inline_picture(ch, p, userdata, callback);
	else if (ch == FLOATING_PICTURE)
		return doc_get_floating_picture(ch, p, userdata, callback);
	else 
		ERR("Not a picture CH: 0x%X", ch);
}
