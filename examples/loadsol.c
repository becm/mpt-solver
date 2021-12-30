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
#include MPT_INCLUDE(types.h)
#include MPT_INCLUDE(config.h)
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

int main(int argc, char * const argv[])
{
	const char txt[] = "load: ";
	char buf[128];
	MPT_INTERFACE(logger) *log;
	MPT_INTERFACE(metatype) *mt = 0;
	mtrace();
	
	mpt_init(argc, argv);
	log = mpt_log_default();
	
	fputs(txt, stdout);
	while (fgets(buf, sizeof(buf), stdin)) {
		MPT_SOLVER(interface) *s;
		size_t len;
		
		if (!(len = strlen(buf))) {
			break;
		}
		buf[len - 1] = '\0';
		
		if ((s = mpt_solver_load(&mt, 0, buf, log))) {
			MPT_INTERFACE(object) *obj = 0;
			const char *n;
			if (MPT_metatype_convert(mt, MPT_ENUM(TypeObjectPtr), &obj) > 0
			    && obj
			    && (n = mpt_object_typename(obj))) {
				puts(n);
			}
			s->_vptr->report(s, -1, wrap_fw, stdout);
		}
		fputs(txt, stdout);
	}
	if (mt) {
		mt->_vptr->unref(mt);
	}
	
	return 0;
}
