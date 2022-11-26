#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "base32.h"

#define __CLEANUP(f) __attribute__((cleanup(f)))

static void cleanup_free(char **ptr)
{
	free(*ptr);
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
