/*
 * point_line.h
 *
 *  Created on: Nov 8, 2010
 *      Author: rui
 */

#ifndef POINT_LINE_H_
#define POINT_LINE_H_

#include <math.h>

#include <qrmobile/common/basic_types.h>

#define QR_POINT(type)  \
typedef struct          \
{                       \
	type x;             \
	type y;             \
}                       \
type##_point;           \
                        \
type##_point type##_point_inv(type##_point p0);                  \
type##_point type##_point_add(type##_point p0, type##_point p1); \
type##_point type##_point_sub(type##_point p0, type##_point p1); \
type##_point type##_point_rotate_90cw(type##_point p0);          \
type##_point type##_point_rotate_90ccw(type##_point p0);         \
type##_point type##_point_multiply(type##_point p0, qr_float m); \
type##_size  type##_point_square_norm(type##_point p0);          \
qr_int_point type##_point_to_int_point(type##_point p0)


QR_POINT(qr_int);
QR_POINT(qr_float);
QR_POINT(qr_double);

#undef QR_POINT

typedef qr_int_point qr_point;

#define qr_point_inv(p0)          qr_int_point_inv(p0)
#define qr_point_add(p0, p1)      qr_int_point_add(p0, p1)
#define qr_point_sub(p0, p1)      qr_int_point_sub(p0, p1)
#define qr_point_rotate_90cw(p0)  qr_int_point_rotate_90cw(p0)
#define qr_point_rotate_90ccw(p0) qr_int_point_rotate_90ccw(p0)
#define qr_point_multiply(p0, m)  qr_int_point_multiply(p0, m)
#define qr_point_square_norm(p0)  ((qr_size) qr_int_point_square_norm(p0))

#define qr_point_norm(p0)         sqrtf((float) qr_point_square_norm(p0))

#define QR_LINE(type)          \
typedef struct                 \
{                              \
	type##_point p0;           \
	type##_point p1;           \
}                              \
type##_line

QR_LINE(qr_int);
QR_LINE(qr_float);
QR_LINE(qr_double);

typedef qr_int_line qr_line;

/*
 * < 0 -> point at left
 * = 0 -> point belongs to line
 * > 0 -> point at right
 */
#define qr_point_side(l, p)   ((l.p1.x - l.p0.x) * (p.y - l.p0.y) - (l.p1.y - l.p0.y) * (p.x - l.p0.x))
#define qr_line_steep(l)      (qr_abs(l.p1.y - l.p0.y) > qr_abs(l.p1.x - l.p0.x))

qr_bool qr_line_intersect(qr_float_point *res, qr_line l0, qr_line l1);

#undef QR_LINE

#endif /* POINT_LINE_H_ */
