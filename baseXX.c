#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "baseXX.h"

static char *alloc_output_buffer_to_baseXX(size_t len, size_t bits_per_chunk,
	size_t base_bits)
{
	size_t modulus;
	char *dst = NULL;

	/* output characters only */
	len = (len * 8 + (base_bits - 1)) / base_bits;
	/* padding */
	modulus = len % (bits_per_chunk / base_bits);
	len += modulus ? (bits_per_chunk / base_bits) - modulus : 0;
	/* line breaks */
	len += ((len + OUTPUT_WIDTH - 1) / OUTPUT_WIDTH) - 1;

	/* + 1 for the terminating '\0' */
	dst = malloc(len + 1);
	dst[len] = 0;

	return dst;
}

static char *alloc_output_buffer_from_baseXX(const char *ptr, size_t len,
		baseXX_conversion_t *conv)
{
	char *dst = NULL;
	const char *pad, *tmp;
	size_t newlines, base_bits;

	if (len)
	{
		pad = strchr(ptr, PADDING) ?: ptr + len;

		if (pad - ptr)
		{
			newlines = 0;
			tmp = ptr;

			while ((tmp = strchr(tmp, '\n')) && (tmp < pad))
			{
				++newlines;
				++tmp;
			}

			base_bits = conv->base_bits;
			/* without padding */
			len = (pad - ptr);
			/* without newlines */
			len -= newlines;
			/* mapped onto byte size */
			len = (len * base_bits + base_bits - 1) / 8;

			/* the output buffer from baseXX is logically or'd repeatedly;
			 * hence it is crucial to work on properly zeroed memory */
			dst = calloc(len + 1, 1);
		}
	}

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

	if ((size_t)(c[0] | c[1]) > (conv->alphabet_size - 1))
	{
		fprintf(stderr, "converted value 0x%02hhx at postion %zu of input is "
				"outside the destination alphabet\n", (u_char)(c[0] | c[1]),
				offset);
		c[0] = conv->alphabet_size - 1;
		c[1] = 0;
	}

	return conv->alphabet[c[0] | c[1]];
}

static char get_value(char c, baseXX_conversion_t *conv)
{
	size_t i;
	char value = -1, v = 0;
	baseXX_char_range_t *char_range;

	for (i = 0; i < conv->char_range_size; ++i)
	{
		char_range = &conv->char_ranges[i];

		if (c >= char_range->lower && c <= char_range->upper)
		{
			value = v + (c - char_range->lower);
			break;
		}
		v += (char_range->upper - char_range->lower) + 1;
	}

	return value;
}

static size_t byte_from_baseXX(char c, char *dst, size_t *dst_idx,
		bool *printable, baseXX_conversion_t *conv, size_t byte_conv_idx)
{
	size_t i;
	char cc, v;
	int ret = 0;
	baseXX_byte_conversion_t *byte_conv = &conv->conv_array[byte_conv_idx];

	v = get_value(c, conv);

	if (v != -1)
	{
		for (i = 0; i < 2; ++i)
		{
			if (!byte_conv->mask[i])
			{
				continue;
			}

			if (byte_conv->shift[i] < 0)
			{
				cc = v << -byte_conv->shift[i];
			}
			else if (byte_conv->shift[i] > 0)
			{
				cc = v >> byte_conv->shift[i];
			}
			else
			{
				cc = v;
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
	size_t i, idx_in, idx_out, end, offset;

	offset = conv->conv_array_size * conv->base_bits;
	dst = alloc_output_buffer_to_baseXX(len, offset, conv->base_bits);
	offset /= 8;

	idx_in = idx_out = 0;
	end = ((len + offset - 1) / offset) * offset;

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
		idx_in += offset;
	}

	return dst;
}

char *baseXX_to_str(const char *ptr, size_t len, size_t *ptr_len,
		bool *printable, baseXX_conversion_t *conv)
{
	char c;
	char *dst = NULL;
	size_t i, inc, idx, dst_idx;

	dst = alloc_output_buffer_from_baseXX(ptr, len, conv);

	if (dst)
	{
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
	}

	return dst;
}
