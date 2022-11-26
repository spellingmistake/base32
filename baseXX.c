#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "baseXX.h"

static char *alloc_output_buffer_to_baseXX(size_t len, size_t base_bits)
{
	char *dst = NULL;

	len = ((len + base_bits - 1) / base_bits) * 8;
	len += len / OUTPUT_WIDTH + 1;

	dst = malloc(len);
	dst[len - 1] = '\0';

	return dst;
}

static char byte_to_baseXX(const char *ptr, size_t len, size_t ptr_idx,
		baseXX_conversion_t *conv, size_t byte_conv_idx)
{
	char c[2] = { 0 };
	size_t i, offset;
	baseXX_byte_conversion_t *byte_conv = &conv->conv_array[byte_conv_idx];

	for (i = 0; i < 2; ++i)
	{
		offset = ptr_idx + byte_conv->offset[i];

		if (offset >= len)
		{
			if (!i)
			{	/* end of input, add padding sign '=' */
				c[0] = conv->alphabet_size - 1;
			}
			break;
		}

		c[i] |= (ptr[offset] & byte_conv->mask[i]);

		if (byte_conv->shift[i] < 0)
		{
			c[i] >>= -byte_conv->shift[i];
		}
		else if (byte_conv->shift[i] > 0)
		{
			c[i] <<= byte_conv->shift[i];
		}
	}

	return conv->alphabet[c[0] | c[1]];
}

static char func(char c, baseXX_conversion_t *conv)
{
	size_t i;
	char diff = 0;
	baseXX_char_range_t *char_range;

	for (i = 0; i < conv->char_range_size; ++i)
	{
		char_range = &conv->char_ranges[i];

		if (c >= char_range->lower && c <= char_range->upper)
		{
			diff = char_range->diff;
			break;
		}
	}

	return diff;
}

static size_t byte_from_baseXX(char c, char *dst, size_t *dst_idx,
		bool *printable, baseXX_conversion_t *conv, size_t byte_conv_idx)
{
	size_t i;
	int ret = 0;
	char cc, diff;
	baseXX_byte_conversion_t *byte_conv = &conv->conv_array[byte_conv_idx];

	diff = func(c, conv);

	if (diff)
	{
		c -= diff;

		for (i = 0; i < 2; ++i)
		{
			if (!byte_conv->mask[i])
			{
				continue;
			}

			if (byte_conv->shift[i] < 0)
			{
				cc = c << -byte_conv->shift[i];
			}
			else if (byte_conv->shift[i] > 0)
			{
				cc = c >> byte_conv->shift[i];
			}
			else
			{
				cc = c;
			}

			dst[*dst_idx + i] |= (cc & byte_conv->mask[i]);
		}

		i = byte_conv->offset[1] - byte_conv->offset[0];

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
	else if ('\n' != c && '\r' != c && PADDING != c)
	{
		fprintf(stderr, "invalid input character 0x%02hhx\n", c);
	}

	return ret;
}

char *str_to_baseXX(const char *ptr, size_t len, baseXX_conversion_t *conv)
{
	char *dst;
	size_t i, idx_in, idx_out, end, base_bits;

	dst = alloc_output_buffer_to_baseXX(len, conv->base_bits);

	base_bits= conv->base_bits;
	idx_in = idx_out = 0;
	end = ((len + base_bits - 1) / base_bits) * base_bits;

	while (idx_in < end)
	{
		for (i = 0; i < conv->conv_array_size; ++i)
		{
			dst[idx_out++] = byte_to_baseXX(ptr, len, idx_in, conv, i);
			if ((idx_out % (OUTPUT_WIDTH + 1)) == (OUTPUT_WIDTH))
			{
				dst[idx_out++] = '\n';
			}
		}
		idx_in += base_bits;
	}

	return dst;
}

char *baseXX_to_str(const char *ptr, size_t len, size_t *ptr_len,
		bool *printable, baseXX_conversion_t *conv)
{
	char c;
	const char *tmp;
	char *dst = NULL;
	size_t i, inc, idx, dst_idx, dst_len;

	if (len)
	{
		tmp = strchr(ptr, PADDING) ?: ptr + len;

		if (tmp - ptr)
		{
			dst_len = ((tmp - ptr) + 7) / 8 * conv->base_bits + 1;
			// TODO Idkw, but if I use malloc here, valgrind complains...
			dst = calloc(dst_len, 1);
		}
	}

	i = idx = dst_idx = 0;

	while (idx < len)
	{
		c = ptr[idx];
		inc = byte_from_baseXX(c, dst, &dst_idx, printable, conv, i);
		i = (i + inc) % conv->conv_array_size;
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