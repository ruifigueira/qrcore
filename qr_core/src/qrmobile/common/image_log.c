/*
 * image_log.c
 *
 *  Created on: Nov 13, 2010
 *      Author: rui
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <png.h>

#include <qrmobile/common/math.h>
#include <qrmobile/common/utils.h>
#include <qrmobile/common/image_log.h>

#define QR_IMAGE_LOG_RADIUS 1

struct qr_draw_data {
	FILE *fp;
	qr_size width;
	qr_size height;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;
}
__qr_draw_log_data;

void qr_abort(qr_string s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}

void qr_draw_point_helper(qr_uint x, qr_uint y, qr_color color)
{
	if(x < 0 || x >= __qr_draw_log_data.width || y < 0 || y >= __qr_draw_log_data.height) return;

	png_bytep cursor = __qr_draw_log_data.row_pointers[y] + x * 3;
	*cursor = (png_byte) (color >> 16) & 0xff; cursor++;
	*cursor = (png_byte) (color >>  8) & 0xff; cursor++;
	*cursor = (png_byte) (color >>  0) & 0xff; cursor++;
}

void qr_draw_init (qr_string file_path, qr_bit_matrix *image)
{
	qr_size width  = image->width;
	qr_size height = image->height;

    /* create file */
    FILE *fp = fopen(file_path, "wb");
    if (!fp)
    	qr_abort("[write_png_file] File %s could not be opened for writing", file_path);

    /* initialize stuff */
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    	qr_abort("[write_png_file] png_create_write_struct failed");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    	qr_abort("[write_png_file] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
    	qr_abort("[write_png_file] Error during init_io");

    png_init_io(png_ptr, fp);

    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
    	qr_abort("[write_png_file] Error during writing header");

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
    	qr_abort("[write_png_file] Error during writing bytes");

    qr_uint x, y;

    png_bytep *row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (y = 0; y < height; y++)
    	row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr, info_ptr));

	__qr_draw_log_data.fp           = fp;
	__qr_draw_log_data.width        = width;
	__qr_draw_log_data.height       = height;
	__qr_draw_log_data.png_ptr      = png_ptr;
	__qr_draw_log_data.info_ptr     = info_ptr;
	__qr_draw_log_data.row_pointers = row_pointers;

	for (y = 0; y < height; ++y)
	{
		for (x = 0; x < width; ++x)
		{
			qr_color color = qr_image_get(image, x, y) ? QR_COLOR_BLACK : QR_COLOR_WHITE;
			qr_draw_point_helper(x, y, color);
		}
	}

}

void qr_draw_end()
{
	FILE *fp                = __qr_draw_log_data.fp;
	qr_size height          = __qr_draw_log_data.height;
	png_structp png_ptr     = __qr_draw_log_data.png_ptr;
	png_infop info_ptr      = __qr_draw_log_data.info_ptr;
	png_bytep *row_pointers = __qr_draw_log_data.row_pointers;

    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    /* cleanup heap allocation */
    qr_uint y;
    for (y = 0; y < height; y++)
		free(row_pointers[y]);
	free(row_pointers);

	fclose(fp);
}

void qr_draw_point_radius(qr_uint x, qr_uint y, qr_size radius, qr_color color)
{
	qr_uint xmin = x - radius;
	qr_uint xmax = x + radius;
	qr_uint ymin = y - radius;
	qr_uint ymax = y + radius;

	for (y = ymin; y <= ymax; ++y)
	{
		for (x = xmin; x <= xmax; ++x)
		{
			qr_draw_point_helper(x, y, color);
		}
	}
}

void qr_draw_point(qr_point p, qr_color color)
{
	qr_draw_point_radius(p.x, p.y, QR_IMAGE_LOG_RADIUS, color);
}

void qr_draw_line (qr_line line, qr_color color)
{
/*
 * This macro defines plot as a check if pixel is black. In that case, returns false.
 */
#define qr_plot(x, y)	qr_draw_point_radius(x, y, 1, color)

	qr_int x0 = line.p0.x;
	qr_int y0 = line.p0.y;

	qr_int x1 = line.p1.x;
	qr_int y1 = line.p1.y;

	qr_plot(x0, y0);
	qr_plot(x1, y1);

	qr_bool steep = qr_abs(y1 - y0) > qr_abs(x1 - x0);

	if (steep)
	{
		qr_swap(x0, y0);
		qr_swap(x1, y1);
	}

	if (x0 > x1)
	{
		qr_swap(x0, x1);
		qr_swap(y0, y1);
	}

	qr_int delta_x = (x1 - x0);
	qr_int delta_y = qr_abs(y1 - y0);
	qr_int error = delta_x >> 1;

	qr_int y = y0;
	qr_int ystep = y0 < y1 ? 1 : -1;

	qr_int x;
	for (x = x0; x < x1; x += 1)
	{
		if (steep)
		{
			qr_plot(y, x);
		}
		else
		{
			qr_plot(x, y);
		}

		error -= delta_y;
		if (error < 0)
		{
			y += ystep;
			error += delta_x;
		}
	}

#undef qr_plot
}

