/*
 * log.h
 *
 *  Created on: Oct 28, 2010
 *      Author: rui
 */

#ifndef LOG_H_
#define LOG_H_

#include <qrmobile/common/globals.h>

#ifdef ANDROID
#include <android/log.h>
#else
#include <stdio.h>
#endif

#ifdef ANDROID
#define LOG_TAG    "qrmobile"

#define qr_log_info(...)     __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

#if defined(QR_DEBUG) && QR_DEBUG != 0
#define qr_log_debug(...)    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define qr_log_debug(...)	((void) 0)
#endif

#else
#define qr_log_info(...)     do { printf(__VA_ARGS__); printf("\n"); } while(0)

#if defined(QR_DEBUG) && QR_DEBUG != 0
#define qr_log_debug(...)    do { printf(__VA_ARGS__); printf("\n"); } while(0)
#else
#define qr_log_debug(...)	((void) 0)
#endif

#endif

#define qr_log_var(var)   	  qr_log_debug("(%s:%d) value of %s: %d",  __FUNCTION__, __LINE__, #var, (var))

#if defined (QR_ASSERTIONS_ON) && QR_ASSERTIONS_ON != 0
#define qr_assert(exp) 		if(!(exp)) { qr_log_debug("(%s:%d) assertion failed: %s", __FUNCTION__, __LINE__, #exp); }
#else
#define qr_assert(exp) 		((void) 0)
#endif

#define qr_trace_func() 	qr_log_debug("(%s) entered function", __FUNCTION__)

#endif /* LOG_H_ */
