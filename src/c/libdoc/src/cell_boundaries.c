/**
 * File              : cell_boundaries.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 26.05.2024
 * Last Modified Date: 28.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "../include/libdoc/cell_boundaries.h"

/* 2.4.4 Determining Cell Boundaries
 * This section describes an algorithm to find the
 * boundaries of the innermost table cell containing a
 * given character position or to determine that the given
 * character position is not in a table cell. Every
 * valid character position in a document belongs to a
 * paragraph, so table depth can be computed for
 * each paragraph. If a paragraph is found to be at depth
 * zero, that paragraph is not in a table cell.
 * Given character position cp, use the following algorithm
 * to determine if cp is in a table cell.*/
int get_cell_depth_for_cp(cfb_doc_t *doc, CP cp)
{
/* 1. Follow the procedure from Direct Paragraph Formatting
 * to find the paragraph properties for the
 * paragraph that contains cp. Apply the properties, and
 * determine the table depth as specified in
 * Overview of Tables. Call this itapOrig.*/

	return 0;
}

