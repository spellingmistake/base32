#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "base32.h"
#include "base64.h"

#define __CLEANUP(f) __attribute__((cleanup(f)))

static void cleanup_free(char **ptr)
{
	free(*ptr);
}

static void test_base32(const char *input)
{
	size_t len;
	bool printable = true;
	char *b32 __CLEANUP(cleanup_free) = NULL,
		 *buf __CLEANUP(cleanup_free) = NULL;

	b32 = str_to_base32(input, strlen(input));
	fprintf(stdout, "%s\n", b32);
	buf = base32_to_str(b32, strlen(b32), &len, &printable);
	if (printable)
	{
		fprintf(stdout, "%s\n", buf);
	}
	else
	{
		fprintf(stdout, "output contains unprintable characters\n");
	}

	assert(len == strlen(input));
	assert(!memcmp(buf, input, len));
}

static void test_base64(const char *input)
{
	size_t len;
	bool printable = true;
	char *b64 __CLEANUP(cleanup_free) = NULL,
		 *buf __CLEANUP(cleanup_free) = NULL;

	b64 = str_to_base64(input, strlen(input));
	fprintf(stdout, "%s\n", b64);
	buf = base64_to_str(b64, strlen(b64), &len, &printable);
	if (printable)
	{
		fprintf(stdout, "%s\n", buf);
	}
	else
	{
		fprintf(stdout, "output contains unprintable characters\n");
	}

	assert(len == strlen(input));
	assert(!memcmp(buf, input, len));
}

int main(int argc __UNUSED, char *argv[] __UNUSED)
{
	char *input;

	input = "bin  data  Desktop  Documents  Downloads  GNUstep  i  Mail  mbox  "
		"share  tmp  tmux.conf  usr  var";

	test_base32(input);
	test_base32("12");
	test_base32("123");
	test_base64(input);
	test_base64("12");
	test_base64("123");

	return 0;
}
