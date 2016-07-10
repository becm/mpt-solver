/*!
 * check solver interfaces
 */

#include <stdio.h>

#include <mpt/solver/vode.h>

#include <mpt/solver/dassl.h>
#include <mpt/solver/mebdfi.h>
#include <mpt/solver/radau.h>

#include <mpt/solver/limex.h>

#include <mpt/solver/bacol.h>

#include <mpt/solver/sundials.h>

#include <mpt/solver/minpack.h>
#include <mpt/solver/portn2.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;
using namespace mpt::solver;

static void info(generic &s)
{
	const char *name, *ver;
	property pr("");
	
	s.property(&pr);
	name = pr.name;
	
	pr.name = "version";
	s.property(&pr);
	ver = pr.val.fmt ? 0 : (char *) pr.val.ptr;
	
	printf("<%s> %s\n", name ? name : "", ver ? ver : "");
	
	mpt_solver_info(&s, 0);
	mpt_solver_report(&s, 0);
	puts("");
}
static void pde(generic &s)
{
	// PDE equotations and intervals
	mpt_object_set(&s, "", "ii", 3, 10);
	// PDE time and start values
	mpt_object_set(&s, 0, "dddd", 0.5, 1.1, 1.2, 1.3);
}

int main()
{
	mtrace();
	
	Vode v;     pde(v); info(v);
	
	Dassl d;    pde(d); info(d);
	Mebdfi m;   pde(m); info(m);
	Radau r;    pde(r); info(r);
	
	Limex l;    pde(l); info(l);
	
	CVode cv;   pde(cv); info(cv);
	IDA ida;    pde(ida); info(ida);
	
	MinPack mp; info(mp);
	PortN2 n2;  info(n2);
	
	dvecpar vd(7.6);
	std::cout << vd.value().fmt << ": " << vd.data() << std::endl;
	
	vd.resize(8);
	std::cout << vd.value().fmt << ": " << vd.data() << std::endl;
	
	vecpar<short> vi;
	vi[0] = 4;
	std::cout << vi.value().fmt << ": " << *vi.data().base() << std::endl;
	vi.resize(8);
	std::cout << "vi[5] = " << vi[5] << std::endl;
	
	vi[3] = 9;
	std::cout << "vi = " << vi.data() << std::endl;
	
	return 0;
}