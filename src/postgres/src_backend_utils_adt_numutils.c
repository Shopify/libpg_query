/*--------------------------------------------------------------------
 * Symbols referenced in this file:
 * - pg_strtouint64
 *--------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 *
 * numutils.c
 *	  utility functions for I/O of built-in numeric types.
 *
 * Portions Copyright (c) 1996-2022, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/utils/adt/numutils.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <math.h>
#include <limits.h>
#include <ctype.h>

#include "common/int.h"
#include "utils/builtins.h"
#include "port/pg_bitutils.h"

/*
 * A table of all two-digit numbers. This is used to speed up decimal digit
 * generation by copying pairs of digits into the final output.
 */


/*
 * Adapted from http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
 */




/*
 * Convert input string to a signed 16 bit integer.
 *
 * Allows any number of leading or trailing whitespace characters. Will throw
 * ereport() upon bad input format or overflow.
 *
 * NB: Accumulate input as a negative number, to deal with two's complement
 * representation of the most negative number, which can't be represented as a
 * positive number.
 */


/*
 * Convert input string to a signed 32 bit integer.
 *
 * Allows any number of leading or trailing whitespace characters. Will throw
 * ereport() upon bad input format or overflow.
 *
 * NB: Accumulate input as a negative number, to deal with two's complement
 * representation of the most negative number, which can't be represented as a
 * positive number.
 */


/*
 * Convert input string to a signed 64 bit integer.
 *
 * Allows any number of leading or trailing whitespace characters. Will throw
 * ereport() upon bad input format or overflow.
 *
 * NB: Accumulate input as a negative number, to deal with two's complement
 * representation of the most negative number, which can't be represented as a
 * positive number.
 */


/*
 * pg_itoa: converts a signed 16-bit integer to its string representation
 * and returns strlen(a).
 *
 * Caller must ensure that 'a' points to enough memory to hold the result
 * (at least 7 bytes, counting a leading sign and trailing NUL).
 *
 * It doesn't seem worth implementing this separately.
 */


/*
 * pg_ultoa_n: converts an unsigned 32-bit integer to its string representation,
 * not NUL-terminated, and returns the length of that string representation
 *
 * Caller must ensure that 'a' points to enough memory to hold the result (at
 * least 10 bytes)
 */


/*
 * pg_ltoa: converts a signed 32-bit integer to its string representation and
 * returns strlen(a).
 *
 * It is the caller's responsibility to ensure that a is at least 12 bytes long,
 * which is enough room to hold a minus sign, a maximally long int32, and the
 * above terminating NUL.
 */


/*
 * Get the decimal representation, not NUL-terminated, and return the length of
 * same.  Caller must ensure that a points to at least MAXINT8LEN bytes.
 */


/*
 * pg_lltoa: converts a signed 64-bit integer to its string representation and
 * returns strlen(a).
 *
 * Caller must ensure that 'a' points to enough memory to hold the result
 * (at least MAXINT8LEN + 1 bytes, counting a leading sign and trailing NUL).
 */



/*
 * pg_ultostr_zeropad
 *		Converts 'value' into a decimal string representation stored at 'str'.
 *		'minwidth' specifies the minimum width of the result; any extra space
 *		is filled up by prefixing the number with zeros.
 *
 * Returns the ending address of the string result (the last character written
 * plus 1).  Note that no NUL terminator is written.
 *
 * The intended use-case for this function is to build strings that contain
 * multiple individual numbers, for example:
 *
 *	str = pg_ultostr_zeropad(str, hours, 2);
 *	*str++ = ':';
 *	str = pg_ultostr_zeropad(str, mins, 2);
 *	*str++ = ':';
 *	str = pg_ultostr_zeropad(str, secs, 2);
 *	*str = '\0';
 *
 * Note: Caller must ensure that 'str' points to enough memory to hold the
 * result.
 */


/*
 * pg_ultostr
 *		Converts 'value' into a decimal string representation stored at 'str'.
 *
 * Returns the ending address of the string result (the last character written
 * plus 1).  Note that no NUL terminator is written.
 *
 * The intended use-case for this function is to build strings that contain
 * multiple individual numbers, for example:
 *
 *	str = pg_ultostr(str, a);
 *	*str++ = ' ';
 *	str = pg_ultostr(str, b);
 *	*str = '\0';
 *
 * Note: Caller must ensure that 'str' points to enough memory to hold the
 * result.
 */


/*
 * Yugabyte still needs this function from Pg15.
 *
 * pg_strtouint64
 *		Converts 'str' into an unsigned 64-bit integer.
 *
 * This has the identical API to strtoul(3), except that it will handle
 * 64-bit ints even where "long" is narrower than that.
 *
 * For the moment it seems sufficient to assume that the platform has
 * such a function somewhere; let's not roll our own.
 */
uint64
pg_strtouint64(const char *str, char **endptr, int base)
{
#ifdef _MSC_VER					/* MSVC only */
	return _strtoui64(str, endptr, base);
#elif defined(HAVE_STRTOULL) && SIZEOF_LONG < 8
	return strtoull(str, endptr, base);
#else
	return strtoul(str, endptr, base);
#endif
}
