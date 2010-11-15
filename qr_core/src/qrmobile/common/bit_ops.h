/*
 * bit_ops.h
 *
 *  Created on: Oct 28, 2010
 *      Author: Rui Figueira
 */

#ifndef BIT_OPS_H_
#define BIT_OPS_H_

#include <math.h>

#include <qrmobile/common/basic_types.h>

/*
 * Counts consecutive zeros in a word from right (most significative) to left (least significative).
 */
qr_size qr_most_right_zeros(qr_uint i);

/*
 * Counts consecutive zeros in a word from left (least significative) to right (most significative).
 */
qr_size qr_most_left_zeros(qr_uint i);

#define qr_most_left_ones(i)   qr_most_left_zeros(~(i))
#define qr_most_right_ones(i)  qr_most_right_zeros(~(i))

qr_size qr_most_left_ones_from(qr_uint i, qr_size s);
qr_size qr_most_right_ones_from(qr_uint i, qr_size s);

/*
 * Counts consecutive zeros in a word from right (most significative) to left (least significative)
 * starting at bit position s.
 */
#define qr_most_right_zeros_from(i, s)	qr_most_right_ones_from(~(i), s)

/*
 * Counts consecutive zeros in a word from left (least significative) to right (most significative)
 * starting at bit position s.
 */
#define qr_most_left_zeros_from(i, s)	qr_most_left_ones_from(~(i), s)

qr_size qr_bitset_most_left_zeros_from(qr_uint_p bitset, qr_uint_p bitset_end, qr_size s);
qr_size qr_bitset_most_right_zeros_from(qr_uint_p bitset, qr_size s);

qr_size qr_bitset_most_left_ones_from(qr_uint_p bitset, qr_uint_p bitset_end, qr_size s);
qr_size qr_bitset_most_right_ones_from(qr_uint_p bitset, qr_size s);

#endif /* BIT_OPS_H_ */
