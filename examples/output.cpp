/*!
 * check solver interfaces
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(solver.h)
#include MPT_INCLUDE(values.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
	mpt::solver_output o;
	
	// demo with printable values
	o.set_flags('x', 0);
	o.set_flags('y', 1);
	o.set_flags('z', 2);
	
	std::cout << o.pass() << std::endl;
	
	// use values from mpt::msgbind::DataFlags
	o.set_flags(mpt::valsrc::All,  0);
	o.set_flags(mpt::valsrc::Init, 1);
	o.set_flags(mpt::valsrc::Fini, 2);
	
	for (int a : o.pass()) {
		std::cout << a << ' ';
	}
	std::cout << std::endl;
}
