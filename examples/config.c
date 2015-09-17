/*!
 * load solver from shared library,
 * print report
 */

#include <stdio.h>
#include <string.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

#include <mpt/convert.h>

#include <mpt/client.h>

#include <mpt/solver.h>

static int write_fd(void *out, const char *s, size_t len)
{
	return fwrite(s, len, 1, out);
}
static int wrap_fw(void *out, MPT_STRUCT(property) *pr)
{
	fwrite(pr->name, strlen(pr->name), 1, out);
	fwrite(" = ", 3, 1, out);
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
		if ((err = mpt_library_assign(&h, n))) {
			fputs(err, stderr); fputc('\n', stderr); fputs(txt, stdout); continue;
		}
		if ((s = h.create())) {
			puts(mpt_meta_typename((void *) s));
			s->_vptr->report(s, -1, wrap_fw, stdout);
			s->_vptr->_mt.unref((void *) s);
		} else {
			perror("failed getting solver descriptor");
		}
		mpt_library_close(&h);
		fputs(txt, stdout);
	}
	
	return 0;
}

