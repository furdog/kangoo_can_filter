#include "kangoo_data_extractor.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Sample data: */
uint8_t sample_x424[8U] = {
	0x11U, 0x40U, 0x1AU, 0xA0U, 0x40U, 0x34U, 0x55U, 0x41U
};

uint8_t sample_x425[8U] = {
	0x0AU, 0x4EU, 0x44U, 0x9CU, 0x42U, 0x5CU, 0x01U, 0x2DU
};

uint8_t sample_x445[7U] = {
	0x40U, 0x55U, 0x55U, 0x83U, 0x2EU, 0x96U, 0x80U
};

uint8_t sample_x445_test_write[7U] = {
	0x40U, 0x55U, 0x55U, 0x00U, 0x00U, 0x00U, 0x00U
};

int main()
{
	/* Check x425 values are correct */
	assert(kangoo_x424_get_max_batt_temp_C(sample_x424) == 25U);
	assert((uint16_t)(kangoo_x425_get_highest_cell_V(sample_x425) *
			  100.0f) == 402U);
	assert((uint16_t)(kangoo_x425_get_lowest_cell_V(sample_x425) *
			  100.0f) == 401U);

	/* Check x445 values are correct */
	assert(kangoo_x445_get_max_batt_temp_C(sample_x445) == 25U);
	assert((uint16_t)(kangoo_x445_get_highest_cell_V(sample_x445) *
			  100.0f) == 402U);
	assert((uint16_t)(kangoo_x445_get_lowest_cell_V(sample_x445) *
			  100.0f) == 401U);

	/* Test write 445 and compare with original sample */
	assert(memcmp(sample_x445, sample_x445_test_write, 7U) != 0U);

	kangoo_x445_set_max_batt_temp_C(sample_x445_test_write, 25);
	assert(kangoo_x445_get_max_batt_temp_C(sample_x445_test_write) == 25);

	kangoo_x445_set_highest_cell_V(sample_x445_test_write, 4.02);
	assert((uint16_t)(kangoo_x445_get_highest_cell_V(
				    sample_x445_test_write) * 100.0f) == 402U);

	kangoo_x445_set_lowest_cell_V(sample_x445_test_write, 4.01);
	assert((uint16_t)(kangoo_x445_get_lowest_cell_V(
				    sample_x445_test_write) * 100.0f) == 401U);

	assert(memcmp(sample_x445, sample_x445_test_write, 7U) == 0U);

	/* Check changed x445 values are correct */
	kangoo_x445_set_max_batt_temp_C(sample_x445, -13);
	assert(kangoo_x445_get_max_batt_temp_C(sample_x445) == -13);

	kangoo_x445_set_highest_cell_V(sample_x445, 2.13);
	assert((uint16_t)(kangoo_x445_get_highest_cell_V(sample_x445) *
			  100.0f) == 213U);

	kangoo_x445_set_lowest_cell_V(sample_x445, 2.13);
	assert((uint16_t)(kangoo_x445_get_lowest_cell_V(sample_x445) *
			  100.0f) == 213U);

	return 0;
}
