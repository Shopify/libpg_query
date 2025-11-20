/*--------------------------------------------------------------------
 * Symbols referenced in this file:
 * - YbCopyBitMatrix
 *--------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 *
 * ybbitmatrix.c
 *	  Yugabyte bit matrix package
 *
 * This module provides a boolean matrix data structure that is internally
 * implemented as a bitmapset (nodes/bitmapset.h). This allows for efficient
 * storage and row-level operations.
 *
 *
 * Copyright (c) YugabyteDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied.  See the License for the specific language governing permissions and limitations
 * under the License.
 *
 * IDENTIFICATION
 *	  src/backend/nodes/ybbitmatrix.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include <inttypes.h>

#include "nodes/ybbitmatrix.h"









void
YbCopyBitMatrix(YbBitMatrix *dest, const YbBitMatrix *src)
{
	*dest = *src;
	dest->data = bms_copy(src->data);
}









/*
 * This function returns the smallest row greater than "prev_row" that has a
 * member in the given column, or -2 if there is none.
 * "prev_row" must NOT be less than -1, or the behavior is unpredictable.
 *
 * This is intended as support for iterating through the members of a set.
 * The typical pattern is
 *
 *		x = -1;
 *		while ((x = YbBitMatrixNextMemberInColumn(inputmatrix, col, row_x)) >= 0)
 *			process member row_x;
 *
 * Notice that when there are no more members, we return -2, not -1 as you
 * might expect. The rationale for that is to allow distinguishing the
 * loop-not-started state (x == -1) from the loop-completed state (x == -2).
 * It makes no difference in simple loop usage, but complex iteration logic
 * might need such an ability.
 * This semantics of this function are based on bms_next_member/bms_prev_member
 * in bitmapset.c.
 */


/*
 * This function returns the smallest column greater than "prev_col" that has a
 * member in the given row, or -2 if there is none.
 * "prev_col" must NOT be less than -1, or the behavior is unpredictable.
 *
 * For semantics, see note on YbBitMatrixNextMemberInColumn.
 */





