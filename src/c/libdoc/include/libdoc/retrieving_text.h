#ifndef RETRIEVING_TEXT_H
#define RETRIEVING_TEXT_H

#include "doc.h"

void get_char_for_cp(cfb_doc_t *doc, CP cp,
		void *user_data,
		int (*callback)(void *user_data, ldp_t *p, int ch));

#endif /* ifndef RETRIEVING_TEXT_H */
