/**
 * File              : section_boundaries.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 05.08.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/section_boundaries.h"

/* Determining Section Boundaries */
CP last_cp_in_section(cfb_doc_t *doc, CP cp)
{
	int i;
	for (i=0; doc->plcfSed->aCP[i] < cp;)
		i++;

	return doc->plcfSed->aCP[i] - 1;
}

CP first_cp_in_section(cfb_doc_t *doc, CP cp)
{
	int i;
	for (i=0; doc->plcfSed->aCP[i] <= cp;)
		i++;
	i--;	

	return doc->plcfSed->aCP[i];
}
