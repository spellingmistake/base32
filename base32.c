#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define __UNUSED __attribute__((unused))
#define __PACKED __attribute__((packed))
#define __CLEANUP(f) __attribute__((cleanup(f)))

#define OUTPUT_WIDTH 76
#define PADDING '='
#define CONV_ARRAY_SIZE (sizeof(b32_convs) / sizeof(b32_convs[0]))

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

typedef struct baseXX_conversion_t baseXX_conversion_t;

struct baseXX_conversion_t {
	unsigned char offset[2];
	char mask[2];
	char shift[2];
}  __PACKED b32_convs[8] = {
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

char alphabeth[33] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5',
	'6', '7', PADDING,
};

static void cleanup_free(char **ptr)
{
	free(*ptr);
}

static inline char byte_to_base32(const char *ptr, size_t len, size_t idx,
		baseXX_conversion_t *b32_conv)
{
	char c[2] = { 0 };
	size_t i;

	for (i = 0; i < 2; ++i)
	{
		size_t offset = idx + b32_conv->offset[i];
		if (offset >= len)
		{
			if (!i)
			{	/* end of input, add padding sign '=' */
				c[0] = 32;
			}
			break;
		}

		c[i] |= (ptr[offset] & b32_conv->mask[i]);

		if (b32_conv->shift[i] < 0)
		{
			c[i] >>= -b32_conv->shift[i];
		}
		else if (b32_conv->shift[i] > 0)
		{
			c[i] <<= b32_conv->shift[i];
		}
	}

	return alphabeth[c[0] | c[1]];
}

static char *alloc_output_buffer_to_base32(size_t len)
{
	char *dst = NULL;

	len = ((len + 4) / 5) * 8;
	len += len / OUTPUT_WIDTH + 1;

	dst = malloc(len);
	dst[len - 1] = '\0';

	return dst;
}

static char *str_to_base32(const char *ptr, size_t len)
{
	char *dst;
	size_t i, idx_in, idx_out, end;

	dst = alloc_output_buffer_to_base32(len);
	idx_in = idx_out = 0;
	end = ((len  + 4) / 5) * 5;

	while (idx_in < end)
	{
		for (i = 0; i < CONV_ARRAY_SIZE; ++i)
		{
			dst[idx_out++] = byte_to_base32(ptr, len, idx_in, &b32_convs[i]);
			if ((idx_out % (OUTPUT_WIDTH + 1)) == (OUTPUT_WIDTH))
			{
				dst[idx_out++] = '\n';
			}
		}
		idx_in += 5;
	}

	return dst;
}

static size_t byte_from_base32(char c, baseXX_conversion_t *b32_conv,
		char *dst, size_t *dst_idx, bool *printable)
{
	size_t i;
	int ret = 0;
	char cc, diff = 0;

	if (c >= 'A' && c <= 'Z')
	{	/* values 0x00 - 0x */
		diff = 'A';
	}
	else if (c >= '2' && c <= '7')
	{	/* - 1 because zero and one are missing */
		diff = 'Z' - 'A' - 1;
	}
	else if ('\n' != c && '\r' != c && PADDING != c)
	{
		fprintf(stderr, "invalid input character 0x%02hhx\n", c);
	}

	if (diff)
	{
		c -= diff;

		for (i = 0; i < 2; ++i)
		{
			if (!b32_conv->mask[i])
			{
				continue;
			}

			if (b32_conv->shift[i] < 0)
			{
				cc = c << -b32_conv->shift[i];
			}
			else if (b32_conv->shift[i] > 0)
			{
				cc = c >> b32_conv->shift[i];
			}
			else
			{
				cc = c;
			}

			dst[*dst_idx + i] |= (cc & b32_conv->mask[i]);
		}

		i = b32_conv->offset[1] - b32_conv->offset[0];

		if (printable && i)
		{
			if (*printable && !isprint(dst[*dst_idx]))
			{
				*printable = false;
			}
		}

		*dst_idx += i;
		ret = 1;
	}

	return ret;
}

char *base32_to_str(const char *ptr, size_t len, size_t *ptr_len,
	bool *printable)
{
	char c;
	const char *tmp;
	char *dst = NULL;
	size_t i, inc, idx, dst_idx, dst_len;

	if (len)
	{
		tmp = strchr(ptr, '=') ?: ptr + len;

		if (tmp - ptr)
		{
			dst_len = ((tmp - ptr) + 7) / 8 * 5 + 1;
			// TODO Idkw, but if I use malloc here, valgrind complains...
			dst = calloc(dst_len, 1);
		}
	}

	i = idx = dst_idx = 0;

	while (idx < len)
	{
		c = ptr[idx];
		inc = byte_from_base32(c, &b32_convs[i], dst, &dst_idx, printable);
		i = (i + inc) % CONV_ARRAY_SIZE;
		++idx;
	}

	if (ptr_len)
	{
		*ptr_len = dst_idx;
	}

	// TODO: see above comment about valgrind
	//if (dst)
	//{
	//	memset(&dst[dst_idx], 0, dst_len - dst_idx);
	//}

	return dst;
}

int main(int argc __UNUSED, char *argv[] __UNUSED)
{
	size_t len;
	bool printable = true;
	char *input, *b32 __CLEANUP(cleanup_free) = NULL,
		 *buf __CLEANUP(cleanup_free) = NULL;

	input = "bin  data  Desktop  Documents  Downloads  GNUstep  i  Mail  mbox  "
		"share  tmp  tmux.conf  usr  var";

	b32 = str_to_base32(input, strlen(input));
	fprintf(stdout, "%s\n", b32);
	buf = base32_to_str(b32, strlen(b32), &len, &printable);
	if (printable)
	{
		fprintf(stdout, "%s\n", buf);
	}
	else
	{
		fprintf(stdout, "output contains unprintable characters\n", buf);
	}

	assert(len == strlen(input));
	assert(!memcmp(buf, input, len));

	return 0;
}
