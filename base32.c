#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef __UNUSED
# define __UNUSED __attribute__((unused))
#endif

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
}  __attribute__((packed)) b32_convs[8] = {
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

char alphabeth[32] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5',
	'6', '7',
};

static inline char byte_to_base32(const char *ptr, baseXX_conversion_t *b32_conv)
{
	char c[2] = { 0 };
	size_t i;

	for (i = 0; i < 2; ++i)
	{
		c[i] |= (ptr[b32_conv->offset[i]] & b32_conv->mask[i]);
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

static void str_to_base32(const char *ptr, size_t len)
{
	char buf[5];
	size_t i, j, idx, end;

	j = idx = 0;
	end = (len / 5) * 5;

	while (idx < end)
	{
		for (i = 0; i < sizeof(b32_convs) / sizeof(b32_convs[0]); ++i)
		{
			fprintf(stdout, "%c%s", byte_to_base32(&ptr[idx], &b32_convs[i]),
					!(++j % 76) ? "\n": "");
		}
		idx += 5;
	}

	if (len - end)
	{
		memcpy(buf, &ptr[end], len - end);
		memset(&buf[len - end], 0, 5 - len + end);
		end = (((len - end) * 8 + 4)/ 5);

		for (i = 0; i < sizeof(b32_convs) / sizeof(b32_convs[0]); ++i)
		{
			fprintf(stdout, "%c%s", (i < end) ?
					byte_to_base32(buf, &b32_convs[i]) : '=',
					!(++j % 76) ? "\n": "");
		}
	}
}

static size_t byte_from_base32(char c, baseXX_conversion_t *b32_conv,
		char *dst, size_t *dst_idx)
{
	int i;
	char cc;

	if (c >= 'A' && c <= 'Z')
	{	/* values 0x00 - 0x */
		c -= 'A';
	}
	else if (c >= '2' && c <= '7')
	{	/* - 1 because zero and one are missing */
		c -= 'Z' - 'A' - 1;
	}
	else
	{
		return 0;
	}

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

	*dst_idx += b32_conv->offset[1] - b32_conv->offset[0];
	return 1;
}

char *base32_to_str(const char *ptr, size_t len, size_t *ptr_len)
{
	char c;
	size_t i, inc, idx, dst_idx;
	char *tmp, *dst = NULL;

	if (len)
	{
		tmp = strchr(ptr, '=');

		if (tmp)
		{	/* drop padding */
			len = tmp - ptr;
		}

		if (len)
		{
			dst = malloc(len);
		}
	}

	i = idx = dst_idx = 0;

	while (idx < len)
	{
		c = ptr[idx];
		inc = byte_from_base32(c, &b32_convs[i], dst, &dst_idx);
		i = (i + inc) % (sizeof(b32_convs) / sizeof(b32_convs[0]));
		++idx;
	}

	if (ptr_len)
	{
		*ptr_len = dst_idx;
	}

	return dst;
}

int main(int argc __UNUSED, char *argv[] __UNUSED)
{
	char *buf;
	size_t len;

	buf = "bin  data  Desktop  Documents  Downloads  GNUstep  i  Mail  mbox  "
		"share  tmp  tmux.conf  usr  var";

	str_to_base32(buf, strlen((char *)buf));

	buf = base32_to_str("MJUW4IBAMRQXIYJAEBCGK43LORXXAIBAIRXWG5LNMVXHI4ZAEBCG653ONRXWCZDTEAQEOTSVON2G"
			"K4BAEBUSAICNMFUWYIBANVRG66BAEBZWQYLSMUQCA5DNOAQCA5DNOV4C4Y3PNZTCAIDVONZCAIDW"
			"MFZA====", 2 * 76 + 8, &len);
	fprintf(stdout, "\n\"%s\" (%zu)\n", buf, len);
	free(buf);

	return 0;
}
