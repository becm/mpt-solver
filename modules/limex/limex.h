/*!
 * interface to LIMEX solver
 */

#ifndef _MPT_LIMEX_H
#define _MPT_LIMEX_H  @INTERFACE_VERSION@

#include "../solver.h"

__MPT_SOLVER_BEGIN

typedef void limex_fcn_t(int *, int *, double *, double *, double *, double *, int *, int *, int *);
typedef void limex_jac_t(int *, double *, double *, double *, double *, int *, int *, int *, int *, int *);

MPT_SOLVER_STRUCT(limex_param) {
	int neqs, _pad;
	const MPT_SOLVER_STRUCT(limex) *lx;
	const MPT_IVP_STRUCT(daefcn) *ufcn;
};
MPT_SOLVER_STRUCT(limex)
{
#ifdef __cplusplus
public:
	limex();
	~limex();
#endif
	MPT_IVP_STRUCT(parameters) ivp;  /* inherit IVP parameter */
	
	double  t,    /* reference time */
	        h;    /* initial (suggested next) step size */
	
	double *y,    /* values at current time */
	       *ys;   /* deviation at current time */
	
	int    *ipos; /* set to check coresponding position non-zero */
	
	const MPT_IVP_STRUCT(daefcn) *ufcn;  /* user functions */
	
	MPT_SOLVER_TYPE(dvecpar) rtol, atol; /* tolerances */
	
	limex_fcn_t *fcn;   /* calculate right-hand side */
	limex_jac_t *jac;   /* user supplied jacobian */
	
	int    iopt[30];    /* integer option array (min size 30) */
	double ropt[5];     /* double option array (min size 5) */
	
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
extern int mpt_limex_set(MPT_SOLVER_STRUCT(limex) *, const char *, MPT_INTERFACE(convertable) *);

/* validate settings and working space for use */
extern int mpt_limex_prepare(MPT_SOLVER_STRUCT(limex) *);

/* initialize/clear/reset limex integrator descriptor */
extern void mpt_limex_init(MPT_SOLVER_STRUCT(limex) *);
extern void mpt_limex_fini(MPT_SOLVER_STRUCT(limex) *);

/* set wrapper for user functions */
extern int mpt_limex_ufcn(MPT_SOLVER_STRUCT(limex) *, MPT_IVP_STRUCT(daefcn) *, int , const void *);

/* limex report information */
extern int mpt_limex_report(const MPT_SOLVER_STRUCT(limex) *, int , MPT_TYPE(property_handler) , void *);

/* setup generic solver to use limex */
#ifndef __cplusplus
extern MPT_INTERFACE(metatype) *mpt_limex_create(void);
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
	Limex() : _fcn(0), _t(0)
	{
		if ((_lx = mpt_limex_global())) {
			_lx->ufcn = &_fcn;
		}
	}
	~Limex() __MPT_OVERRIDE
	{
		if (_lx) mpt_limex_fini(_lx);
	}
	/* object operations */
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return _lx ? mpt_limex_get(_lx, pr) : BadOperation;
	}
	int set_property(const char *pr, convertable *src) __MPT_OVERRIDE
	{
		if (!_lx) {
			return BadOperation;
		}
		if (!pr && !src) {
			int ret = mpt_limex_prepare(_lx);
			if (ret >= 0) _t = _lx->t;
			return ret;
		}
		if (_is_time_property(pr)) {
			return mpt_solver_module_nextval(&_t, _lx->t, src);
		}
		return mpt_limex_set(_lx, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return _lx ? mpt_limex_step(_lx, _t) : BadOperation;
	}
	int report(int what, property_handler_t out, void *opar) __MPT_OVERRIDE
	{
		return _lx ? mpt_limex_report(_lx, what, out, opar) : BadOperation;
	}
	int set_functions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return _lx ? mpt_limex_ufcn(_lx, &_fcn, type, ptr) : 0;
	}
protected:
	struct daefcn _fcn;
	limex *_lx;
	double _t;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_LIMEX_H */

