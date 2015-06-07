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

int wrap_fw(void *out, MPT_STRUCT(property) *pr)
{
	char buf[64];
	
	fwrite(pr->name, strlen(pr->name), 1, out);
	fwrite(" =", 2, 1, out);
	
	if (!pr->val.fmt) {
		fputc(' ', out);
		fwrite(pr->val.ptr, strlen(pr->val.ptr), 1, out);
		fputc('\n', out);
		return 0;
	}
	
	while (*pr->val.fmt) {
		int len;
		fputc(' ', out);
		if ((len = mpt_data_print(buf, sizeof(buf), *(pr->val.fmt++), &pr->val.ptr)) < 0) {
			return -1;
		}
		fwrite(buf, len, 1, out);
	}
	fputc('\n', out);
	
	return 0;
}

int main(int argc, char *argv[])
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

