/*
 * point_line.c
 *
 *  Created on: Nov 11, 2010
 *      Author: rui
 */
#include <math.h>

#include <qrmobile/common/point_line.h>

#define QR_POINT_DEFS(type)  \
type##_point type##_point_inv(type##_point p0)                    { type##_point res = { -p0.x, -p0.y };                         return res; } \
type##_point type##_point_add(type##_point p0, type##_point p1)   { type##_point res = { p0.x + p1.x, p0.y + p1.y };             return res; } \
type##_point type##_point_sub(type##_point p0, type##_point p1)   { type##_point res = { p0.x - p1.x, p0.y - p1.y };             return res; } \
type##_point type##_point_rotate_90cw (type##_point p0)           { type##_point res = { -p0.y,  p0.x };                         return res; } \
type##_point type##_point_rotate_90ccw(type##_point p0)           { type##_point res = {  p0.y, -p0.x };                         return res; } \
type##_point type##_point_multiply(type##_point p0, qr_float m)   { type##_point res = { ((type) p0.x * m), ((type) p0.y * m) }; return res; } \
qr_point     type##_point_to_int_point(type##_point p0)           { qr_point     res = { (qr_int) p0.x, (qr_int) p0.y };         return res; } \
type##_size  type##_point_square_norm (type##_point p0)           { return p0.x * p0.x + p0.y * p0.y; }

QR_POINT_DEFS(qr_int);
QR_POINT_DEFS(qr_double);
QR_POINT_DEFS(qr_float);

#undef QR_POINT_DEFS /* we no longer need it */

qr_bool qr_line_intersect(qr_float_point *res, qr_line l0, qr_line l1)
{
    /* Introduction:
     * This code is based on the solution of these two input equations:
     *  Pa = P1 + ua (P2-P1)
     *  Pb = P3 + ub (P4-P3)
     *
     * Where line one is composed of points P1 and P2 and line two is composed
     *  of points P3 and P4.
     *
     * ua/b is the fractional value you can multiple the x and y legs of the
     *  triangle formed by each line to find a point on the line.
     *
     * The two equations can be expanded to their x/y components:
     *  Pa.x = p1.x + ua(p2.x - p1.x)
     *  Pa.y = p1.y + ua(p2.y - p1.y)
     *
     *  Pb.x = p3.x + ub(p4.x - p3.x)
     *  Pb.y = p3.y + ub(p4.y - p3.y)
     *
     * When Pa.x == Pb.x and Pa.y == Pb.y the lines intersect so you can come
     *  up with two equations (one for x and one for y):
     *
     * p1.x + ua(p2.x - p1.x) = p3.x + ub(p4.x - p3.x)
     * p1.y + ua(p2.y - p1.y) = p3.y + ub(p4.y - p3.y)
     *
     * ua and ub can then be individually solved for.  This results in the
     *  equations used in the following code.
     */

	qr_int px0 = l0.p0.x; qr_int py0 = l0.p0.y;
	qr_int px1 = l0.p1.x; qr_int py1 = l0.p1.y;
	qr_int px2 = l1.p0.x; qr_int py2 = l1.p0.y;
	qr_int px3 = l1.p1.x; qr_int py3 = l1.p1.y;

	qr_int d = (px0 - px1) * (py2 - py3) - (py0 - py1) * (px2 - px3);
	if (d == 0) return QR_FALSE;

	res->x = ((px2 - px3) * (px0 * py1 - py0 * px1) - (px0 - px1) * (px2 * py3 - py2 * px3)) / (d * 1.0f);
	res->y = ((py2 - py3) * (px0 * py1 - py0 * px1) - (py0 - py1) * (px2 * py3 - py2 * px3)) / (d * 1.0f);

	return QR_TRUE;
}
