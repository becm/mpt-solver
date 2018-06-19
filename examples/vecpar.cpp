/*!
 * check solver interfaces
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(solver.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt::solver;

int main()
{
	mtrace();
	
	dvecpar vd(7.6);
	std::cout << vd.value().fmt << ": " << vd << std::endl;
	
	vd.resize(8);
	std::cout << vd.value().fmt << ": " << vd << std::endl;
	
	vecpar<short> vi;
	vi[0] = 4;
	std::cout << vi.value().fmt << ": " << *vi.data().begin() << std::endl;
	vi.resize(8);
	std::cout << "vi[5] = " << vi[5] << std::endl;
	
	vi[3] = 9;
	std::cout << "vi = " << vi.data() << std::endl;
	
	return 0;
}
