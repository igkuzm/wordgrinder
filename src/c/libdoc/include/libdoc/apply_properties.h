#ifndef APPLY_PROPERTIES_S
#define APPLY_PROPERTIES_S

#include "doc.h"
#include "sprm.h"
#include "prl.h"

// l = 0 for char, 1 for paragraph, 2 for section
int apply_property(cfb_doc_t *doc, int l, struct Prl *prl);

#endif /* ifndef APPLY_PROPERTIES_S */
