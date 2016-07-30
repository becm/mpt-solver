/*!
 * check solver interfaces
 */

#ifdef with_vode
# include <mpt/solver/vode.h>
#endif

#ifdef with_daesolv
# include <mpt/solver/dassl.h>
# include <mpt/solver/mebdfi.h>
# include <mpt/solver/radau.h>
#endif

#ifdef with_limex
# include <mpt/solver/limex.h>
#endif

#ifdef with_sundials
# include <mpt/solver/sundials.h>
#endif

#ifdef with_nlsolv
# include <mpt/solver/minpack.h>
# include <mpt/solver/portn2.h>
#endif

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;
using namespace mpt::solver;

static void info(generic &s)
{
	property pr("");
	s.property(&pr);
	const char *name = pr.name ? pr.name : "";
	const char *type = pr.desc ? pr.desc : "";
	
	pr.name = "version";
	s.property(&pr);
	const char *ver = pr.val.fmt || !pr.val.ptr ? "" : reinterpret_cast<const char *>(pr.val.ptr);
	
	println("<%s> %s (%s)", name, ver, type);
	
	mpt_solver_info(&s, 0);
	mpt_solver_report(&s, 0);
	println(0);
}
static void pde(class IVP &s)
{
	// PDE equotations and intervals
	mpt_object_set(&s, "", "ii", 3, 10);
	// PDE time and start values
	mpt_object_set(&s, 0, "dddd", 0.5, 1.1, 1.2, 1.3);
	
	pdefcn *fcn = s;
	fcn->rside = 0;
}

int main()
{
	mtrace();
#ifdef with_vode
	Vode v;     pde(v); info(v);
#endif
#ifdef with_daesolv
	Dassl d;    pde(d); info(d);
	Mebdfi m;   pde(m); info(m);
	Radau r;    pde(r); info(r);
#endif
#ifdef with_limex
	Limex l;    pde(l); info(l);
#endif
#ifdef with_sundials
	CVode cv;   pde(cv); info(cv);
	IDA ida;    pde(ida); info(ida);
#endif
#ifdef with_nlsolv
	MinPack mp; info(mp);
	PortN2 n2;  info(n2);
#endif
}
