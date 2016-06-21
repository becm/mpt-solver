/*!
 * enshure vector size requirements
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/uio.h>

#include "solver.h"

extern void *mpt_vecpar_alloc(struct iovec *vec, size_t need, size_t esize)
{
	void *ptr;
	
	if (!need) {
		if ((ptr = vec->iov_base)) free(ptr);
		vec->iov_base = 0;
		vec->iov_len  = 0;
		return 0;
	}
	if (!esize || SIZE_MAX/esize < need) {
		errno = ERANGE;
		return 0;
	}
	need *= esize;
	
	if (!(ptr = vec->iov_base) || vec->iov_len < need) {
		if (!(ptr = realloc(ptr, need))) {
			return 0;
		}
		if (need > vec->iov_len) {
			memset(((uint8_t *) ptr) + vec->iov_len, 0, need - vec->iov_len);
		}
		vec->iov_base = ptr;
		vec->iov_len  = need;
	}
	return ptr;
}
