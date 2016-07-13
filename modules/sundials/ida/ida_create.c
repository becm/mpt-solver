/*!
 * generic solver interface for IDA.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <ida/ida.h>

#include "sundials.h"

static void idaUnref(MPT_INTERFACE(unrefable) *gen)
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

static int idaReport(MPT_SOLVER(generic) *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return sundials_ida_report((MPT_SOLVER_STRUCT(ida) *) (gen+1), what, out, data);
}
static int idaStep(MPT_SOLVER(IVP) *sol, double *end)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (sol+1);
	int ret;
	if (!end) return sundials_ida_prepare(ida);
	ret = sundials_ida_step(ida, *end);
	*end = ida->t;
	return ret;
}
static void *idaFcn(MPT_SOLVER(IVP) *sol, int type)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (sol+1);
	MPT_SOLVER_IVP_STRUCT(functions) *fcn = (void *) (ida+1);
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): break;
	  case MPT_SOLVER_ENUM(DAE): return ida->ivp.pint ? 0 : &fcn->dae;
	  case MPT_SOLVER_ENUM(PDE): if (!ida->ivp.pint) return 0; fcn->dae.mas = 0; fcn->dae.jac = 0;
	  case MPT_SOLVER_ENUM(IVP): return fcn;
	  default: return 0;
	}
	fcn->dae.mas = 0;
	return &fcn->dae;
}
static double *idaState(MPT_SOLVER(IVP) *sol)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (sol+1);
#ifdef SUNDIALS_DOUBLE_PRECISION
	if (ida->sd.y) {
		return N_VGetArrayPointer(ida->sd.y);
	}
#endif
	return 0;
}

static const MPT_INTERFACE_VPTR(solver_ivp) idaCtl = {
	{ { { idaUnref }, idaRef, idaGet, idaSet }, idaReport },
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
extern MPT_SOLVER(IVP) *sundials_ida_create()
{
	MPT_SOLVER(IVP) *sol;
	MPT_SOLVER_IVP_STRUCT(functions) *uf;
	MPT_SOLVER_STRUCT(ida) *ida;
	
	if (!(sol = malloc(sizeof(*sol)+sizeof(*ida)+sizeof(*uf)))) {
		return 0;
	}
	ida = (MPT_SOLVER_STRUCT(ida) *) (sol+1);
	
	if (sundials_ida_init(ida) < 0) {
		free(ida);
		return 0;
	}
	uf = MPT_IVPFCN_INIT(ida + 1);
	uf->dae.param = ida;
	
	ida->ufcn = uf;
	IDASetUserData(ida->mem, ida);
	
	sol->_vptr = &idaCtl;
	
	return sol;
}
