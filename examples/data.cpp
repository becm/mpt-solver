
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
	mtrace();
	
	mpt::solver::clientdata cd;
	mpt_library_bind(&cd, "sundials_cvode_create@libmpt_sundials.so.1", 0, mpt::mpt_logger_default());
	cd.setSolver(0);
}
