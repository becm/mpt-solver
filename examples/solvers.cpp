/*!
 * check solver interfaces
 */

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/solver/h>
#endif

#ifdef with_vode
# include MPT_INCLUDE(vode.h)
#endif

#ifdef with_daesolv
# include MPT_INCLUDE(dassl.h)
# include MPT_INCLUDE(mebdfi.h)
# include MPT_INCLUDE(radau.h)
#endif

#ifdef with_limex
# include MPT_INCLUDE(limex.h)
#endif

#ifdef with_sundials
# include MPT_INCLUDE(sundials.h)
#endif

#ifdef with_nlsolv
# include MPT_INCLUDE(minpack.h)
# include MPT_INCLUDE(portn2.h)
#endif

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

using namespace mpt;
using namespace mpt::solver;

static int val(void *, const property *pr)
{
	if (!pr || pr->name) {
		return BadArgument;
	}
	return 0;
}

static void info(generic &s)
{
	property pr("");
	const char *name = "";
	const char *type = "";
	const char *ver  = "";
	s.property(&pr);
	if (pr.name) name = pr.name;
	if (pr.desc) type = pr.desc;
	pr.name = "version";
	s.property(&pr);
	if (!pr.val.fmt && pr.val.ptr) {
		ver = reinterpret_cast<const char *>(pr.val.ptr);
	}
	int types = s.report(0, 0, 0);
	println("<%s> [0x%x] %s (%s)", name, types, ver, type);
	
	s.report(s.Values, val, 0);
	mpt_solver_info(&s, 0);
	mpt_solver_report(&s, 0);
	println(0);
}
static void pde(class IVP &s)
{
	// PDE equotations and intervals
	mpt_object_set(&s, "", "iu", 3, 10);
	// PDE time and start values
	mpt_object_set(&s, 0, "dddd", 0.5, 1.1, 1.2, 1.3);
	
	s.set(IVP::rside(0));
	s.prepare();
	s.step(1);
}
template <class T>
class NL : public Solver<T>
{
	int solve() __MPT_OVERRIDE
	{
		return T::solve();
	}
};

int main()
{
	mtrace();
#ifdef with_vode
	Vode v;  pde(v); info(v);
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
	CVode cv;   pde(cv);  info(cv);
	IDA ida;    pde(ida); info(ida);
#endif
#ifdef with_nlsolv
	NL<MinPack> mp; info(mp);
	PortN2 n2;  info(n2);
#endif
}
