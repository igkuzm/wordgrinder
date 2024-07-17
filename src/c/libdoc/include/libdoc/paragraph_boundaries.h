#ifndef PARAGRAPH_BOUNDARIES_H
#define PARAGRAPH_BOUNDARIES_H

#include "doc.h"

CP first_cp_in_paragraph(cfb_doc_t *doc, CP cp);
CP last_cp_in_paragraph( cfb_doc_t *doc, CP cp);

CP last_cp_in_row(cfb_doc_t *doc, CP cp);
CP last_cp_in_cell(cfb_doc_t *doc, CP cp);

#endif /* ifndef PARAGRAPH_BOUNDARIES_H */
