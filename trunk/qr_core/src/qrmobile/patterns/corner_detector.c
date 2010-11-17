/*
 * corner_detector.c
 *
 *  Created on: Nov 7, 2010
 *      Author: rui
 */
#include <math.h>

#include <qrmobile/common/utils.h>
#include <qrmobile/common/math.h>
#include <qrmobile/common/log.h>
#include <qrmobile/common/image_log.h>

#include <qrmobile/patterns/corner_detector.h>

#define QR_NUM_SAMPLES_LN	4
#define QR_NUM_SAMPLES		(1 << NUM_SAMPLES_LN)
#define QR_START_SAMPLE		(NUM_SAMPLES >> 2)

#define QR_MAX_WHITE_CHECKS 20

/*
 * we need this trick to be able to generate different variables in macros (based on line number):
 * http://stackoverflow.com/questions/1597007/creating-c-macro-with-and-line-token-concatenation-with-positioning-macro
 */
#define __QR_VAR_NAME(pref, suf)	pref##suf
#define   QR_VAR_NAME(pref, suf)	__QR_VAR_NAME(pref, suf)
#define   QR_VAR_LN(pref)	          QR_VAR_NAME(pref, __LINE__)

#define qr_outside_limits(px, py, limits)	((px) < (limits)[0].x || (px) > (limits)[1].x || (py) < (limits)[0].y || (py) > (limits)[1].y)
#define qr_point_outside_limits(p, limits)  qr_outside_limits((p).x, (p).y, limits)

#define 	qr_image_log_pattern(pr)	                     \
	qr_image_log_point(pr->center, QR_COLOR_GREEN);          \
	qr_image_log_point(pr->border_points[0], QR_COLOR_PURPLE); \
	qr_image_log_point(pr->border_points[1], QR_COLOR_PURPLE); \
	qr_image_log_point(pr->border_points[2], QR_COLOR_PURPLE); \
	qr_image_log_point(pr->border_points[3], QR_COLOR_PURPLE); \
	qr_image_log_point(pr->border_points[4], QR_COLOR_PURPLE); \
	qr_image_log_point(pr->border_points[5], QR_COLOR_PURPLE); \
	qr_image_log_point(pr->border_points[6], QR_COLOR_PURPLE); \
	qr_image_log_point(pr->border_points[7], QR_COLOR_PURPLE)

const int QR_INDEXES[] = {
/* +-----------+-------+----------+------+ */
/* | mod_slope | steep | is_right | yinc | */
/* +-----------+-------+----------+------+ */
/* |     0     |   0   |    0     |  0   | */    4,
/* |     0     |   0   |    0     |  1   | */    4,
/* |     0     |   0   |    1     |  0   | */    0,
/* |     0     |   0   |    1     |  1   | */    0,
/* |     0     |   1   |    0     |  0   | */    2,
/* |     0     |   1   |    0     |  1   | */    6,
/* |     0     |   1   |    1     |  0   | */    6,
/* |     0     |   1   |    1     |  1   | */    2,
/* |     1     |   0   |    0     |  0   | */    3,
/* |     1     |   0   |    0     |  1   | */    5,
/* |     1     |   0   |    1     |  0   | */    7,
/* |     1     |   0   |    1     |  1   | */    1,
/* |     1     |   1   |    0     |  0   | */    3,
/* |     1     |   1   |    0     |  1   | */    5,
/* |     1     |   1   |    1     |  0   | */    7,
/* |     1     |   1   |    1     |  1   | */    1
/* +-----------+-------+----------+------+ */
};

/*
 * Bresenham's line algorithm implementation extracted from:
 * http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
 *
 * Assumes that both points stand inside the image.
 */
qr_bool qr_check_white_line(qr_line line, qr_bit_matrix *image)
{

/* This macro defines plot as a check if pixel is black. In that case, returns false */
#define qr_plot(x, y)	                                   \
	if(xstep_error < 0)                                    \
	{                                                      \
		qr_image_log_coords((x), (y), QR_COLOR_BLUE);      \
		if(qr_image_get(image, (x), (y))) return QR_FALSE; \
		xstep_error += delta_x;                            \
	}                                                      \
	else                                                   \
	{                                                      \
		xstep_error -= QR_MAX_WHITE_CHECKS;                \
	}

	qr_int x0 = line.p0.x;
	qr_int y0 = line.p0.y;

	qr_int x1 = line.p1.x;
	qr_int y1 = line.p1.y;

	qr_bool steep = qr_abs(y1 - y0) > qr_abs(x1 - x0);

	if (steep)
	{
		qr_swap(x0, y0);
		qr_swap(x1, y1);
	}

	if (x0 > x1)
	{
		qr_swap(x0, x1);
		qr_swap(y0, y1);
	}

	qr_uint delta_x = x1 - x0;
	qr_uint delta_y = qr_abs(y1 - y0);
	qr_int error = delta_x >> 1;

	qr_int y = y0;
	qr_int ystep = y0 < y1 ? 1 : -1;

	qr_int xstep_error  = QR_MAX_WHITE_CHECKS >> 1;

	qr_int x;
	for (x = x0; x <= x1; ++x)
	{
		if (steep)
		{
			qr_plot(y, x);
		}
		else
		{
			qr_plot(x, y);
		}

		error -= delta_y;
		if (error < 0)
		{
			y += ystep;
			error += delta_x;
		}
	}

	return QR_TRUE;

#undef qr_plot
}

typedef struct
{
	qr_point point;
	qr_int square_norm;
}
qr_closest_white;

void qr_update_closest_white(qr_closest_white *closest_white, qr_point p0, qr_point p1)
{
	qr_int min_square_norm = closest_white->square_norm;

	qr_point diff = qr_point_sub(p1, p0);
	qr_size square_norm = qr_point_square_norm(diff);

	if(square_norm < min_square_norm)
	{
		closest_white->point = p1;
		closest_white->square_norm = square_norm;
	}
}

/*
 * Based on the Midpoint Circle algorithm implementation as described in
 * http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 */
qr_bool qr_raster_circle(
		qr_closest_white *closest_white,
		qr_point p0,
		qr_point p1,
		qr_size radius,
		qr_bit_matrix *image,
		qr_point limits[2])
{
	qr_image_log_point(p0, QR_COLOR_PURPLE);

#define qr_plot(x, y)                                                   \
	if(!qr_outside_limits((x), (y), limits))                            \
	{                                                                   \
		qr_point QR_VAR_LN(p_) = {(x), (y)};                            \
		qr_line QR_VAR_LN(line_) = {p0, QR_VAR_LN(p_)};                 \
		if(qr_check_white_line(QR_VAR_LN(line_), image))                \
			qr_update_closest_white(closest_white, p1, QR_VAR_LN(p_));  \
	}

	closest_white->square_norm = QR_INT_INF;

	const qr_int x0 = p0.x;
	const qr_int y0 = p0.y;

	qr_int f = 1 - radius;
	qr_int ddf_x = 1;
	qr_int ddf_y = -2 * radius;
	qr_int x = 0;
	qr_int y = radius;

	qr_plot(x0, y0 + radius);
	qr_plot(x0, y0 - radius);
	qr_plot(x0 + radius, y0);
	qr_plot(x0 - radius, y0);

	while (x < y)
	{
		/*
		 * ddF_x == 2 * x + 1;
		 * ddF_y == -2 * y;
		 * f == x*x + y*y - radius*radius + 2*x - y + 1;
         */
		if (f >= 0)
		{
			y--;
			ddf_y += 2;
			f += ddf_y;
		}

		x++;
		ddf_x += 2;
		f += ddf_x;

		qr_plot(x0 + x, y0 + y);
		qr_plot(x0 - x, y0 + y);
		qr_plot(x0 + x, y0 - y);
		qr_plot(x0 - x, y0 - y);
		qr_plot(x0 + y, y0 + x);
		qr_plot(x0 - y, y0 + x);
		qr_plot(x0 + y, y0 - x);
		qr_plot(x0 - y, y0 - x);
	}

	return closest_white->square_norm != QR_INT_INF;

#undef qr_plot
}

qr_bool qr_check_adjacent_patterns(
		qr_pattern_result *pr0,
		qr_pattern_result *pr1,
		qr_uint border_index,
		qr_bool clockwise,
		qr_bit_matrix *image)
{
	qr_point border_point0 = pr0->border_points[border_index];
	qr_point border_point1 = pr1->border_points[border_index];

	qr_point vect = qr_point_sub(border_point1, border_point0);

	qr_float m = (qr_max(pr0->estimate_width, pr1->estimate_width) >> 2) / qr_point_norm(vect);

	qr_point perpendicular_vect;
	perpendicular_vect = clockwise ? qr_point_rotate_90cw(vect) : qr_point_rotate_90ccw(vect);
	perpendicular_vect = qr_point_multiply(perpendicular_vect, m);

	border_point0 = qr_point_add(border_point0, perpendicular_vect);
	border_point1 = qr_point_add(border_point1, perpendicular_vect);

	qr_point limits[2] = { {0, 0} , { image->width - 1, image->height - 1 } };

	if(qr_point_outside_limits(border_point0, limits) || qr_point_outside_limits(border_point1, limits)) return QR_FALSE;

	qr_line line = { border_point0, border_point1 }; /* qr_image_log_line(line, QR_COLOR_BLUE); */

	return qr_check_white_line(line, image);
}

qr_int qr_find_adjacent_patterns(
		qr_pattern_result *pr0,
		qr_pattern_result *pr1,
		qr_pattern_result *other,
		qr_bit_matrix *image)
{


	if(pr0->center.x > pr1->center.x) {
		/* swap */
		qr_pattern_result *tmp = pr0;
		pr0 = pr1;
		pr1 = tmp;
	}

	qr_line line = { pr0->center, pr1->center };

	qr_size diff_y = qr_abs(line.p1.y - line.p0.y);
	qr_size diff_x = qr_abs(line.p1.x - line.p0.x);

	qr_float slope   = diff_y * 1.0f / diff_x;

	qr_bool mod_slope = slope > 0.4142 && slope < 2.4142;
	qr_bool steep     = diff_y > diff_x;
	qr_bool is_right  = qr_point_side(line, other->center) > 0;
	qr_bool yinc      = pr0->center.y < pr1->center.y;

	qr_uint indexes_index = 0;
	if(mod_slope) indexes_index += 8;
	if(steep    ) indexes_index += 4;
	if(is_right ) indexes_index += 2;
	if(yinc     ) indexes_index += 1;

	qr_uint border_index = QR_INDEXES[indexes_index];

	qr_bool clockwise = !is_right;

	if(qr_check_adjacent_patterns(pr0, pr1, border_index, clockwise, image))
	{
		return border_index;
	}

	return -1;
}

/*
 * Finds a point in the border that connects an opposite pattern corner with the no pattern corner
 */
qr_bool qr_find_no_pattern_border(qr_point *point, qr_point p0, qr_point p1, qr_size dist, qr_bool clockwise, qr_bit_matrix *image)
{
	qr_point vect = qr_point_sub(p1, p0);

	qr_float m = dist / qr_point_norm(vect);

	qr_point perpendicular_vect;
	perpendicular_vect = clockwise ? qr_point_rotate_90cw(vect) : qr_point_rotate_90ccw(vect);
	perpendicular_vect = qr_point_multiply(perpendicular_vect, m);

	p0 = qr_point_add(p0, perpendicular_vect); qr_image_log_point(p0, QR_COLOR_RED);
	p1 = qr_point_add(p1, perpendicular_vect); qr_image_log_point(p1, QR_COLOR_RED);

	/* vector (line.p0 -> line.p1) * .6 */
	qr_point v               = qr_point_multiply(qr_point_sub(p1, p0), 0.6f);
	/* perpendicular vector */
	qr_point pv = clockwise ? qr_point_rotate_90cw(v) : qr_point_rotate_90ccw(v);

	qr_size radius = qr_ceil(qr_point_norm(v));

	qr_point limits[2] =
	{
		{
			qr_max(0,                 p0.x + (v.x < 0 && pv.x < 0 ? -radius : qr_min(v.x, pv.x))),
			qr_max(0,                 p0.y + (v.y < 0 && pv.y < 0 ? -radius : qr_min(v.y, pv.y)))
		},
		{
			qr_min(image->width  - 1, p0.x + (v.x > 0 && pv.x > 0 ?  radius : qr_max(v.x, pv.x))),
			qr_min(image->height - 1, p0.y + (v.y > 0 && pv.y > 0 ?  radius : qr_max(v.y, pv.y)))
		}
	};

	qr_closest_white closest_white;

	if(qr_raster_circle(&closest_white, p0, qr_point_add(p0, v), radius, image, limits))
	{
		*point = closest_white.point;
		qr_image_log_point(*point, QR_COLOR_RED);
		return QR_TRUE;
	}

	return QR_FALSE;
}

qr_bool qr_detect_corners(qr_float_point *corner_points, qr_pattern_result *pattern_results, qr_bit_matrix *image)
{
	qr_pattern_result *pr0 = pattern_results + 0; qr_image_log_pattern(pr0);
	qr_pattern_result *pr1 = pattern_results + 1; qr_image_log_pattern(pr1);
	qr_pattern_result *pr2 = pattern_results + 2; qr_image_log_pattern(pr2);

	qr_uint found = 0;

	qr_pattern_result *middle_pattern = QR_NULL, *opposite_pattern0 = QR_NULL, *opposite_pattern1 = QR_NULL;

	qr_int border_index, border_index0, border_index1;

/*
 * This macro checks if patterns are adjacent and updates opposite and middle pattern
 * variables accordingly
 */
#define qr_find_adjacent_patterns_helper(i, pr0, pr1, pr2)                      \
	border_index = (i == 2 && found == 2) ? -1 :                                \
			qr_find_adjacent_patterns(pr0, pr1, pr2, image);                    \
	if(border_index < 0)                                                        \
	{                                                                           \
		opposite_pattern0 = pr0; opposite_pattern1 = pr1; middle_pattern = pr2; \
	}                                                                           \
	else                                                                        \
	{                                                                           \
	    if(found == 0)                                                          \
			border_index0 = border_index;                                       \
		else                                                                    \
			border_index1 = border_index;                                       \
	    found++;                                                                \
	}                                                                           \
	if (found < i) return QR_FALSE

	qr_find_adjacent_patterns_helper(0, pr0, pr1, pr2);
	qr_find_adjacent_patterns_helper(1, pr0, pr2, pr1);
	qr_find_adjacent_patterns_helper(2, pr1, pr2, pr0);

#undef qr_find_adjacent_patterns_helper

	qr_assert(middle_pattern != QR_NULL && opposite_pattern0 != QR_NULL && opposite_pattern1 != QR_NULL);

	/* we 'canonize' opposite patterns by forcing middle point to be left of opposite_pattern0 -> opposite_pattern1 */
	qr_line opposite_patterns_line = { opposite_pattern0->center, opposite_pattern1->center };

	if(qr_point_side(opposite_patterns_line, middle_pattern->center) > 0)
	{
		/* swap */
		qr_pattern_result *tmp = opposite_pattern0;
		opposite_pattern0 = opposite_pattern1;
		opposite_pattern1 = tmp;

		/* swap */
		qr_int tmp_i = border_index0;
		border_index0 = border_index1;
		border_index1 = tmp_i;
	}

	qr_point opposite_border_point0 = opposite_pattern0->border_points[border_index0];
	qr_point opposite_border_point1 = opposite_pattern1->border_points[border_index1];

	qr_uint other_border_index0 = (border_index0 + 6) % 8;
	qr_uint other_border_index1 = (border_index1 + 2) % 8;

	qr_point other_opposite_border_point0 = opposite_pattern0->border_points[other_border_index0];
	qr_point other_opposite_border_point1 = opposite_pattern1->border_points[other_border_index1];

	qr_size dist = qr_max(opposite_pattern0->estimate_width, opposite_pattern1->estimate_width) >> 3;

	qr_point border_point0, border_point1;

	if(!qr_find_no_pattern_border(&border_point0, other_opposite_border_point0, other_opposite_border_point1, dist, QR_TRUE,  image)) return QR_FALSE;
	if(!qr_find_no_pattern_border(&border_point1, other_opposite_border_point1, other_opposite_border_point0, dist, QR_FALSE, image)) return QR_FALSE;

	qr_image_log_point(border_point0, QR_COLOR_ORANGE);
	qr_image_log_point(border_point1, QR_COLOR_ORANGE);

	/* opposite pattern 0 -> middle pattern */
	qr_line line_0_middle     = { opposite_border_point0, middle_pattern->border_points[border_index0] };
	/* opposite pattern 1 -> middle pattern */
	qr_line line_1_middle     = { opposite_border_point1, middle_pattern->border_points[border_index1] };
	/* opposite pattern 0 -> corner with no pattern */
	qr_line line_0_no_pattern = { other_opposite_border_point0, border_point0 };
	/* opposite pattern 1 -> corner with no pattern */
	qr_line line_1_no_pattern = { other_opposite_border_point1, border_point1 };

	qr_float_point middle_corner, opposite_corner0, opposite_corner1, no_pattern_corner;

	qr_line_intersect(&middle_corner,     line_0_middle,     line_1_middle);
	qr_line_intersect(&opposite_corner0,  line_0_middle,     line_0_no_pattern);
	qr_line_intersect(&opposite_corner1,  line_1_middle,     line_1_no_pattern);
	qr_line_intersect(&no_pattern_corner, line_0_no_pattern, line_1_no_pattern);

	corner_points[0] = middle_corner;     qr_image_log_point(qr_float_point_to_int_point(corner_points[0]), QR_COLOR_RED);
	corner_points[2] = opposite_corner1;  qr_image_log_point(qr_float_point_to_int_point(corner_points[2]), QR_COLOR_RED);
	corner_points[1] = opposite_corner0;  qr_image_log_point(qr_float_point_to_int_point(corner_points[1]), QR_COLOR_RED);
	corner_points[3] = no_pattern_corner; qr_image_log_point(qr_float_point_to_int_point(corner_points[3]), QR_COLOR_RED);

	return QR_TRUE;
}
