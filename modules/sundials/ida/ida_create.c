/*!
 * generic solver interface for IDA.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <ida/ida.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsIda
 * \brief IDA step operation
 * 
 * Execute IDA solver step to requested end.
 * Prepare solver data if needed.
 * 
 * Pass zero pointer user data to query current position.
 * 
 * Pass zero pointer end data to skip step execution.
 * 
 * \return IDA solver instance
 */
extern int sundials_ida_step_(MPT_SOLVER_STRUCT(ida) *ida, double *u, double *end)
{
	int err;
	if (!u && end) {
		if (!ida->sd.y) {
			return -1;
		}
		*end = ida->ivp.last;
		return 0;
	}
	if (!ida->sd.y || !end) {
		if ((err = sundials_ida_prepare(ida, u)) < 0 || !end) {
			return err;
		}
	}
	err = sundials_ida_step(ida, u, *end);
	*end = ida->ivp.last;
	return err;
}

static int idaFini(MPT_INTERFACE(metatype) *gen)
{ sundials_ida_fini((MPT_SOLVER_STRUCT(ida) *) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *idaRef()
{ return 0; }

static int idaProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return sundials_ida_property((MPT_SOLVER_STRUCT(ida) *) (gen+1), pr, src); }

static void *idaCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int idaReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return sundials_ida_report((MPT_SOLVER_STRUCT(ida) *) (gen+1), what, out, data); }

static int idaStep(MPT_SOLVER_INTERFACE *gen, double *u, double *end, double *x)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (gen+1);
	(void) x;
	return sundials_ida_step_(ida, u, end);
}

static MPT_SOLVER_STRUCT(ivpfcn) *idaFcn(const MPT_SOLVER_INTERFACE *gen)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (gen+1);
	return (void *) ida->ufcn;
}

static const MPT_INTERFACE_VPTR(Ivp) idaCtl = {
	{ { idaFini, idaRef, idaProperty, idaCast }, idaReport },
	idaStep,
	idaFcn
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
	MPT_SOLVER_STRUCT(ivpfcn) *uf;
	MPT_SOLVER_STRUCT(ida) *ida;
	
	if (!(gen = malloc(sizeof(*gen)+sizeof(*ida)+sizeof(MPT_SOLVER_STRUCT(ivpfcn))))) {
		return 0;
	}
	ida = (MPT_SOLVER_STRUCT(ida) *) (gen+1);
	if (sundials_ida_init(ida) < 0) {
		free(ida);
		return 0;
	}
	
	IDASetUserData(ida->mem, ida);
	
	ida->ufcn = uf = MPT_IVPFCN_INIT(ida + 1);
	uf->param = &ida->ivp;
	
	gen->_vptr = &idaCtl.gen;
	
	return gen;
}
