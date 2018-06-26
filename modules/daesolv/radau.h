/*!
 * interface to RADAU solver
 */

#ifndef _MPT_RADAU_H
#define _MPT_RADAU_H  @INTERFACE_VERSION@

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void radau_fcn_t(int *, double *, double *, double *, double *, int *);
typedef void radau_jac_t(int *, double *, double *, double *, int *, double *, int *);
typedef void radau_mas_t(int *, double *, int *, double *, int *);
typedef void radau_sol_t(int *, double *, double *, double *, double *, int *, int *, double *, int *, int *);

MPT_SOLVER_STRUCT(radau)
{
#ifdef __cplusplus
public:
	radau();
	~radau();
#endif
	MPT_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	double t;  /*reference time */
	double h;  /* initial/previous step size */
	
	union {
		int raw[7];
	    struct {
		int nfev,   /* f evaluations */
		    njac,   /* Jacobian evaluations */
		    nsteps, /* integration steps */
		    naccpt, /* accepted steps */
		    pad,    /* <empty> */
		    nlud;   /* LU decompositions */
	    } st;
	} count;
	
	int ijac;           /* use user supplied jacobian */
	
	double *y;          /* state data */
	double *dmas;       /* temporal data for partial mass matrix */
	
	MPT_SOLVER_TYPE(dvecpar) rtol, atol;  /* tolerances */
	
	struct iovec rwork, /* radau real work vector */
	             iwork; /* radau integer work vector */
	
	double *rpar;       /* double parameter */
	int *ipar;          /* integer parameter */
	
	int mljac, mujac;   /* lower/upper bandwidth of jacobian */
	int mlmas, mumas;   /* lower/upper bandwidth of mass matrix */
	
	/* radau versions of user functions (default to standard wrapper */
	radau_fcn_t *fcn;   /* calculate g(t,y,ys) */
	radau_jac_t *jac;   /* analythic jacobi matrix */
	radau_mas_t *mas;   /* explicit mass matrix */
	radau_sol_t *sol;   /* output of interim values */
};

__MPT_EXTDECL_BEGIN

/* c definition for radau fortran call */
extern void radau_(int *, radau_fcn_t *, double *, double *, double *, double *,
                   double *, double *, int *,
                   radau_jac_t *, int *, int *, int *,
                   radau_mas_t *, int *, int *, int *,
                   radau_sol_t *, int *, double *, int *, int *, int *,
                   double *, int *, int *);

/* execute next step on supplied radau instance */
extern int mpt_radau_step(MPT_SOLVER_STRUCT(radau) *, double);

/* set bacol functions/parameters */
extern int mpt_radau_get(const MPT_SOLVER_STRUCT(radau) *, MPT_STRUCT(property) *);
extern int mpt_radau_set(MPT_SOLVER_STRUCT(radau) *, const char *, const MPT_INTERFACE(metatype) *);

/* validate settings and working space for use */
extern int mpt_radau_prepare(MPT_SOLVER_STRUCT(radau) *);

/* initialize/clear radau integrator descriptor */
extern void mpt_radau_init(MPT_SOLVER_STRUCT(radau) *);
extern void mpt_radau_fini(MPT_SOLVER_STRUCT(radau) *);
/* set wrapper for user functions */
extern int mpt_radau_ufcn(MPT_SOLVER_STRUCT(radau) *, MPT_IVP_STRUCT(daefcn) *, int , const void *);

/* radau status information */
extern int mpt_radau_report(const MPT_SOLVER_STRUCT(radau) *, int , MPT_TYPE(property_handler) , void *);

/* setup generic solver to use radau */
#ifndef __cplusplus
extern MPT_INTERFACE(metatype) *mpt_radau_create(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline radau::radau()
{
	mpt_radau_init(this);
}
inline radau::~radau()
{
	mpt_radau_fini(this);
}
class Radau : public IVP, radau
{
public:
	inline Radau() : _fcn(0)
	{ }
	~Radau() __MPT_OVERRIDE
	{ }
	/* object operations */
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_radau_get(this, pr);
	}
	int set_property(const char *pr, const metatype *src = 0) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_radau_prepare(this);
		}
		return mpt_radau_set(this, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return mpt_radau_step(this, _t);
	}
	int report(int what, property_handler_t out, void *opar) __MPT_OVERRIDE
	{
		return mpt_radau_report(this, what, out, opar);
	}
	int set_functions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return mpt_radau_ufcn(this, &_fcn, type, ptr);
	}
protected:
	struct daefcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* !defined(_MPT_RADAU_H) */

