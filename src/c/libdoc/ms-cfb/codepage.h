/**
 * File              : codepage.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.11.2022
 * Last Modified Date: 16.11.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef CODEPAGE_H_
#define CODEPAGE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdio.h>	
#include <stdlib.h>	
#include <errno.h>	
#include <iconv.h>

/*
 * Functions to manage MS-CFB codepages
 * Uses library iconv
 */

/*
 * function `encoding_for_codepage`
 * Return encoding code UTF8 string (to use with iconv) form MS-CFB codepage int value. 
 */	
static const char *
encoding_for_codepage(uint16_t codepage);	

/*
 * function `unicode_decode_iconv`
 * Return UTF8 string(or other codepage) form MS-CFB codepage string. 
 * Uses iconv.
 * To init iconv: iconv_t ic = iconv_open("UTF-8", encoding_for_codepage(codepage)); 
 */
static char* 
unicode_decode_iconv(const char *s, size_t len, iconv_t ic);

/*
 * IMP
 */

struct codepage_entry_t {
    int code;
    const char *name;
};

static struct codepage_entry_t _codepage_entries[] = {
    { .code = 874,   .name = "WINDOWS-874"		},
    { .code = 932,   .name = "SHIFT-JIS"		},
    { .code = 936,   .name = "WINDOWS-936"		},
    { .code = 950,   .name = "BIG-5"			},
    { .code = 951,   .name = "BIG5-HKSCS"		},
    { .code = 1250,  .name = "WINDOWS-1250"		},
    { .code = 1251,  .name = "WINDOWS-1251"		},
    { .code = 1252,  .name = "WINDOWS-1252"		},
    { .code = 1253,  .name = "WINDOWS-1253"		},
    { .code = 1254,  .name = "WINDOWS-1254"		},
    { .code = 1255,  .name = "WINDOWS-1255"		},
    { .code = 1256,  .name = "WINDOWS-1256"		},
    { .code = 1257,  .name = "WINDOWS-1257"		},
    { .code = 1258,  .name = "WINDOWS-1258"		},
    { .code = 10000, .name = "MACROMAN"			},
    { .code = 10004, .name = "MACARABIC"		},
    { .code = 10005, .name = "MACHEBREW"		},
    { .code = 10006, .name = "MACGREEK"			},
    { .code = 10007, .name = "MACCYRILLIC"		},
    { .code = 10010, .name = "MACROMANIA"		},
    { .code = 10017, .name = "MACUKRAINE"		},
    { .code = 10021, .name = "MACTHAI"			},
    { .code = 10029, .name = "MACCENTRALEUROPE" },
    { .code = 10079, .name = "MACICELAND"		},
    { .code = 10081, .name = "MACTURKISH"		},
    { .code = 10082, .name = "MACCROATIAN"      },
};

static int codepage_compare(const void *key, const void *value) {
    const struct codepage_entry_t *cp1 = key;
    const struct codepage_entry_t *cp2 = value;
    return cp1->code - cp2->code;
}

static const char *encoding_for_codepage(uint16_t codepage) {
    struct codepage_entry_t key = { .code = codepage };
    struct codepage_entry_t *result = bsearch(&key, _codepage_entries,
            sizeof(_codepage_entries)/sizeof(_codepage_entries[0]),
            sizeof(_codepage_entries[0]), &codepage_compare);
    if (result) {
        return result->name;
    }
    return "WINDOWS-1252";
}

static char* unicode_decode_iconv(const char *s, size_t len, iconv_t ic) {
    char* outbuf = 0;

    if(s && len && ic)
    {
        size_t outlenleft = len;
        int outlen = len;
        size_t inlenleft = len;
        const char* src_ptr = s;
        char* out_ptr = 0;

        size_t st; 
        outbuf = malloc(outlen + 1);

		if(outbuf)
        {
            out_ptr = outbuf;
            while(inlenleft)
            {
                st = iconv(ic, (char **)&src_ptr, &inlenleft, 
						(char **)&out_ptr,(size_t *) &outlenleft);
                if(st == (size_t)(-1))
                {
                    if(errno == E2BIG)
                    {
                        size_t diff = out_ptr - outbuf;
                        outlen += inlenleft;
                        outlenleft += inlenleft;
                        outbuf = realloc(outbuf, outlen + 1);
                        if(!outbuf)
                        {
                            break;
                        }
                        out_ptr = outbuf + diff;
                    }
                    else
                    {
                        free(outbuf), outbuf = NULL;
                        break;
                    }
                }
            }
        }
        outlen -= outlenleft;

        if(outbuf)
        {
            outbuf[outlen] = 0;
        }
    }
    return outbuf;
}

#ifdef __cplusplus
}
#endif

#endif //CODEPAGE_H_
