/**
 * File              : style_properties.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 28.05.2024
 * Last Modified Date: 07.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/style_properties.h"
#include "../include/libdoc/prl.h"
#include "../include/libdoc/apply_properties.h"
#include "memread.h"
#include <stdint.h>
#include <stdio.h>

static int callbackPar(void *userdata, struct Prl *prl);
static int callbackChar(void *userdata, struct Prl *prl);
/* 2.4.6.5 Determining Properties of a Style
 * This section specifies an algorithm to determine the set
 * of properties to apply to text, a paragraph, a
 * table, or a list when a particular style is applied to
 * it. Given an istd, one or more arrays of Prl can be
 * derived that express the differences from defaults for
 * this style. Depending on its stk, a style can
 * specify properties for any combination of tables,
 * paragraphs, and characters. */

/* Given an istd: */
 struct LPStd *apply_style_properties(cfb_doc_t *doc, USHORT istd)
{
/* 1. Read the FIB from offset zero in the WordDocument
 * Stream. */
	struct Fib *fib = &doc->fib;

/* 2. All versions of the FIB contain exactly one
 * FibRgFcLcb97 though it can be nested in a larger
 * structure. Read a STSH from offset FibRgFcLcb97.fcStshf
 * in the Table Stream with size
 * FibRgFcLcb97.lcbStshf. */
	struct STSH *STSH = &doc->STSH;

/* 3. The given istd is a zero-based index into
 * STSH.rglpstd. Read an LPStd at STSH.rglpstd[istd]. */
	USHORT cstd = doc->STSH.lpstshi->stshi->stshif.cstd;
	struct LPStd *LPStd = 
		LPStd_at_index(STSH->rglpstd, 
				cstd, istd);
	if (!LPStd){
#ifdef DEBUG
	LOG("no STD int STSH at index: %d", istd);
#endif
		return NULL;
	}

	if (LPStd->cbStd == 0){
#ifdef DEBUG
	LOG("STD at index %d is 0 size - skiping...", istd);
#endif
		return NULL;
	}

/* 4. Read the STD structure as LPStd.std, of length
 * LPStd.cbStd bytes. */
	struct STD *STD = (struct STD *)LPStd->STD;

/* 5. From the STD.stdf.stdfBase obtain istdBase. If
 * istdBase is any value other than 0x0FFF, then
 * this style is based on another style. Recursively apply
 * this algorithm using istdBase as the
 * starting istd to obtain one or more arrays of Prls as the
 * properties for tables, paragraphs and
 * characters from the base style. */
	struct StdfBase *stdfBase = (struct StdfBase *)STD;
	USHORT istdBase = StdfBaseIstdBase(stdfBase);
#ifdef DEBUG
	LOG("parent style: %d (0x%04X)", 
			istdBase, istdBase);
#endif
	if (istdBase != 0x0FFF){
		// recursion 
		apply_style_properties(doc, istdBase);
	}

/* 6. From the STD.stdf.stdfBase obtain stk. For more
 * information, see the description of the cupx
 * member of StdfBase. Read an STD.grLPUpxSw. Based on the
 * stk, grLPUpxSw contains one of
 * the following structures: StkParaGRLPUPX, StkCharGRLPUPX,
 * StkTableGRLPUPX, StkListGRLPUPX */
	BYTE stk  = StdfBaseStk (stdfBase);
	BYTE cpux = StdfBaseCupx(stdfBase);
#ifdef DEBUG
	LOG("stk: %d, cpux: %d", stk, cpux);
#endif

	// check if STD->Stdf has StdfPost2000;
	struct STSHI *STSHI = STSH->lpstshi->stshi;
	USHORT cbSTDBaseInFile = STSHI->stshif.cbSTDBaseInFile;
#ifdef DEBUG
	LOG("cbSTDBaseInFile: 0x%04X", cbSTDBaseInFile);
#endif

	BYTE *p = NULL;

	if (cbSTDBaseInFile == 0x000A){
		// no StdfPost2000
		p = (BYTE *)(((struct STD_noStdfPost2000 *)STD)->xstzName_grLPUpxSw);

	} else if (cbSTDBaseInFile == 0x0012){
		// has StdfPost2000
		p = (BYTE *)(STD->xstzName_grLPUpxSw);
	
	} else {
		ERR("cbSTDBaseInFile");
		return NULL;
	}

	// get style name
	struct Xst *xst = (struct Xst *)p;
#ifdef DEBUG
	char str[BUFSIZ] = "";
	_utf16_to_utf8(xst->rgtchar, xst->cch > BUFSIZ?BUFSIZ:xst->cch, str);
	LOG("style %d name: %s, nlen: %d, stk: %d, cpux: %d", istd, str, xst->cch, stk, cpux);
#endif

// skip xstzName
	USHORT skip = 
		2              // size of cch (2 bytes) 
		+ xst->cch * 2 // number   16-bit Unicode characters
		+ 2;           // xstzName 16-bit null-terminated 
									 //
/* 7. Each of the preceding structures contains one or more
 * of the following: LPUpxPapx, LPUpxChpx,
 * LPUpxTapx. Each of the latter structures leads to one or
 * more arrays of Prl that specify properties.
 * For more information, see the sections documenting these
 * structures for how to obtain these
 * arrays.*/
	BYTE *ptr = p + skip;

	switch (stk) {
		case stkPar:
			{
				// paragraph prop
				USHORT cbUpx = *ptr;
				USHORT _istd = *(ptr + 2);
				int fc = 2;
				if (istd == _istd){
					fc+=2;
					cbUpx -= 2;
				}
				#ifdef DEBUG
					LOG("UpxPapx len: %d", cbUpx);
				#endif
				parse_grpprl(
					ptr+fc, 
					cbUpx, 
					doc, callbackPar);

				// character prop
				fc += cbUpx;
				// check padding 
				if(cbUpx % 2 != 0)
					fc++;
				
				cbUpx = *(ptr + fc); 
				#ifdef DEBUG
					LOG("UpxChpx len: %d", cbUpx);
				#endif
				fc += 2;
				BYTE *CHPX = ptr + fc;
				#ifdef DEBUG
					LOG("UpxChpx len: %d", cbUpx);
				#endif
			
				parse_grpprl(
					CHPX, 
					cbUpx, 
					doc, callbackPar);

				// revision marking prop
				if (cpux == 3){
					fc += cbUpx;
					
					// check padding 
					if(cbUpx % 2 != 0)
						fc++;

					/* TODO:  parse StkParaLpUpxGrLpUpxRM */
				}
			}
			break;
		case stkCha:
			{
				USHORT cbUpx = *ptr; 
				#ifdef DEBUG
					LOG("UpxChpx len: %d", cbUpx);
				#endif
				BYTE *CHPX = ptr + 2;
				#ifdef DEBUG
				Sprm sprm = *CHPX;
				fprintf(stderr, "sprm: 0x%X\n", sprm);
				USHORT ismpd = SprmIspmd(sprm);
				BYTE sgc = SprmSgc(sprm);
				LOG("sgc: 0x%x, ismpd: 0x%02x", sgc, ismpd);

				int i;
				for (i = 2; i < cbUpx; ++i) {
					fprintf(stderr, "0x%X ", CHPX[i]);
				}
				fprintf(stderr, "\n");
				#endif
				
				parse_grpprl(
				CHPX, 
				cbUpx, 
				doc, callbackChar);

				// revision marking prop
				if (cpux == 2){
					int fc = 2;
					fc += cbUpx;
					
					// check padding 
					if(cbUpx % 2 != 0)
						fc++;
				
					/* TODO:  parse StkParaLpUpxGrLpUpxRM */
				}
			}
			break;
		
		default:
#ifdef DEBUG
	LOG("no rule to parse stk: %d", stk);
#endif
		return LPStd;
	}

/* 8. For each array obtained in step 7 that specifies
 * properties of a table, paragraph, or characters,
 * append to the beginning of the corresponding array from
 * step 5, if any. The resulting arrays of Prl
 * are the desired output. Leave the algorithm. */
#ifdef DEBUG
	LOG("done");
#endif
	return LPStd;
}
int callbackPar(void *userdata, struct Prl *prl){
	// parse properties
	//USHORT ismpd = SprmIspmd(prl->sprm);
	//BYTE sgc = SprmSgc(prl->sprm);
	//LOG("sgc: 0x%x, ismpd: 0x%02x", sgc, ismpd);
	cfb_doc_t *doc = userdata;
	apply_property(doc, 1, prl);
	return 0;
}
int callbackChar(void *userdata, struct Prl *prl){
		
	//USHORT ismpd = SprmIspmd(prl->sprm);
	//BYTE sgc = SprmSgc(prl->sprm);
	//LOG("sgc: 0x%x, ismpd: 0x%02x", sgc, ismpd);
	// parse properties
	cfb_doc_t *doc = userdata;
	apply_property(doc, 0, prl);
	return 0;
}
