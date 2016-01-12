/*!
 * generic solver interface for IDA.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <ida/ida.h>

#include "sundials.h"

static void idaUnref(MPT_INTERFACE(object) *gen)
{
	  sundials_ida_fini((MPT_SOLVER_STRUCT(ida) *) (gen+1));
	  free(gen);
}
static uintptr_t idaRef()
{
	return 0;
}
static int idaGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	return sundials_ida_get((MPT_SOLVER_STRUCT(ida) *) (gen+1), pr);
}
static int idaSet(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return sundials_ida_set((MPT_SOLVER_STRUCT(ida) *) (gen+1), pr, src);
}

static int idaReport(MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return sundials_ida_report((MPT_SOLVER_STRUCT(ida) *) (gen+1), what, out, data);
}
static int idaStep(MPT_SOLVER_INTERFACE *gen, double *end)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (gen+1);
	int ret;
	if (!end) return sundials_ida_prepare(ida);
	ret = sundials_ida_step(ida, *end);
	*end = ida->t;
	return ret;
}
static void *idaFcn(const MPT_SOLVER_INTERFACE *gen, int type)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (gen+1);
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): return ida->ivp.pint ? 0 : (ida+1);
	  case MPT_SOLVER_ENUM(DAE): return ida->ivp.pint ? 0 : (ida+1);
	  case MPT_SOLVER_ENUM(PDE): return ida->ivp.pint ? (ida+1) : 0;
	  default: return 0;
	}
}
static double *idaState(MPT_SOLVER_INTERFACE *gen)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (gen+1);
#ifdef SUNDIALS_DOUBLE_PRECISION
	if (ida->sd.y) {
		return N_VGetArrayPointer(ida->sd.y);
	}
#endif
	return 0;
}

static const MPT_INTERFACE_VPTR(Ivp) idaCtl = {
	{ { idaUnref, idaRef, idaGet, idaSet }, idaReport },
	idaStep,
	idaFcn,
	idaState
};

/*!
 * \ingroup mptSundialsIda
 * \brief create IDA solver
 * 
 * Create solver interface to Sundials IDA solver.
 * 
 * \return IDA solver instance
 */
extern MPT_SOLVER_INTERFACE *sundials_ida_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_TYPE(ivpfcn) *uf;
	MPT_SOLVER_STRUCT(ida) *ida;
	
	if (!(gen = malloc(sizeof(*gen)+sizeof(*ida)+sizeof(*uf)))) {
		return 0;
	}
	ida = (MPT_SOLVER_STRUCT(ida) *) (gen+1);
	
	if (sundials_ida_init(ida) < 0) {
		free(ida);
		return 0;
	}
	uf = MPT_IVPFCN_INIT(ida + 1);
	uf->dae.param = &ida->ivp;
	
	ida->ufcn = &uf->dae;
	IDASetUserData(ida->mem, ida);
	
	gen->_vptr = &idaCtl.gen;
	
	return gen;
}
