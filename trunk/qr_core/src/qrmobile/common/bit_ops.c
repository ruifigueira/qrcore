/*
 * bit_ops.c
 *
 *  Created on: Oct 28, 2010
 *      Author: Rui Figueira
 */

#include <qrmobile/common/bit_ops.h>
#include <qrmobile/common/log.h>

/*
 * Implementation from:
 * java.lang.Integer.numberOfLeadingZeros
 */
qr_size qr_most_right_zeros(qr_uint i)
{
	// HD, Figure 5-6
	if (i == 0) return 32;

	qr_size n = 1;

	if (i >> 16 == 0) { n += 16; i <<= 16; }
	if (i >> 24 == 0) { n +=  8; i <<=  8; }
	if (i >> 28 == 0) { n +=  4; i <<=  4; }
	if (i >> 30 == 0) { n +=  2; i <<=  2; }

	n -= i >> 31;
	return n;
}

/*
 * Implementation from:
 * java.lang.Integer.numberOfTrailingZeros
 */
qr_size qr_most_left_zeros(qr_uint i)
{
	// HD, Figure 5-14
	qr_size y;
	if (i == 0) return 32;

	qr_size n = 31;

	y = i <<16; if (y != 0) { n = n -16; i = y; }
	y = i << 8; if (y != 0) { n = n - 8; i = y; }
	y = i << 4; if (y != 0) { n = n - 4; i = y; }
	y = i << 2; if (y != 0) { n = n - 2; i = y; }

	return n - ((i << 1) >> 31);
}

qr_size qr_most_left_ones_from(qr_uint i, qr_size s)
{
	if(s == 0) return qr_most_left_ones(i);
	i >>= s;
	return qr_most_left_ones(i);
}

qr_size qr_most_right_ones_from(qr_uint i, qr_size s)
{
	s = 31 - s;
	if(s == 0) return qr_most_right_ones(i);
	i <<= s;
	return qr_most_right_ones(i);
}


qr_size qr_bitset_most_right_zeros_from(qr_uint_p bitset, qr_size s)
{
	qr_size accum = 0;

	qr_size word_s = s & 31;
	qr_uint_p word_pos = bitset + (s >> 5);
	qr_size zeros = qr_most_right_zeros_from(*word_pos, word_s);
	if(zeros != word_s + 1) return zeros;

	accum += zeros;

	for (word_pos--; word_pos >= bitset; word_pos--) {
		zeros = qr_most_right_zeros(*word_pos);
		accum += zeros;
		if(zeros != 32) break;
	}

	return accum;
}

qr_size qr_bitset_most_left_zeros_from(qr_uint_p bitset, qr_uint_p bitset_end, qr_size s)
{
	qr_size accum = 0;

	qr_size word_s = s & 31;
	qr_uint_p word_pos = bitset + (s >> 5);
	qr_assert(word_pos < bitset_end);

	qr_size zeros = qr_most_left_zeros_from(*word_pos, word_s);
	if(zeros != (32 - word_s)) return zeros;

	accum += zeros;

	for (word_pos++; word_pos < bitset_end; word_pos++) {
		zeros = qr_most_left_zeros(*word_pos);
		accum += zeros;
		if(zeros != 32) break;
	}

	return accum;
}

qr_size qr_bitset_most_right_ones_from(qr_uint_p bitset, qr_size s)
{
	qr_size accum = 0;

	qr_size word_s = s & 31;
	qr_uint_p word_pos = bitset + (s >> 5);
	qr_size ones = qr_most_right_ones_from(*word_pos, word_s);
	if(ones != word_s + 1) return ones;

	accum += ones;

	for (word_pos--; word_pos >= bitset; word_pos--) {
		ones = qr_most_right_ones(*word_pos);
		accum += ones;
		if(ones != 32) break;
	}

	return accum;
}

qr_size qr_bitset_most_left_ones_from(qr_uint_p bitset, qr_uint_p bitset_end, qr_size s)
{
	qr_size accum = 0;

	qr_size word_s = s & 31;
	qr_uint_p word_pos = bitset + (s >> 5);
	qr_assert(word_pos < bitset_end);

	qr_size ones = qr_most_left_ones_from(*word_pos, word_s);

	if(ones != (32 - word_s)) return ones;

	accum += ones;

	for (word_pos++; word_pos < bitset_end; word_pos++) {
		ones = qr_most_left_ones(*word_pos);
		accum += ones;
		if(ones != 32) break;
	}

	return accum;
}

