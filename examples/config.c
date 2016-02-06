/*!
 * load solver from shared library,
 * print report
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#include <mpt/convert.h>
#include <mpt/client.h>
#include <mpt/parse.h>

#include <mpt/solver.h>

static int write_fd(void *out, const char *s, size_t len)
{
	return fwrite(s, len, 1, out);
}
static int wrap_fw(void *out, const MPT_STRUCT(property) *pr)
{
	if (!pr) {
		return 0;
	}
	if (pr->name) {
		fwrite(pr->name, strlen(pr->name), 1, out);
		fwrite(" = ", 3, 1, out);
	}
	mpt_tostring(&pr->val, write_fd, out);
	fputc('\n', out);
	return 0;
}

int main(void)
{
	const char txt[] = "load: ";
	char buf[128];
	MPT_STRUCT(libhandle) h = { 0, 0 };
	const char *err;
	
	mtrace();
	
	mpt_config_load(getenv("MPT_PREFIX"), 0, 0);
	
	fputs(txt, stdout);
	while (fgets(buf, sizeof(buf), stdin)) {
		MPT_SOLVER_INTERFACE *s;
		const char *n;
		size_t len;
		if (!(len = strlen(buf))) break;
		buf[len-1] = '\0';
		
		if (!(n = mpt_solver_alias(buf))) {
			n = buf;
		}
		if ((err = mpt_library_assign(&h, n, getenv("MPT_PREFIX_LIB")))) {
			fputs(err, stderr);
			fputc('\n', stderr);
			fputs(txt, stdout);
			continue;
		}
		if ((s = h.create())) {
			if ((n = mpt_object_typename((void *) s))) puts(n);
			s->_vptr->report(s, -1, wrap_fw, stdout);
			s->_vptr->obj.unref((void *) s);
		} else {
			perror("failed getting solver descriptor");
		}
		mpt_library_close(&h);
		fputs(txt, stdout);
	}
	
	return 0;
}

