/*
 * point_line.c
 *
 *  Created on: Nov 8, 2010
 *      Author: rui
 */
#include <qrmobile/common/log.h>
#include <qrmobile/common/point_line.h>

#include <qrmobile/tests/test_utils.h>

#include <stdlib.h>

void test_operation_points()
{
	qr_point p0 = {  5, 4 };
	qr_point p1 = { -1, 2 };
	qr_point res_add = qr_point_add(p0, p1);
	qr_point res_sub = qr_point_sub(p0, p1);
	qr_point res_inv = qr_point_inv(p0);

	qr_assert(res_add.x ==  4 && res_add.y ==  6);
	qr_assert(res_sub.x ==  6 && res_sub.y ==  2);
	qr_assert(res_inv.x == -5 && res_inv.y == -4);
}

void test_line_intersection()
{
	qr_line line1 = { {10, 10}, {20, 20} };
	qr_line line2 = { {10, 20}, {20, 10} };

	qr_float_point res;
	qr_line_intersect(&res, line1, line2);

	qr_assert(res.x == 15.0f && res.y == 15.0f);
}

int main_pl(int argc, char** argv)
{
	qr_log_info("Starting tests...");

	qr_run_test(test_operation_points);
	qr_run_test(test_line_intersection);

	qr_log_info("Tests finished...");

	return QR_TESTS_EXIT_SUCCESS;
}
