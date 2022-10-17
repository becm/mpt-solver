/*!
 * sys4.c: solver setup for BACOL example
 */

#include <stdlib.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# error: bad
# define mtrace()
#endif

#include "convert.h"

#include "bacol.h"

static ssize_t fw(void *ctx, const char *txt, size_t len)
{
	return fwrite(txt, 1, len, static_cast<FILE *>(ctx));
}

static int status(void *ctx, const mpt::value *val)
{
	mpt::mpt_print_value(val, fw, ctx);
	fputs(mpt::mpt_newline_string(0), static_cast<FILE *>(ctx));
	return 1;
}

int main()
{
	mtrace();
	
	mpt::solver::Bacol bac;
	double val[] = { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1 };
	mpt::span<const double> grid(val, 11);
	
	int neqs = 4;
	
	mpt_object_set(&bac, "", "iD", neqs, grid);
	
	bac.prepare();
	
	mpt_solver_info(&bac, 0);
	for (int i = 1; i <= 10; i++) {
		bac.step(i);
		mpt_solver_status(&bac, 0, status, stdout);
	}
	mpt_solver_report(&bac, mpt::mpt_log_default());
}
