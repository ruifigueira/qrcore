/*
 * image.c
 *
 *  Created on: Oct 27, 2010
 *      Author: rui
 */
#include <qrmobile/common/image.h>

void qr_init_image_luminance(qr_image_luminance *image_luminance, qr_size width, qr_size height, qr_ubyte_p luminances)
{
	image_luminance->width = width;
	image_luminance->height = height;
	image_luminance->luminances = luminances;
}

void qr_init_bit_matrix(qr_bit_matrix *bit_matrix, qr_size width, qr_size height, qr_uint_p bits)
{
	bit_matrix->row_size = ((width + 31) >> 5);

	bit_matrix->width = width;
	bit_matrix->height = height;
	bit_matrix->bits = bits;
}
