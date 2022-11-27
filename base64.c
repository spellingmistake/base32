#include "base64.h"

static char base64_alphabeth[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '+', '/', PADDING,
};

/*
 * |       |               0               |               1               ...
 * |       | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * |-------^-----------------------^-------|---------------^---------------|
 * | mask  | 0xfc                  | 0x03  | 0xf0          | 0x0f          |
 * | shift | -2                    | 4     | -4            | 2             |
 * |-------^-----------------------^-----------------------^----------------
 * |       |           0           |           1           |       2
 * -
 * |       |               2               |
 * |       | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * |-------|-------^-----------------------^
 * | mask  | 0xc0  | 0x3f                  |
 * | shift | -6    | 0                     |
 * |-------|-------^-----------------------^
 * |       |   2   |           3           |
 */
static baseXX_byte_conversion_t base64_byte_conversion[] =
{
	{	/* 0 */
		.offset = { 0, 0 },
		.mask =   { 0xfc, 0x00 },
		.shift =  { -2, 0 },
	},
	{	/* 1 */
		.offset = { 0, 1 },
		.mask =   { 0x03, 0xf0 },
		.shift =  { 4, -4 },
	},
	{	/* 2 */
		.offset = { 1, 2 },
		.mask =   { 0x0f, 0xc0 },
		.shift =  { 2, -6 },
	},
	{	/* 3 */
		.offset = { 2, 3 },
		.mask =   { 0x3f, 0x00 },
		.shift =  { 0, 0 },
	},
};

static baseXX_char_range_t base64_char_ranges[] =
{
	{
		.lower = 'A',
		.upper = 'Z',
	},
	{
		.lower = 'a',
		.upper = 'z',
	},
	{
		.lower = '0',
		.upper = '9',
	},
	{
		.lower = '+',
		.upper = '+',
	},
	{
		.lower = '/',
		.upper = '/',
	},
};

baseXX_conversion_t base64_conversion = {
	.alphabet = base64_alphabeth,
	.alphabet_size = sizeof(base64_alphabeth),
	.base_bits = 6,
	.conv_array_size = ARRAY_SIZE(base64_byte_conversion),
	.char_range_size = ARRAY_SIZE(base64_char_ranges),
	.conv_array = base64_byte_conversion,
	.char_ranges = base64_char_ranges,
};

char *str_to_base64(const char *ptr, size_t len)
{
	return str_to_baseXX(ptr, len, &base64_conversion);
}

char *base64_to_str(const char *ptr, size_t len, size_t *ptr_len,
	bool *printable)
{
	return baseXX_to_str(ptr, len, ptr_len, printable, &base64_conversion);
}
