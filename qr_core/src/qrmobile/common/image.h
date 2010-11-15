/*
 * image.h
 *
 *  Created on: Oct 27, 2010
 *      Author: rui
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <qrmobile/common/basic_types.h>
#include <qrmobile/common/point_line.h>

/* Maximum allowed image sizes */
#define QR_MAX_IMAGE_WIDTH	480
#define QR_MAX_IMAGE_HEIGHT	360
#define QR_MAX_IMAGE_SIZE	(QR_MAX_IMAGE_WIDTH * QR_MAX_IMAGE_HEIGHT)

/* Each word contains 32 bits */
#define QR_MAX_ROW_WORDS_SIZE 	(QR_MAX_IMAGE_WIDTH >> 5)
#define QR_MAX_BITS_WORDS_SIZE	(QR_MAX_ROW_WORDS_SIZE * QR_MAX_IMAGE_HEIGHT)

typedef struct
{
	qr_size width;
	qr_size height;
	qr_ubyte_p luminances;
} qr_image_luminance;

typedef struct
{
	qr_size width;
	qr_size height;
	qr_size row_size;
	qr_uint_p bits;
} qr_bit_matrix;

#define qr_row_get(row, x)	 		((qr_bool) (*((row) + ((x) >> 5)) >> ((x) & 0x1f)) & 1)

#define qr_image_get_row(img, y)    ((qr_uint_p) ((img)->bits + (img)->row_size * (y)))
#define qr_image_get(img, x, y)  	qr_row_get(qr_image_get_row((img), (y)), (x))

/* Image iluminance functions */
void qr_init_image_luminance(qr_image_luminance *image_luminance, qr_size width, qr_size height, qr_ubyte_p luminances);

/* Bit matrix functions */
void qr_init_bit_matrix(qr_bit_matrix *bit_matrix, qr_size width, qr_size height, qr_uint_p bits);

#endif /* IMAGE_H_ */
