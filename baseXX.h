#ifndef _BASEXX_H_
#define _BASEXX_H_

#include <stdio.h>
#include <stdbool.h>

#ifndef __UNUSED
# define __UNUSED __attribute__((unused))
#endif

#ifndef __PACKED
# define __PACKED __attribute__((packed))
#endif

#define OUTPUT_WIDTH 76
#define PADDING '='

#define ARRAY_SIZE(convs) ((sizeof(convs) / sizeof((convs)[0])))

typedef struct baseXX_conversion_t baseXX_conversion_t;
typedef struct baseXX_char_range_t baseXX_char_range_t;
typedef struct baseXX_byte_conversion_t baseXX_byte_conversion_t;

struct baseXX_char_range_t {
	char lower;
	char upper;
} __PACKED;

struct baseXX_byte_conversion_t {
	unsigned char offset[2];
	char mask[2];
	char shift[2];
} __PACKED;

struct baseXX_conversion_t {
	char *alphabet;
	size_t alphabet_size;
	size_t base_bits;
	size_t conv_array_size;
	size_t char_range_size;
	baseXX_byte_conversion_t *conv_array;
	baseXX_char_range_t *char_ranges;
};

char *str_to_baseXX(const char *ptr, size_t len, baseXX_conversion_t *conv);

char *baseXX_to_str(const char *ptr, size_t len, size_t *ptr_len,
	bool *printable, baseXX_conversion_t *conv);

#endif	/* #ifndef _BASEXX_H_ */
