#ifndef _BASE64_H_
#define _BASE64_H_

#include "baseXX.h"

char *str_to_base64(const char *ptr, size_t len);
char *base64_to_str(const char *ptr, size_t len, size_t *ptr_len,
	bool *printable);

#endif	/* #ifndef _BASE64_H_ */
