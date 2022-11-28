#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "base32.h"
#include "base64.h"

#define __CLEANUP(f) __attribute__((cleanup(f)))

static void cleanup_free(char **ptr)
{
	free(*ptr);
}

typedef struct test_vectors_t test_vectors_t;

struct test_vectors_t {
	const char *input;
	const char *expected;
	bool printable;
};

/* test vectors from RFC 4648 */
static test_vectors_t tv_base64[] = {
	{ "", "", true },
	{ "f", "Zg==", true },
	{ "fo", "Zm8=", true },
	{ "foo", "Zm9v", true },
	{ "foob", "Zm9vYg==", true },
	{ "fooba", "Zm9vYmE=", true },
	{ "foobar", "Zm9vYmFy", true },
};

static test_vectors_t tv_base32[] = {
	{ "", "", true },
	{ "f", "MY======", true },
	{ "fo", "MZXQ====", true },
	{ "foo", "MZXW6===", true },
	{ "foob", "MZXW6YQ=", true },
	{ "fooba", "MZXW6YTB", true },
	{ "foobar", "MZXW6YTBOI======", true },
};

static void test_baseXX(test_vectors_t *tvs, size_t i, const char *base,
		bool verbose, char *(*str_to_baseXX)(const char *, size_t len),
		char *(*baseXX_to_str)(const char *, size_t, size_t *, bool *))
{
	size_t len;
	bool printable = true;
	char *bXX __CLEANUP(cleanup_free) = NULL,
		 *buf __CLEANUP(cleanup_free) = NULL;

	bXX = str_to_baseXX(tvs[i].input, strlen(tvs[i].input));
	buf = baseXX_to_str(bXX, strlen(bXX), &len, &printable);

	assert(tvs[i].printable == printable);
	assert(len == strlen(tvs[i].input));
	assert(!memcmp(buf, tvs[i].input, len));
	fprintf(stdout, "%s test vector %zu ok", base, i + 1);
	if (verbose)
	{
		fprintf(stdout, " (input '%s')\n", tvs[i].input);
	}
	else
	{
		fprintf(stdout, "\n");
	}
}

static void test_base64(void)
{
	size_t i;

	for (i = 0; i < sizeof(tv_base64) / sizeof(tv_base64[0]); ++i)
	{
		test_baseXX(tv_base64, i, __func__, false, str_to_base64, base64_to_str);
	}
}

static void test_base32(void)
{
	size_t i;

	for (i = 0; i < sizeof(tv_base32) / sizeof(tv_base32[0]); ++i)
	{
		test_baseXX(tv_base32, i, __func__, false, str_to_base32, base32_to_str);
	}
}

int main(int argc __UNUSED, char *argv[] __UNUSED)
{
	test_base64();
	test_base32();
	return 0;
}
