/*
 * pattern_detector.c
 *
 *  Created on: Oct 28, 2010
 *      Author: Rui Figueira
 */

#include <qrmobile/common/log.h>
#include <qrmobile/tests/test_utils.h>

#include <qrmobile/patterns/pattern_detector.h>

qr_uint __bits_160x2[] =
{
	0, 0, 0, 0, 0,
	0x0, 0xffffff00, 0xffffffff, 0x00003fff, 0x0,
	0x0, 0x0001ff00, 0x00000000, 0x00000fc0, 0x0,
	0x0, 0xf003ff00, 0x03ffffff, 0x00003ff0, 0x0,
	0x0, 0x0003ff00, 0x00000000, 0x00003ff0, 0x0,
	0x0, 0xffffff00, 0xffffffff, 0x00003fff, 0x0,
	0, 0, 0, 0, 0,
};

void qr_detect_position_patterns_test()
{
	qr_uint MBS1[] =
	{
			0, 0, 0, 0, 0, 0,
			0xf000ffff,
			0x0001ffff,
			0x0001fff0,
			0x00000000,
			0x0001fff0,
			0
	};

	qr_pattern_result results[5];
	qr_bit_matrix image;

	qr_init_bit_matrix(&image, 192, 2, MBS1);

	qr_size num_results = qr_detect_position_patterns(results, &image);
}

void qr_detect_position_patterns_test_2()
{
	qr_pattern_result results[5];
	qr_bit_matrix image;

	qr_init_bit_matrix(&image, 150, 7, (qr_uint_p) &__bits_160x2);

	qr_size num_results = qr_detect_position_patterns(results, &image);

	qr_assert(num_results == 1);
}

void qr_row_border_limits_test()
{
	qr_bit_matrix image;

	qr_init_bit_matrix(&image, 150, 7, __bits_160x2);

	qr_size limits[2];
	qr_uint_p row = qr_image_get_row(&image, 3);
	qr_uint_p row_end = row + image.row_size;
	qr_row_border_limits(limits, row, row_end, 45);

	qr_assert(limits[0] == 40);
	qr_assert(limits[1] == 49);
}

void qr_row_detect_pattern_test()
{
	qr_bit_matrix image;

	qr_init_bit_matrix(&image, 150, 7, __bits_160x2);
	qr_assert(qr_check_border(&image, 3, 40, 70, -1));
	qr_assert(qr_check_border(&image, 3, 40, 70,  1));

	qr_pattern_result results[QR_NUM_PATTERNS];
	qr_assert(qr_detect_position_patterns(results, &image) == 1);

}

int main3(int argc, char** argv)
{
	qr_log_info("Starting tests...");

	qr_pattern_detector_init();

	qr_run_test(qr_detect_position_patterns_test);
//	qr_run_test(qr_detect_position_patterns_test_2);
	qr_run_test(qr_row_border_limits_test);
	qr_run_test(qr_row_detect_pattern_test);


	qr_log_info("Tests finished...");

	return QR_TESTS_EXIT_SUCCESS;
}
