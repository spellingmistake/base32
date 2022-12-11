#ifndef _BASE32_H_
#define _BASE32_H_

#include "baseXX.h"

char *str_to_base32(const char *ptr, size_t len);
char *base32_to_str(const char *ptr, size_t len, size_t *ptr_len,
		bool *printable);
char *str_to_base32_ext_hex(const char *ptr, size_t len);
char *base32_to_str_ext_hex(const char *ptr, size_t len, size_t *ptr_len,
		bool *printable);

#endif	/* #ifndef _BASE32_H_ */

