#ifndef   KANGOO_DATA_EXTRACTOR_GUARD
#define   KANGOO_DATA_EXTRACTOR_GUARD

#include <stdint.h>

/******************************************************************************
 * x424
 *****************************************************************************/
/* 7 BIT (Probably unsigned) BIG ENDIAN */
int16_t kangoo_x424_get_max_batt_temp_C(uint8_t *data)
{
	uint8_t val = 0;

	uint8_t mask = 0xFFU >> (8U - 7U);

	val |= ((uint8_t)data[7U] << 0U);

	return (val & mask) - 40U;
}

/* 9 BIT (Probably unsigned) BIG ENDIAN */
float kangoo_x425_get_highest_cell_V(uint8_t *data)
{
	uint16_t val = 0;

	uint16_t mask = 0xFFFFU >> (16U - 9U);

	val |= ((uint16_t)data[4U] << 8U);
	val |= ((uint16_t)data[5U] << 0U);

	return (((val >> 1) & mask) * 0.01f) + 1U;
}

/* 9 BIT (Probably unsigned) BIG ENDIAN */
float kangoo_x425_get_lowest_cell_V(uint8_t *data)
{
	uint16_t val = 0;

	uint16_t mask = 0xFFFFU >> (16U - 9U);

	val |= ((uint16_t)data[6U] << 8U);
	val |= ((uint16_t)data[7U] << 0U);

	return ((val & mask) * 0.01f) + 1U;
}

/******************************************************************************
 * x445
 *****************************************************************************/
/* 7 BIT (Probably unsigned) BIG ENDIAN */
int16_t kangoo_x445_get_max_batt_temp_C(uint8_t *data)
{
	uint8_t val = 0;

	uint8_t mask = 0xFFU >> (8U - 7U);

	val |= ((uint8_t)data[3U] << 0U);

	return ((val >> 1U) & mask) - 40U;
}

void kangoo_x445_set_max_batt_temp_C(uint8_t *data, int16_t _val)
{
	int16_t val = (_val + 40);

	uint8_t mask = (uint8_t)~(0xFFU << 1U);

	data[3U] &= mask;
	data[3U] |= ((uint8_t)val << 1U);
}

/* 9 BIT (Probably unsigned) BIG ENDIAN */
float kangoo_x445_get_highest_cell_V(uint8_t *data)
{
	uint16_t val = 0;

	uint16_t mask = 0xFFFFU >> (16U - 9U);

	val |= ((uint16_t)data[3U] << 8U);
	val |= ((uint16_t)data[4U] << 0U);

	return ((val & mask) * 0.01f) + 1U;
}

void kangoo_x445_set_highest_cell_V(uint8_t *data, float _val)
{
	uint16_t val = (uint16_t)((_val - 1.0f) * 100U);

	uint16_t mask = (uint16_t)~(0xFFFFU >> 7U);

	data[3U] &= (mask >> 8U) & 0xFFU;
	data[4U] &= (mask >> 0U) & 0xFFU;

	data[3U] |= (val >> 8U) & 0xFFU;
	data[4U] |= (val >> 0U) & 0xFFU;
}

/* 9 BIT (Probably unsigned) BIG ENDIAN */
float kangoo_x445_get_lowest_cell_V(uint8_t *data)
{
	uint16_t val = 0;

	uint16_t mask = 0xFFFFU >> (16U - 9U);

	val |= ((uint16_t)data[5U] << 8U);
	val |= ((uint16_t)data[6U] << 0U);

	return (((val >> 7U) & mask) * 0.01f) + 1U;
}

void kangoo_x445_set_lowest_cell_V(uint8_t *data, float _val)
{
	uint16_t val = ((uint16_t)((_val - 1.0f) * 100U) << 7U);

	uint16_t mask = (uint16_t)~(0xFFFFU << 7U);

	data[5U] &= (mask >> 8U) & 0xFFU;
	data[6U] &= (mask >> 0U) & 0xFFU;

	data[5U] |= (val >> 8U) & 0xFFU;
	data[6U] |= (val >> 0U) & 0xFFU;
}

#endif /* KANGOO_DATA_EXTRACTOR_GUARD */
