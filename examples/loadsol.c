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

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(convert.h)
#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(parse.h)
#include MPT_INCLUDE(meta.h)

#include MPT_INCLUDE(solver.h)

static ssize_t write_fd(void *out, const char *s, size_t len)
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
	MPT_STRUCT(proxy) p = MPT_PROXY_INIT;
	MPT_INTERFACE(logger) *log;
	mtrace();
	
	mpt_config_load(getenv("MPT_PREFIX"), 0, 0);
	log = mpt_log_default();
	
	fputs(txt, stdout);
	while (fgets(buf, sizeof(buf), stdin)) {
		MPT_SOLVER(generic) *s;
		size_t len;
		if (!(len = strlen(buf))) break;
		buf[len-1] = '\0';
		
		if ((s = mpt_solver_load(&p, 0, buf, log))) {
			const char *n;
			if ((n = mpt_object_typename((void *) s))) {
				puts(n);
			}
			s->_vptr->report(s, -1, wrap_fw, stdout);
		}
		fputs(txt, stdout);
	}
	if (p._ref) {
		p._ref->_vptr->ref.unref((void *) p._ref);
		p._ref = 0;
	}
	
	return 0;
}

