/*
 * corner_detector_test.c
 *
 *  Created on: Nov 8, 2010
 *      Author: rui
 */
#include <qrmobile/common/log.h>
#include <qrmobile/common/image_log.h>
#include <qrmobile/patterns/corner_detector.h>
#include <qrmobile/patterns/pattern_detector.h>

#include <qrmobile/tests/test_utils.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <png.h>

#define IN_FILE	    "/opt/workspace-helios/qr_core/test/data/qrcode-3/01.png"
#define OUT_FILE	"/opt/workspace-helios/qr_core/test/data/qrcode-3/01_out.png"


#define QR_ERR_MSG_NOT_PNG 			"[read_png_file] File %s is not recognized as a PNG file"
#define QR_ERR_MSG_COULD_NOT_READ	"[read_png_file] File %s could not be opened for reading"

void qr_read_from_file(qr_image_luminance *image_luminance, qr_string file_path)
{
	png_byte header[8];    // 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_path, "rb");
	if (!fp) qr_abort(QR_ERR_MSG_COULD_NOT_READ, file_path);

	fread(header, 1, 8, fp);

	if (png_sig_cmp(header, 0, 8)) qr_abort(QR_ERR_MSG_NOT_PNG, file_path);


	/* initialize stuff */
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
			qr_abort("[read_png_file] png_create_read_struct failed");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
			qr_abort("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
			qr_abort("[read_png_file] Error during init_io");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 width      = png_get_image_width (png_ptr, info_ptr);
	png_uint_32 height     = png_get_image_height(png_ptr, info_ptr);

	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		qr_abort("[read_png_file] Error during read_image");

    if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_GRAY)
    	qr_abort("[process_file] input file must be PNG_COLOR_TYPE_GRAY");

    qr_uint x, y;

    png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (y = 0; y < height; y++)
    	row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr, info_ptr));

	png_read_image(png_ptr, row_pointers);

	fclose(fp);

	image_luminance->width  = width;
	image_luminance->height = height;

	for (y = 0; y < height; ++y)
	{
		for (x = 0; x < width; ++x) {
			png_byte byte = row_pointers[y][x];
			image_luminance->luminances[y * height + x] = byte;
		}
	}

	/* cleanup heap allocation */
	for (y = 0; y < height; y++)
		free(row_pointers[y]);
	free(row_pointers);
}

void test_corner_detector()
{
	qr_ubyte_p luminance_data[QR_MAX_IMAGE_SIZE];
	qr_image_luminance luminance;
	luminance.luminances = luminance_data;

	qr_assert(luminance.luminances == luminance_data);

	qr_read_from_file(&luminance, IN_FILE);

	qr_uint bits[(QR_MAX_IMAGE_WIDTH >> 5) * QR_MAX_IMAGE_HEIGHT];
	qr_bit_matrix image;
	qr_init_bit_matrix(&image, luminance.width, luminance.height, bits);

	qr_uint thresholds[(QR_MAX_IMAGE_WIDTH >> 3) * (QR_MAX_IMAGE_HEIGHT >> 3)];
	qr_compute_thresholds(thresholds, &luminance);

	qr_binarize(&image, &luminance, thresholds);

	qr_image_log_init(OUT_FILE, &image);

	qr_pattern_result patterns[3];
	qr_size patterns_found = qr_detect_position_patterns(patterns, &image);

	qr_assert(patterns_found == 3);

	qr_float_point corners[4];
	qr_bool detected = qr_detect_corners(corners, patterns, &image);

	qr_assert(detected);

	qr_image_log_end();
}

int main(int argc, char** argv)
{
	qr_log_info("Starting tests...");

	qr_run_test(test_corner_detector);

	qr_log_info("Tests finished...");

	return QR_TESTS_EXIT_SUCCESS;
}
