/*!
 * interface to dVODE solver
 */

#ifndef _MPT_VODE_H
#define _MPT_VODE_H  @INTERFACE_VERSION@

#include "../solver.h"

__MPT_SOLVER_BEGIN

typedef void vode_fcn_t(int *, double *, double *, double *, double *, int *);
typedef void vode_jac_t(int *, double *, double *, int *, int *, double *, int *, double *, int *);

MPT_SOLVER_STRUCT(vode)
{
#ifdef __cplusplus
public:
	~vode();
	vode();
#endif
	MPT_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	double *y;        /* state data */
	double  t;        /* independent variable */
	
	MPT_SOLVER_TYPE(dvecpar) rtol, atol;  /* tolerances */
	
	char meth, miter, /* method flags */
	     jsv,         /* save jacobian iteration */
	     iopt;        /* flag for optional input */
	
	short istate;     /* dvode state */
	short itask;      /* type of integration step */
	
	struct iovec rwork, /* vode real work vector */
	             iwork; /* vode integer work vector */
	
	double *rpar;     /* real parameters */
	int    *ipar;     /* integer parameters */
	
	vode_fcn_t *fcn;  /* calculate right-hand side */
	vode_jac_t *jac;  /* user supplied jacobian */
};

__MPT_EXTDECL_BEGIN

/* c definition for vode fortran call */
extern void dvode_(vode_fcn_t *, int *, double *, double *, double *, int *, double *, double *,
                   int *, int *, int *, double *, int *, int *, int *,
                   vode_jac_t *, int *, double *, int *);

/* execute next step on supplied vode instance */
extern int mpt_vode_step(MPT_SOLVER_STRUCT(vode) *, double);

/* set vode parameter */
extern int mpt_vode_get(const MPT_SOLVER_STRUCT(vode) *, MPT_STRUCT(property) *);
extern int mpt_vode_set(MPT_SOLVER_STRUCT(vode) *, const char *, MPT_INTERFACE(convertable) *);

/* validate settings and working space for use */
extern int mpt_vode_prepare(MPT_SOLVER_STRUCT(vode) *);

/* initialize/clear vode integrator descriptor */
extern void mpt_vode_init(MPT_SOLVER_STRUCT(vode) *);
extern void mpt_vode_fini(MPT_SOLVER_STRUCT(vode) *);
/* set wrapper for user functions */
extern int mpt_vode_ufcn(MPT_SOLVER_STRUCT(vode) *, MPT_IVP_STRUCT(odefcn) *, int , const void *);

/* vode status information */
extern int mpt_vode_report(const MPT_SOLVER_STRUCT(vode) *, int , MPT_TYPE(property_handler) , void *);

/* setup generic solver to use vode */
#ifndef __cplusplus
extern MPT_INTERFACE(metatype) *mpt_vode_create();
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline vode::vode()
{ mpt_vode_init(this); }
inline vode::~vode()
{ mpt_vode_fini(this); }

class Vode : public IVP, vode
{
public:
	inline Vode() : _fcn(0)
	{ }
	~Vode() __MPT_OVERRIDE
	{ }
	/* object operations */
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_vode_get(this, pr);
	}
	int set_property(const char *pr, convertable *src = 0) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_vode_prepare(this);
		}
		return mpt_vode_set(this, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return mpt_vode_step(this, _t);
	}
	int report(int what, property_handler_t out, void *opar) __MPT_OVERRIDE
	{
		return mpt_vode_report(this, what, out, opar);
	}
	int set_functions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return mpt_vode_ufcn(this, &_fcn, type, ptr);
	}
protected:
	struct odefcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_VODE_H */

