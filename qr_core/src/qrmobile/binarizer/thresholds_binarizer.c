/*
 * thresholds_binarizer.c
 *
 *  Created on: Oct 27, 2010
 *      Author: Rui Figueira
 */

#include <qrmobile/common/image.h>
#include <qrmobile/common/log.h>
#include <qrmobile/common/math.h>

#include "thresholds_binarizer.h"

void qr_compute_black_points(
		qr_uint_p  black_points,
		qr_ubyte_p luminances,
		qr_size    sub_width,
		qr_size    sub_height,
		qr_size    stride)
{

	qr_ubyte_p block_luminances_read_cursor = luminances;
	qr_uint_p  black_points_write_cursor = black_points;

	qr_int y, x;
	for (y = 0; y < sub_height; y++)
	{
		for (x = 0; x < sub_width; x++)
		{
			qr_uint sum = 0;
			qr_uint min = 255;
			qr_uint max = 0;

			qr_ubyte_p luminances_read_cursor = block_luminances_read_cursor;

			qr_int yy, xx;
			for (yy = 0; yy < 8; yy++)
			{
				for (xx = 0; xx < 8; xx++)
				{
					qr_assert(luminances_read_cursor == luminances + (y * 8 + yy) * stride + (x * 8 + xx));

					qr_ubyte pixel = (*luminances_read_cursor) & 0xff;
					sum += pixel;

					if (pixel < min)
						min = pixel;
					else
					if (pixel > max)
						max = pixel;

					luminances_read_cursor++;
				}

				luminances_read_cursor += (stride - 8);
			}

			/*
			 * If the contrast is inadequate, use half the minimum, so that this block will be
			 * treated as part of the white background, but won't drag down  neighboring blocks
			 * too much.
			 */
			qr_int average;
			if (max - min > 24)
			{
				average = sum >> 6;
			}
			else
			{
				/* When min == max == 0, let average be 1 so all is black */
				average = max == 0 ? 1 : min >> 1;
			}

			qr_assert(black_points_write_cursor == black_points + y * sub_width + x);

			*black_points_write_cursor = average;
			black_points_write_cursor++;

			block_luminances_read_cursor += 8;
		}

		/* we need to jump 8 lines, but we can't forget that we run an entire line,
		 * so we must subtract that one */
		block_luminances_read_cursor += (stride << 3) - (sub_width << 3);
	}
}

/*
 * For each 8x8 block in the image, calculate the average black point using a 5x5 grid
 * of the blocks around it. Also handles the corner cases, but will ignore up to 7 pixels
 * on the right edge and 7 pixels at the bottom of the image if the overall dimensions are not
 * multiples of eight. In practice, leaving those pixels white does not seem to be a problem.
 */
void qr_compute_thresholds_from_black_points(qr_uint_p block_thresholds, qr_size sub_width, qr_size sub_height, qr_uint_p black_points)
{
	qr_uint_p block_thresholds_write_cursor = block_thresholds;

	qr_int x, y, z;
	for (y = 0; y < sub_height; y++)
	{
		for (x = 0; x < sub_width; x++)
		{
			qr_int left = (x > 1) ? x : 2;
			left = (left < sub_width - 2) ? left : sub_width - 3;

			qr_int top = (y > 1) ? y : 2;
			top = (top < sub_height - 2) ? top : sub_height - 3;

			qr_uint_p black_row = black_points + sub_width * (top - 2) + left - 2;

			qr_uint sum = 0;
			for (z = -2; z <= 2; z++)
			{
				qr_assert(black_row == black_points + sub_width * (top + z) + left - 2);

				qr_uint_p black_row_cursor = black_row;
				sum += *black_row_cursor; black_row_cursor++;
				sum += *black_row_cursor; black_row_cursor++;
				sum += *black_row_cursor; black_row_cursor++;
				sum += *black_row_cursor; black_row_cursor++;
				sum += *black_row_cursor;

				black_row += sub_width;
			}

			qr_uint average = sum / 25.0f;

			qr_assert(block_thresholds_write_cursor == block_thresholds + y * sub_width + x);

			*block_thresholds_write_cursor = average;
			block_thresholds_write_cursor++;
		}
	}
}


void qr_compute_thresholds(qr_uint_p block_thresholds, qr_image_luminance *image_luminance)
{
	qr_uint black_points[(QR_MAX_IMAGE_WIDTH >> 3) * (QR_MAX_IMAGE_HEIGHT >> 3)];

	qr_size width  = image_luminance->width;
	qr_size height = image_luminance->height;

	/* width and height must be a multiple of 8 */
	qr_assert((width & 7)  == 0);
	qr_assert((height & 7) == 0);

	qr_size sub_width = width >> 3;
	qr_size sub_height = height >> 3;

	qr_compute_black_points(black_points, image_luminance->luminances, sub_width, sub_height, width);
	qr_compute_thresholds_from_black_points(block_thresholds, sub_width, sub_height, black_points);
}

/*
 * Binarizes four blocks of 8x8, which means 32x8, that is, 8 words, one per line.
 * In the case of the end of a line, we probably won't have 4 blocks but less, so num_blocks is
 * passed with that value.
 */
void qr_binarize_blocks(
		qr_uint_p  matrix_write_cursor,
		qr_size    matrix_stride,
		qr_ubyte_p luminances_read_cursor,
		qr_size    luminances_stride,
		qr_uint_p  thresholds_read_cursor,
		qr_size    num_blocks)
{
	qr_uint y, b, x;

	// for each line
	for( y = 0; y < 8; y++)
	{
		qr_uint blocks_line_bits = 0;

		// for each block
		for(b = 0; b < num_blocks; b++)
		{
			qr_uint start = b << 3;
			qr_uint end = (b + 1) << 3;

			// for each pixel
			for(x = start; x < end; x++)
			{
				// read current threshold
				qr_uint  threshold = *thresholds_read_cursor;
				qr_ubyte value = *luminances_read_cursor;

				if(value < threshold) blocks_line_bits |= (1 << x);

				luminances_read_cursor++;
			}

			// we're about to move to next block, so change
			// threshold cursor to point to next value
			thresholds_read_cursor++;
		}

		*matrix_write_cursor = blocks_line_bits;
		matrix_write_cursor += matrix_stride;

		// we need to subtract the number of luminances we consumed in one line
		luminances_read_cursor += (luminances_stride - (num_blocks << 3));

		// we need to rewind the thresholds cursor now to where it started
		thresholds_read_cursor -= num_blocks;
	}
}

void qr_binarize(qr_bit_matrix *bit_matrix, qr_image_luminance *image_luminance, qr_uint_p thresholds)
{
	qr_size width  = image_luminance->width;
	qr_size height = image_luminance->height;

	qr_assert(bit_matrix->width == width && bit_matrix->height == height);
	qr_assert(width % 8 == 0 && height % 8 == 0);

	qr_uint_p  bits                   = bit_matrix->bits;
	qr_ubyte_p luminances             = image_luminance->luminances;

	qr_size    matrix_stride          = ((width + 31) >> 5);
	qr_size    luminances_stride      = width;
	qr_size    thresholds_stride      = width >> 3;

	qr_uint y, x;

	for(y = 0; y < height; y += 8)
	{
		qr_uint_p  matrix_write_cursor    = bits       + (y        * matrix_stride);
		qr_ubyte_p luminances_read_cursor = luminances + (y        * luminances_stride);
		qr_uint_p  thresholds_read_cursor = thresholds + ((y >> 3) * thresholds_stride);

		for (x = 0; x < width; x += 32)
		{
			qr_size num_blocks = qr_min(32, width - x) >> 3;
			qr_binarize_blocks(matrix_write_cursor, matrix_stride, luminances_read_cursor, luminances_stride, thresholds_read_cursor, num_blocks);

			matrix_write_cursor++;

			// num_blocks * 8
			luminances_read_cursor += (num_blocks << 3);
			thresholds_read_cursor += num_blocks;
		}

	}
}

