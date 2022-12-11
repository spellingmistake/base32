#include "base32.h"

static const char base32_alphabets[] = {
	/* Base 32 Encoding */
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5',
	'6', '7', PADDING,
	/* Base 32 Encoding with Extended Hex Alphabet */
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
	'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', PADDING,
};

/*
 * |       |               0               |               1           ...
 * |       | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 |
 * |-------^-------------------^-----------|-------^-------------------^
 * | mask  | 0xf8              | 0x07      | 0xc0  | 0x3e              |
 * | shift | -3                | 2         | -6    | -1                |
 * |-------^-------------------^-------------------^-------------------^
 * |       |         0         |         1         |         2         |
 * -
 * |       | 1 |               2               |               3       ...
 * |       | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 |
 * |-------|---|---------------^---------------|---^-------------------^
 * | mask  |0x1| 0xf0          | 0x0f          |x80| 0x7c              |
 * | shift | 4 | -4            | 1             |-7 | -2                |
 * |-------|---^---------------^-------------------^-------------------^
 * |       |   |     3         |         4         |         5         |
 * -
 * |       |   3   |               4               |
 * |       | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * |-------|-------|-----------^-------------------^
 * | mask  | 0x03  | 0xe0      | 0x1f              |
 * | shift | 3     | -5        | 0                 |
 * |-------|-------------------^-------------------^
 * |       |         6         |         7         |
 */
static baseXX_byte_conversion_t base32_byte_conversion[] =
{
	{	/* 0 */
		.offset = { 0, 0 },
		.mask =   { 0xf8, 0x00 },
		.shift =  { -3, 0 },
	},
	{	/* 1 */
		.offset = { 0, 1 },
		.mask =   { 0x07, 0xc0 },
		.shift =  { 2, -6 },
	},
	{	/* 2 */
		.offset = { 1, 1 },
		.mask =   { 0x3e, 0x00 },
		.shift =  { -1, 0 },
	},
	{	/* 3 */
		.offset = { 1, 2 },
		.mask =   { 0x01, 0xf0 },
		.shift =  { 4, -4 },
	},
	{	/* 4 */
		.offset = { 2, 3 },
		.mask =   { 0x0f, 0x80 },
		.shift =  { 1, -7 },
	},
	{	/* 5 */
		.offset = { 3, 3 },
		.mask =   { 0x7c, 0x00 },
		.shift =  { -2, 0 },
	},
	{	/* 6 */
		.offset = { 3, 4 },
		.mask =   { 0x03, 0xe0 },
		.shift =  { 3, -5 },
	},
	{	/* 7, offset 5 doesn't exist, but as the diff of the offsets
		 * is used to move dst_ptr forward during decoding, we need it */
		.offset = { 4, 5 },
		.mask =   { 0x1f, 0x00 },
		.shift =  { 0, 0 },
	},
};

static baseXX_char_range_t base32_char_ranges[] =
{
	{
		.lower = 0,
		.upper = 25,
	},
	{
		.lower = 26,
		.upper = 31,
	},
};

static baseXX_char_range_t base32_char_ranges_ext_hex[] =
{
	{
		.lower = 0,
		.upper = 9,
	},
	{
		.lower = 10,
		.upper = 31,
	},
};

static baseXX_conversion_t base32_conversion = {
	.alphabet = base32_alphabets,
	.alphabet_size = sizeof(base32_alphabets) / 2,
	.base_bits = 5,
	.conv_array_size = ARRAY_SIZE(base32_byte_conversion),
	.char_range_size = ARRAY_SIZE(base32_char_ranges),
	.conv_array = base32_byte_conversion,
	.char_ranges = base32_char_ranges,
};

static baseXX_conversion_t base32_conversion_ext_hex = {
	.alphabet = &base32_alphabets[sizeof(base32_alphabets) / 2],
	.alphabet_size = sizeof(base32_alphabets) / 2,
	.base_bits = 5,
	.conv_array_size = ARRAY_SIZE(base32_byte_conversion),
	.char_range_size = ARRAY_SIZE(base32_char_ranges_ext_hex),
	.conv_array = base32_byte_conversion,
	.char_ranges = base32_char_ranges_ext_hex,
};

char *str_to_base32(const char *ptr, size_t len)
{
	return str_to_baseXX(ptr, len, &base32_conversion);
}

char *base32_to_str(const char *ptr, size_t len, size_t *ptr_len,
	bool *printable)
{
	return baseXX_to_str(ptr, len, ptr_len, printable, &base32_conversion);
}

char *str_to_base32_ext_hex(const char *ptr, size_t len)
{
	return str_to_baseXX(ptr, len, &base32_conversion_ext_hex);
}

char *base32_to_str_ext_hex(const char *ptr, size_t len, size_t *ptr_len,
	bool *printable)
{
	return baseXX_to_str(ptr, len, ptr_len, printable,
			&base32_conversion_ext_hex);
}
