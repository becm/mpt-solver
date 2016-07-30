
#include <stdio.h>
#include <stdlib.h>

#include <mpt/message.h>
#include <mpt/client.h>

#include <mpt/solver.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
	struct mpt_libhandle lh = MPT_LIBHANDLE_INIT;
	struct mpt_solver_generic *sol;
	const char *str;
	
	mtrace();
	
	if ((str = mpt_library_assign(&lh, "sundials_cvode_create@libmpt_sundials.so.1", getenv("MPT_PREFIX_LIB")))) {
		fputs(str, stderr);
		return 1;
	}
	if ((sol = lh.create())) {
		puts(mpt_object_typename((void *) sol));
		sol->_vptr->obj.ref.unref((void *) sol);
	}
	mpt_library_close(&lh);
	return 0;
}
