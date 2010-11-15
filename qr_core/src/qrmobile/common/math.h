/*
 * math.h
 *
 *  Created on: Nov 1, 2010
 *      Author: rui
 */

#ifndef QR_MATH_H_
#define QR_MATH_H_

#include <math.h>

#include <qrmobile/common/basic_types.h>

#define QR_INT_INF		0x7fffffff

#define qr_round(i)		((qr_int) ((i) + 0.5))
#define qr_ceil(i)		((qr_int) ceilf(i))

#define qr_max( a, b )  ( ((a) > (b)) ? (a) : (b) )
#define qr_min( a, b )  ( ((a) < (b)) ? (a) : (b) )
#define qr_abs( a )     ( ((a) > 0) ? (a) : -(a) )

#endif /* QR_MATH_H_ */
