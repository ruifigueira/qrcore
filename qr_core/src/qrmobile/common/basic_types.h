/*
 * basic_types.h
 *
 *  Created on: Oct 27, 2010
 *      Author: rui
 */

#ifndef BASIC_TYPES_H_
#define BASIC_TYPES_H_


typedef signed int    qr_int;
typedef signed char   qr_byte;

typedef unsigned int  qr_uint;
typedef unsigned char qr_ubyte;

typedef unsigned int  *qr_uint_p;
typedef unsigned char *qr_ubyte_p;

typedef signed int    *qr_int_p;
typedef signed char   *qr_byte_p;

typedef float  		  qr_float;
typedef double 		  qr_double;

typedef unsigned char qr_bool;

typedef const char    *qr_string;

typedef signed int    qr_size;

typedef signed int    qr_int_size;
typedef float         qr_float_size;
typedef double        qr_double_size;

typedef signed int    *qr_size_p;

#define QR_NULL		0

#define QR_TRUE		1
#define QR_FALSE	0

#endif /* BASIC_TYPES_H_ */
