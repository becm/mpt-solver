/*!
 * generic solver interface for CVode.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "cvode/cvode_impl.h"
#include "sundials.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode step operation
 * 
 * Execute CVode solver step to requested end.
 * Prepare solver data if needed.
 * 
 * Pass zero pointer user data to query current position.
 * 
 * Pass zero pointer end data to skip step execution.
 * 
 * \return prepare/step operation result
 */
extern int sundials_cvode_step_(MPT_SOLVER_STRUCT(cvode) *cv, double *u, double *end)
{
	int err;
	if (!u && end) {
		if (!cv->sd.y) return -1;
		*end = cv->ivp.last;
		return 0;
	}
	if (!cv->sd.y || !end) {
		if ((err = sundials_cvode_prepare(cv, u)) < 0) return err;
		if (!end) return err;
	}
	err = sundials_cvode_step(cv, u, *end);
	*end = cv->ivp.last;
	return err;
}

static int cVodeFini(MPT_INTERFACE(metatype) *gen)
{ sundials_cvode_fini((MPT_SOLVER_STRUCT(cvode) *) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *cVodeRef()
{ return 0; }

static int cVodeProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return sundials_cvode_property((MPT_SOLVER_STRUCT(cvode) *) (gen+1), pr, src); }

static void *cVodeCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int cVodeReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return sundials_cvode_report((MPT_SOLVER_STRUCT(cvode) *) (gen+1), what, out, data); }

static int cVodeStep(MPT_SOLVER_INTERFACE *gen, double *u, double *end, double *x)
{
	MPT_SOLVER_STRUCT(cvode) *cv = (void *) (gen+1);
	(void) x;
	return sundials_cvode_step_(cv, u, end);
}

static MPT_SOLVER_STRUCT(ivpfcn) *cVodeFcn(const MPT_SOLVER_INTERFACE *gen)
{
	MPT_SOLVER_STRUCT(cvode) *cv = (void *) (gen+1);
	return (void *) (cv+1);
}

static const MPT_INTERFACE_VPTR(Ivp) cVodeCtl = {
	{ { cVodeFini, cVodeRef, cVodeProperty, cVodeCast }, cVodeReport },
	cVodeStep,
	cVodeFcn
};

extern MPT_SOLVER_INTERFACE *sundials_cvode_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(ivpfcn) *uf;
	MPT_SOLVER_STRUCT(cvode) *cv;
	
	if (!(gen = malloc(sizeof(*gen)+sizeof(*cv)+sizeof(MPT_SOLVER_STRUCT(ivpfcn))))) {
		return 0;
	}
	cv = (MPT_SOLVER_STRUCT(cvode) *) (gen+1);
	sundials_cvode_init(cv);
	
	CVodeSetUserData(cv->mem, cv);
	
	cv->ufcn = uf = MPT_IVPFCN_INIT(cv + 1);
	uf->param = &cv->ivp;
	
	
	gen->_vptr = &cVodeCtl.gen;
	
	return gen;
}
