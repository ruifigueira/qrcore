/*
 * pattern_detector.c
 *
 *  Created on: Oct 28, 2010
 *      Author: Rui Figueira
 */

#include <qrmobile/common/bit_ops.h>
#include <qrmobile/common/image_log.h>
#include <qrmobile/common/image.h>
#include <qrmobile/common/log.h>
#include <qrmobile/common/math.h>

#include <qrmobile/patterns/pattern_detector.h>

qr_size __qr_pattern_sizes_lookup_table[QR_PATTERN_MAX_SIZE + 1][5];
qr_bool __qr_pattern_sizes_lookup_table_initialized = QR_FALSE;

#define qr_sizes_for(pattern_size) ((qr_size_p) __qr_pattern_sizes_lookup_table[(pattern_size)])

void qr_init_pattern_sizes_lookup_table()
{
	if(__qr_pattern_sizes_lookup_table_initialized)
	{
		return;
	}

	__qr_pattern_sizes_lookup_table_initialized = QR_TRUE;

	qr_size pattern_size;
	for (pattern_size =  QR_PATTERN_MIN_SIZE;
	     pattern_size <= QR_PATTERN_MAX_SIZE;
	     pattern_size++)
	{
		qr_size_p sizes = qr_sizes_for(pattern_size);
		qr_size accum_sizes = 0;

		*sizes = qr_round(1 * pattern_size * 1.0 / 7) - accum_sizes; accum_sizes += *sizes; sizes++;
		*sizes = qr_round(2 * pattern_size * 1.0 / 7) - accum_sizes; accum_sizes += *sizes; sizes++;
		*sizes = qr_round(5 * pattern_size * 1.0 / 7) - accum_sizes; accum_sizes += *sizes; sizes++;
		*sizes = qr_round(6 * pattern_size * 1.0 / 7) - accum_sizes; accum_sizes += *sizes; sizes++;
		*sizes = pattern_size - accum_sizes;
	}
}

void qr_row_border_limits(qr_size_p limits, qr_uint_p row, qr_uint_p row_end, qr_size point_in_border)
{
	limits[0] = point_in_border - qr_bitset_most_right_ones_from(row, point_in_border) + 1;
	limits[1] = point_in_border + qr_bitset_most_left_ones_from (row, row_end, point_in_border) - 1;

	qr_assert(limits[0] <= limits[1]);

	qr_assert(qr_row_get(row, limits[0]));
	qr_assert(qr_row_get(row, limits[1]));
}

/*
 * result goes to other_row
 */
void qr_mutable_row_bitwise_and(qr_uint_p mutable_row, qr_uint_p row, qr_uint_p row_end)
{
	while(row != row_end)
	{
		*mutable_row &= *row;
		row++;
		mutable_row++;
	}
}

void qr_mutable_row_copy(qr_uint_p dest, qr_uint_p row, qr_uint_p row_end)
{
	while(row != row_end)
	{
		*dest = *row;
		row++;
		dest++;
	}
}

void qr_mutable_row_clear(qr_uint_p mutable_row, qr_uint_p mutable_row_end)
{
	while(mutable_row != mutable_row_end)
	{
		*mutable_row = 0;
		mutable_row++;
	}
}

void qr_mutable_row_set_region(qr_uint_p mutable_row, qr_size_p region_limits)
{
	qr_size left_word        	= region_limits[0] >> 5;
	qr_size left_word_offset 	= region_limits[0] & 0x1f;

	qr_size right_word       	= region_limits[1] >> 5;
	qr_size right_word_offset 	= region_limits[1] & 0x1f;

	qr_uint left_word_mask 		= 0xffffffff << left_word_offset;
	qr_uint right_word_mask 	= 0xffffffff >> (31 - right_word_offset);

	qr_size i;

	qr_size diff = right_word - left_word;

	switch (diff) {
		case 0:
			mutable_row[left_word] |= (left_word_mask & right_word_mask);
			break;
		case 1:
			mutable_row[left_word]  |= left_word_mask;
			mutable_row[right_word] |= right_word_mask;
			break;
		default:
			mutable_row[left_word]  |= left_word_mask;
			mutable_row[right_word] |= right_word_mask;

			for (i = left_word + 1; i != right_word; i++) mutable_row[i] = 0xffffffff;
			break;
	}
}

qr_bool qr_mutable_row_find_border_points(
		qr_int_p left_border_point,
		qr_int_p right_border_point,
		qr_uint_p mutable_row,
		qr_int_p left_prev_border_limits,
		qr_int_p right_prev_border_limits)
{
	qr_int border_point;
	qr_int border_left_limit, border_right_limit;

	border_left_limit  = left_prev_border_limits[0];
	border_right_limit = left_prev_border_limits[1];

	if(qr_row_get(mutable_row, border_right_limit))
		border_point = border_right_limit;
	else if(qr_row_get(mutable_row, border_left_limit))
		border_point = border_left_limit;
	else
	{
		border_point = border_right_limit - qr_bitset_most_right_zeros_from(mutable_row, border_right_limit);
		if(!qr_row_get(mutable_row, border_point) || border_point < border_left_limit || border_point > border_right_limit)
		{
			/* there is no border */
			return QR_FALSE;
		}
	}

	qr_assert(border_point >= border_left_limit && border_point <= border_right_limit);

	*left_border_point = border_point;

	border_left_limit  = right_prev_border_limits[0];
	border_right_limit = right_prev_border_limits[1];

	if(qr_row_get(mutable_row, border_right_limit))
		border_point = border_right_limit;
	else if(qr_row_get(mutable_row, border_left_limit))
		border_point = border_left_limit;
	else
	{
		border_point = border_right_limit - qr_bitset_most_right_zeros_from(mutable_row, border_right_limit);
		if(!qr_row_get(mutable_row, border_point) || border_point < border_left_limit || border_point > border_right_limit)
		{
			/* there is no border */
			return QR_FALSE;
		}
	}

	*right_border_point = border_point;

	return QR_TRUE;
}

qr_bool qr_check_border(qr_bit_matrix *image, qr_int start_y, qr_int pattern_left_x, qr_size pattern_width, qr_int inc_y)
{
	qr_size height = image->height;
	qr_size row_size = image->row_size;

	qr_uint mutable_row_data[QR_MAX_ROW_WORDS_SIZE];

	qr_uint_p mutable_row = mutable_row_data;

	qr_uint_p row = qr_image_get_row(image, start_y);

	qr_int left_border_limits[2];
	qr_int right_border_limits[2];

	qr_int y_limit = inc_y > 0 ? qr_min(height - 1, start_y + pattern_width) : qr_max(0, start_y - pattern_width);

	qr_int left_border_point  = pattern_left_x;
	qr_int right_border_point = pattern_left_x + pattern_width - 1;

	qr_int last_y = start_y;
	qr_int y;
	for(y = start_y + inc_y; (inc_y > 0 ? y <= y_limit : y >= y_limit); y += inc_y)
	{

		/* border limits must be black */
		qr_assert(qr_row_get(row, left_border_point));
		qr_assert(qr_row_get(row, right_border_point));

		qr_row_border_limits(left_border_limits,  row, row + row_size, left_border_point);
		qr_row_border_limits(right_border_limits, row, row + row_size, right_border_point);

		/* if left limits equals right limits, that means they just joined! */
		if(left_border_limits[0] == right_border_limits[0] && left_border_limits[1] == right_border_limits[1])
			return QR_TRUE;

		row = qr_image_get_row(image, y);

		qr_mutable_row_clear(mutable_row, mutable_row + row_size);

		/* draw only pattern borders */
		qr_mutable_row_set_region(mutable_row, left_border_limits);
		qr_mutable_row_set_region(mutable_row, right_border_limits);

		/* intersect previous border and current row */
		qr_mutable_row_bitwise_and(mutable_row, row, row + row_size);

		qr_bool found = qr_mutable_row_find_border_points(
				&left_border_point,
				&right_border_point,
				mutable_row,
				left_border_limits,
				right_border_limits);

		qr_point lp = { left_border_point,  y };
		qr_point rp = { right_border_point, y };
		qr_image_log_point(lp, QR_COLOR_ORANGE);
		qr_image_log_point(rp, QR_COLOR_ORANGE);

		/* TODO : we need to be more tolerant, and give it another try... */
		if(!found) return QR_FALSE;

		last_y = y;
	}

	return QR_FALSE;
}

qr_size qr_check_pattern_size(qr_size_p color_sizes_cursor, qr_size max_pattern_size)
{
	qr_size b1 = *color_sizes_cursor; color_sizes_cursor++;
	qr_size w1 = *color_sizes_cursor; color_sizes_cursor++;
	qr_size b2 = *color_sizes_cursor; color_sizes_cursor++;
	qr_size w2 = *color_sizes_cursor; color_sizes_cursor++;
	qr_size b3 = *color_sizes_cursor; color_sizes_cursor++;

	qr_size pattern_size = b1 + w1 + b2 + w2 + b3;

	if(pattern_size >= QR_PATTERN_MIN_SIZE && pattern_size <= max_pattern_size)
	{
		qr_size_p expected_sizes = qr_sizes_for(pattern_size);

		/* real differences are half the differences between each block size */
		qr_size diff =
			(qr_abs(b1 - expected_sizes[0]) +
			 qr_abs(w1 - expected_sizes[1]) +
			 qr_abs(b2 - expected_sizes[2]) +
			 qr_abs(w2 - expected_sizes[3]) +
			 qr_abs(b3 - expected_sizes[4])) >> 1;

		/* if diff < 12.5% of total size, add the result */
		if(diff < (pattern_size >> 3)) {
			return pattern_size;
		}
	}

	return 0;
}

qr_bool qr_pattern_result_border_position(
		qr_point *result,
		qr_bit_matrix *image,
		qr_int curr_x,
		qr_int curr_y,
		qr_int inc_x,
		qr_int inc_y)
{
	qr_int color_changes = 2;
	qr_bool last_set = QR_TRUE;

	qr_size width = image->width;
	qr_size height = image->height;

	qr_assert(qr_image_get(image, curr_x, curr_y));

	while(color_changes != -1)
	{
		curr_x += inc_x;
		curr_y += inc_y;

		if(curr_x < 0 || curr_x >= width || curr_y < 0 || curr_y >= height) return QR_FALSE;

		if(last_set != qr_image_get(image, curr_x, curr_y))
		{
			last_set = !last_set;
			color_changes--;
		}
	}

	result->x = curr_x;
	result->y = curr_y;

	return QR_TRUE;
}

qr_bool qr_pattern_result_fill_border_points(qr_pattern_result *result, qr_int center_x, qr_int center_y, qr_size pattern_size, qr_bit_matrix *image)
{
	qr_assert(qr_image_get(image, center_x, center_y));

	qr_point *point = result->border_points;

	qr_bool found;

	/* clockwise from top center */
	found = qr_pattern_result_border_position(point    , image, center_x, center_y,  0, -1); if(!found) return QR_FALSE;
	found = qr_pattern_result_border_position(point + 4, image, center_x, center_y,  0,  1); if(!found) return QR_FALSE;

	/* we now recalculate center_y, to let the center point more centered :) */
	qr_int new_center_y = (point[0].y + point[4].y) >> 1;
	if(qr_image_get(image, center_x, new_center_y))
		center_y = new_center_y;

	found = qr_pattern_result_border_position(point + 1, image, center_x, center_y,  1, -1); if(!found) return QR_FALSE;
	found = qr_pattern_result_border_position(point + 2, image, center_x, center_y,  1,  0); if(!found) return QR_FALSE;
	found = qr_pattern_result_border_position(point + 3, image, center_x, center_y,  1,  1); if(!found) return QR_FALSE;

	found = qr_pattern_result_border_position(point + 5, image, center_x, center_y, -1,  1); if(!found) return QR_FALSE;
	found = qr_pattern_result_border_position(point + 6, image, center_x, center_y, -1,  0); if(!found) return QR_FALSE;
	found = qr_pattern_result_border_position(point + 7, image, center_x, center_y, -1, -1); if(!found) return QR_FALSE;

	result->center.x = center_x;
	result->center.y = center_y;
	result->estimate_width = pattern_size;

	return QR_TRUE;
}

qr_bool qr_check_pattern_result_candidate(qr_pattern_result *results, qr_size num_results, qr_int center_x, qr_int center_y, qr_size width)
{
	qr_int pos;
	for (pos = 0; pos < num_results; pos++) {
		qr_pattern_result *result = &results[pos];

		qr_int curr_center_x = result->center.x;
		qr_int curr_center_y = result->center.y;
		qr_size half_width    = width >> 1;

		qr_int x_min = center_x - half_width;
		qr_int x_max = center_x + half_width;
		qr_int y_min = center_y - half_width;
		qr_int y_max = center_y + half_width;

		if(curr_center_x > x_min && curr_center_x < x_max && curr_center_y > y_min && curr_center_y < y_max)
		{
			return QR_FALSE;
		}
	}

	return QR_TRUE;
}


qr_size qr_process_image_row(
		qr_pattern_result *results,
		qr_size           result_size,
		qr_bit_matrix     *image,
		qr_int           y)
{
	qr_size row_size = image->row_size;

	/* for bit operations, we are more interested in row size,
	 * not in the *real* image width */
	qr_size width = row_size << 5;

	qr_uint_p row = qr_image_get_row(image, y);
	qr_uint_p row_end = row + row_size;

	qr_size max_pattern_size = qr_min(width >> 1, QR_PATTERN_MAX_SIZE);

	qr_size color_sizes_array[QR_MAX_IMAGE_WIDTH];
	qr_size *color_sizes_cursor = color_sizes_array;

	qr_bool starts_black = qr_row_get(row, 0);

	qr_int s = starts_black ? 0 : qr_bitset_most_left_zeros_from(row, row_end, 0); if (s == width) goto return_point;

	/* we check each five color blocks (black. white, black, white, black)
	 * trying to find patterns with the right proportions */
	*color_sizes_cursor = qr_bitset_most_left_ones_from (row, row_end, s); s += *color_sizes_cursor; if (s >= width) goto return_point; color_sizes_cursor++;
	*color_sizes_cursor = qr_bitset_most_left_zeros_from(row, row_end, s); s += *color_sizes_cursor; if (s >= width) goto return_point; color_sizes_cursor++;
	*color_sizes_cursor = qr_bitset_most_left_ones_from (row, row_end, s); s += *color_sizes_cursor; if (s >= width) goto return_point; color_sizes_cursor++;

	for(;;)
	{
		*color_sizes_cursor = qr_bitset_most_left_zeros_from(row, row_end, s); s += *color_sizes_cursor; if (s >= width) goto return_point; color_sizes_cursor++;
		*color_sizes_cursor = qr_bitset_most_left_ones_from (row, row_end, s); s += *color_sizes_cursor; if (s >= width) goto return_point; color_sizes_cursor++;

		qr_size pattern_size = qr_check_pattern_size(color_sizes_cursor - 5, max_pattern_size);

		if(pattern_size != 0)
		{
			qr_int start_x = s - pattern_size;
			qr_int center_x = s - (pattern_size >> 1);

			/* first we need to check if there is a result close to this */
			if(pattern_size > 0)
			{
				if(!qr_check_pattern_result_candidate(results, result_size, center_x, y, pattern_size))
				{
					/* there is a pattern result for that pattern position */
					continue;
				}
			}

			qr_int inc_y = qr_max(1, pattern_size >> 4);

			qr_assert(qr_row_get(row, start_x));

			if(!qr_check_border(image, y, start_x, pattern_size, -inc_y)) continue;
			if(!qr_check_border(image, y, start_x, pattern_size,  inc_y)) continue;

			/* if borders are closed above and below y, then we have a position pattern! */
			qr_pattern_result *result = results + result_size;

			if(!qr_pattern_result_fill_border_points(result, center_x, y, pattern_size, image)) continue;

			result_size++;

			if(result_size == QR_NUM_PATTERNS) goto return_point;
		}
	}

	goto return_point;

return_point: /* return point label */
	return result_size;
}


qr_size qr_detect_position_patterns(qr_pattern_result *results, qr_bit_matrix *image)
{
	/* we must ensure we don't forget to initialize our lookup table... */
	qr_init_pattern_sizes_lookup_table();

	qr_size height = image->height;
	qr_size ln_height = 32 - qr_most_right_zeros(height - 1);

	qr_size result_size = 0;

	/* perform a binary scanning through lines.
	 * It will not check y = 0, but we don't really care :) */
	qr_int d, k;

	for (d = 0; d < ln_height; d++)
	{
		qr_size num_divisions = (1 << d);
		qr_size division_height = (1 << ln_height) >> d;

		qr_int y = (division_height >> 1);

		for(k = 0; k < num_divisions; k++)
		{
			if(y >= height) break;

			result_size = qr_process_image_row(results, result_size, image, y);
			if(result_size == QR_NUM_PATTERNS) goto return_label;

			y += division_height;
		}
	}

	goto return_label;

return_label:
	return result_size;
}

void qr_pattern_detector_init()
{
	qr_init_pattern_sizes_lookup_table();
}


