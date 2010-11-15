/*
 * thresholds_binarizer.h
 *
 *  Created on: Oct 27, 2010
 *      Author: Rui Figueira
 */

#ifndef THRESHOLDS_BINARIZER_H_
#define THRESHOLDS_BINARIZER_H_

#include "../common/basic_types.h"
#include "../common/image.h"

/*
 * We provide this function separated from qr_binarize so that we can
 * "cache" computed thresholds for posterior binarization operations.
 */
void qr_compute_thresholds(qr_uint_p thresholds, qr_image_luminance *image_luminance);

/*
 * Binarizes an image given its luminance data and 8x8 block threshold values
 */
void qr_binarize(qr_bit_matrix *bit_matrix, qr_image_luminance *image_luminance, qr_uint_p thresholds);

#endif /* THRESHOLDS_BINARIZER_H_ */
