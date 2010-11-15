/*
 * test_utils.h
 *
 *  Created on: Nov 2, 2010
 *      Author: rui
 */

#ifndef TEST_UTILS_H_
#define TEST_UTILS_H_

#include <qrmobile/common/log.h>

#define QR_TESTS_EXIT_SUCCESS	0

#define qr_run_test(fn)	                               \
	qr_log_info("Test " #fn " running...");            \
	fn();                                              \
	qr_log_info("Test " #fn " ended successfully...")

#endif /* TEST_UTILS_H_ */
