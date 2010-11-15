/*
 * bit_ops_test.c
 *
 *  Created on: Nov 1, 2010
 *      Author: rui
 */
#include <stddef.h>

#include <qrmobile/common/bit_ops.h>
#include <qrmobile/common/log.h>
#include <qrmobile/tests/test_utils.h>

/* 16 right bits set to 0, 16 unset at left */
#define SBS1	0x0000ffff
#define SBS2	0x0001ffff

#define MBS1_WORDS_SIZE		5
#define MBS1_TOTAL_SIZE		(32 * MBS1_WORDS_SIZE)

qr_uint MBS1[] =
{
		0xf000ffff,
		0x0001ffff,
		0x0001fff0,
		0x00000000,
		0x0001fff0
};

qr_size MBS1_COLOR_SIZES[] = { 16, 12, 21, 19, 13, 51, 13, 15 };

void qr_bit_ops_test_most_right()
{
	qr_uint zeros_1 = qr_most_right_zeros(SBS1);
	qr_uint ones_1 = qr_most_right_ones(SBS1);

	qr_assert(zeros_1 == 16);
	qr_assert(ones_1 == 0);

	qr_uint zeros_2 = qr_most_right_zeros(SBS2);
	qr_uint ones_2 = qr_most_right_ones(SBS2);

	qr_assert(zeros_2 == 15);
	qr_assert(ones_2 == 0);
}

void qr_bit_ops_test_most_left()
{
	qr_uint zeros_1 = qr_most_left_zeros(SBS1);
	qr_uint ones_1 = qr_most_left_ones(SBS1);

	qr_assert(zeros_1 == 0);
	qr_assert(ones_1 == 16);

	qr_uint zeros_2 = qr_most_left_zeros(SBS2);
	qr_uint ones_2 = qr_most_left_ones(SBS2);

	qr_assert(zeros_2 == 0);
	qr_assert(ones_2 == 17);
}

void qr_bit_ops_test_most_right_from()
{
	qr_uint zeros_1 = qr_most_right_zeros_from(SBS1, 19);
	qr_uint ones_1 = qr_most_right_ones_from(SBS1, 16);

	qr_assert(zeros_1 == 4);
	qr_assert(ones_1 == 0);

	qr_uint zeros_2 = qr_most_right_zeros_from(SBS2, 19);
	qr_uint ones_2 = qr_most_right_ones_from(SBS2, 16);

	qr_assert(zeros_2 == 3);
	qr_assert(ones_2 == 17);
}

void qr_bit_ops_test_most_left_from()
{

}

void qr_bit_ops_test_bitset_most_right_from()
{
	qr_size s = MBS1_TOTAL_SIZE - 1;

	qr_uint_p p = MBS1;

	qr_size i;
	for (i = 4 - 1; i >= 0; --i)
	{
		qr_size w1 = qr_bitset_most_right_zeros_from(p, s); s -= w1;
		qr_size b1 = qr_bitset_most_right_ones_from (p, s); s -= b1;

		qr_assert(w1 == MBS1_COLOR_SIZES[(i << 1) + 1]);
		qr_assert(b1 == MBS1_COLOR_SIZES[(i << 1)]);
	}
}

void qr_bit_ops_test_bitset_most_left_from()
{
	qr_size s = 0;

	qr_uint_p p = MBS1;

	qr_size i;
	for (i = 0; i < 4; ++i)
	{
		qr_size b1 = qr_bitset_most_left_ones_from (p, p + MBS1_WORDS_SIZE, s); s += b1;
		qr_size w1 = qr_bitset_most_left_zeros_from(p, p + MBS1_WORDS_SIZE, s); s += w1;

		qr_assert(b1 == MBS1_COLOR_SIZES[(i << 1)]);
		qr_assert(w1 == MBS1_COLOR_SIZES[(i << 1) + 1]);
	}
}

int main2(int argc, char** argv)
{
	qr_log_info("Starting tests...");

	qr_run_test(qr_bit_ops_test_most_right);
	qr_run_test(qr_bit_ops_test_most_left);

	qr_run_test(qr_bit_ops_test_most_right_from);
	qr_run_test(qr_bit_ops_test_most_left_from);

	qr_run_test(qr_bit_ops_test_bitset_most_right_from);
	qr_run_test(qr_bit_ops_test_bitset_most_left_from);

	qr_log_info("Tests finished...");

	return QR_TESTS_EXIT_SUCCESS;
}
