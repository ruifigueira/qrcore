/*
 * corner_detector.h
 *
 *  Created on: Nov 7, 2010
 *      Author: rui
 */
#ifndef CORNER_DETECTOR_H_
#define CORNER_DETECTOR_H_

#include <qrmobile/common/point_line.h>
#include <qrmobile/common/image.h>

#include <qrmobile/patterns/pattern_detector.h>

qr_bool qr_detect_corners(qr_float_point *corner_points, qr_pattern_result *pattern_results, qr_bit_matrix *image);

#endif /* CORNER_DETECTOR_H_ */
