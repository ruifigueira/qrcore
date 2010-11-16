/*
 * pattern_detector.h
 *
 *  Created on: Oct 28, 2010
 *      Author: Rui Figueira
 */

#ifndef PATTERN_DETECTOR_H_
#define PATTERN_DETECTOR_H_

#include <qrmobile/common/image.h>

#define QR_NUM_PATTERNS		3

#define QR_PATTERN_MIN_SIZE	14
#define QR_PATTERN_MAX_SIZE	255

typedef struct
{
	qr_point center;
	qr_size estimate_width;
	qr_point border_points[8];
} qr_pattern_result;

void qr_pattern_detector_init();
qr_size qr_detect_position_patterns(qr_pattern_result *results, qr_bit_matrix *image);

void qr_row_border_limits(qr_int_p limits, qr_uint_p row, qr_uint_p row_end, qr_int point_in_border);
qr_bool qr_check_border(qr_bit_matrix *image, qr_int start_y, qr_int pattern_left_x, qr_size pattern_width, qr_int inc_y);

#endif /* PATTERN_DETECTOR_H_ */
