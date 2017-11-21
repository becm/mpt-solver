/*!
 * check solver interfaces
 */

#include <iostream>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(solver.h)
#include MPT_INCLUDE(message.h)

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

int main()
{
    mpt::solver_output o;

    // demo with printable values
    o.setFlags('x', 0);
    o.setFlags('y', 1);
    o.setFlags('z', 2);

    std::cout << o.pass() << std::endl;
    
    // use values from mpt::msgbind::DataFlags
    o.setFlags(mpt::msgbind::All,  0);
    o.setFlags(mpt::msgbind::Init, 1);
    o.setFlags(mpt::msgbind::Fini, 2);
    
    for (int a : o.pass()) {
        std::cout << a << ' ';
    }
    std::cout << std::endl;
}
