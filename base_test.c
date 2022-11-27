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
		fprintf(stdout, "output contains unprintable characters\n", buf);
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

	return 0;
}
