/*!
 * interface to LIMEX solver
 */

#ifndef _MPT_LIMEX_H
#define _MPT_LIMEX_H  201502

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void limex_fcn_t(int *, int *, double *, double *, double *, double *, int *, int *, int *);
typedef void limex_jac_t(int *, double *, double *, double *, double *, int *, int *, int *, int *, int *);

MPT_SOLVER_STRUCT(limex)
{
#ifdef __cplusplus
public:
	limex();
	~limex();
#endif
	MPT_SOLVER_STRUCT(ivppar) ivp;  /* inherit IVP parameter */
	
	double  t;    /* reference time */
	double *y,    /* values at current time */
	       *ys;   /* deviation at current time */
	
	int    *ipos; /* set to check coresponding position non-zero */
	
	const MPT_SOLVER_STRUCT(daefcn) *ufcn; /* user functions */
	
	MPT_SOLVER_TYPE(dvecpar) rtol, atol;   /* tolerances */
	
	double h;           /* initial (suggested next) step size */
	double ropt[5];     /* double option array (min size 5) */
	int    iopt[30];    /* integer option array (min size 30) */
	
	limex_fcn_t *fcn;   /* calculate right-hand side */
	limex_jac_t *jac;   /* user supplied jacobian */
	
	union {
		int raw[3];
		struct {
		int code,   /* limex error code */
		    dgxtrf, /* subroutine dg[be]trf error code */
		    civ;    /* indicate error while CIV computation */
		} st;
	} ifail;
};

__MPT_EXTDECL_BEGIN

/* c definition for limex fortran call */
extern void limex_(int *, limex_fcn_t *, limex_jac_t *, double *, double *, double *, double *,
                   double *, double *, double *, int *, double *, int *, int *);

/* execute step on supplied limex instance */
extern int mpt_limex_step(MPT_SOLVER_STRUCT(limex) *, double);

/* limex parameter */
extern int mpt_limex_get(const MPT_SOLVER_STRUCT(limex) *, MPT_STRUCT(property) *);
extern int mpt_limex_set(MPT_SOLVER_STRUCT(limex) *, const char *, MPT_INTERFACE(metatype) *);

/* validate settings and working space for use */
extern int mpt_limex_prepare(MPT_SOLVER_STRUCT(limex) *);

/* initialize/clear/reset limex integrator descriptor */
extern void mpt_limex_init(MPT_SOLVER_STRUCT(limex) *);
extern void mpt_limex_fini(MPT_SOLVER_STRUCT(limex) *);
extern void mpt_limex_reset(MPT_SOLVER_STRUCT(limex) *);

/* set wrapper for user functions */
extern int mpt_limex_ufcn(MPT_SOLVER_STRUCT(limex) *, const MPT_SOLVER_STRUCT(daefcn) *);

/* limex report information */
extern int mpt_limex_report(const MPT_SOLVER_STRUCT(limex) *, int , MPT_TYPE(PropertyHandler) , void *);

/* setup generic solver to use limex */
#ifndef __cplusplus
extern MPT_SOLVER(IVP) *mpt_limex_create(void);
#else
extern MPT_SOLVER_STRUCT(limex) *mpt_limex_global(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline limex::limex()
{ mpt_limex_init(this); }
inline limex::~limex()
{ mpt_limex_fini(this); }

class Limex : public IVP
{
public:
	Limex() : _fcn(0, 0)
	{
		if ((_lx = mpt_limex_global())) {
			_lx->ufcn = &_fcn;
			_fcn.param = &_lx->ivp;
		}
	}
	virtual ~Limex()
	{
		if (_lx) mpt_limex_fini(_lx);
	}
	void unref()
	{
		delete this;
	}
	int property(struct property *pr) const
	{
		return _lx ? mpt_limex_get(_lx, pr) : MPT_ERROR(BadOperation);
	}
	int setProperty(const char *pr, class metatype *src)
	{
		return _lx ? mpt_limex_set(_lx, pr, src) : MPT_ERROR(BadOperation);
	}
	int report(int what, PropertyHandler out, void *opar)
	{
		return _lx ? mpt_limex_report(_lx, what, out, opar) : MPT_ERROR(BadOperation);
	}
	int step(double *tend)
	{
		if (!_lx) {
			return BadArgument;
		}
		int err;
		if (!tend) {
			if (!_lx->fcn && (err = mpt_limex_ufcn(_lx, &_fcn)) < 0) {
				return err;
			}
			return mpt_limex_prepare(_lx);
		}
		err = mpt_limex_step(_lx, *tend);
		*tend = _lx->t;
		return err;
	}
	void *functions(int type)
	{
		if (!_lx) {
			return 0;
		}
		switch (type) {
		  case odefcn::Type: break;
		  case daefcn::Type: return _lx->ivp.pint ? 0 : &_fcn;
		  case pdefcn::Type: return _lx->ivp.pint ? &_fcn : 0;
		  default: return 0;
		}
		if (_lx->ivp.pint) {
			return 0;
		}
		_fcn.mas = 0;
		return &_fcn;
	}
protected:
	daefcn _fcn;
	limex *_lx;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_LIMEX_H */

