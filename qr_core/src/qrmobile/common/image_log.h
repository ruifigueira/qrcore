/*
 * image_log.h
 *
 *  Created on: Nov 13, 2010
 *      Author: rui
 */

#ifndef IMAGE_LOG_H_
#define IMAGE_LOG_H_

#include <qrmobile/common/log.h>
#include <qrmobile/common/image.h>
#include <qrmobile/common/point_line.h>

typedef qr_uint qr_color;

#define QR_COLOR_WHITE	0xffffff
#define QR_COLOR_BLACK	0x000000

#define QR_COLOR_RED	0xff0000
#define QR_COLOR_GREEN	0x00ff00
#define QR_COLOR_BLUE	0x0000ff
#define QR_COLOR_YELLOW 0xffff00
#define QR_COLOR_ORANGE 0xffa500
#define QR_COLOR_PURPLE 0xa020f0

#if (!ANDROID && defined (QR_DEBUG) && defined (QR_DEBUG_IMAGE) && QR_DEBUG_IMAGE != 0)
#define qr_image_log_init(file, img)		qr_draw_init ((file), (img))
#define qr_image_log_point(p, color)        qr_draw_point((p), (color))
#define qr_image_log_coords(x, y, color)    qr_draw_coords((x), (y), (color))
#define qr_image_log_line(l, color)	    	qr_draw_line ((l), (color))
#define qr_image_log_end()					qr_draw_end()

void qr_abort(qr_string s, ...);
void qr_draw_init(qr_string file_path, qr_bit_matrix *image);
void qr_draw_point(qr_point p, qr_color color);
void qr_draw_coords(qr_int x, qr_int y, qr_color color);
void qr_draw_line(qr_line line, qr_color color);
void qr_draw_end();

#else

#define qr_image_log_init(file, img)		((void) 0)
#define qr_image_log_point(p, color)        ((void) 0)
#define qr_image_log_coords(x, y, color)    ((void) 0)
#define qr_image_log_line(l, color)	        ((void) 0)
#define qr_image_log_end()				    ((void) 0)

#endif

#endif /* IMAGE_LOG_H_ */
