#ifndef PRL_H
#define PRL_H
#include "doc.h"

void parse_grpprl(
				uint8_t *grpprl, int len, void *userdata,
						int (*callback)(void *userdata, struct Prl *prl));

#endif /* ifndef PRL_H */
