/*!
 * interface to RADAU solver
 */

#ifndef _MPT_RADAU_H
#define _MPT_RADAU_H	201405

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
	MPT_SOLVER_STRUCT(ivppar) ivp; /* inherit IVP parameter */
	
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
extern int mpt_radau_set(MPT_SOLVER_STRUCT(radau) *, const char *, MPT_INTERFACE(metatype) *);

/* validate settings and working space for use */
extern int mpt_radau_prepare(MPT_SOLVER_STRUCT(radau) *);

/* initialize/clear radau integrator descriptor */
extern void mpt_radau_init(MPT_SOLVER_STRUCT(radau) *);
extern void mpt_radau_fini(MPT_SOLVER_STRUCT(radau) *);
/* set wrapper for user functions */
extern int mpt_radau_ufcn(MPT_SOLVER_STRUCT(radau) *, const MPT_SOLVER_STRUCT(daefcn) *);

/* radau status information */
extern int mpt_radau_report(const MPT_SOLVER_STRUCT(radau) *, int , MPT_TYPE(PropertyHandler) , void *);

/* setup generic solver to use radau */
#ifndef __cplusplus
extern MPT_SOLVER_INTERFACE *mpt_radau_create(void);
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
class Radau : public Ivp, radau
{
public:
	Radau() : _fcn(0, 0)
	{ }
	virtual ~Radau()
	{ }
	void unref()
	{
		delete this;
	}
	int property(struct property *pr) const
	{
		return mpt_radau_get(this, pr);
	}
	int setProperty(const char *pr, metatype *src = 0)
	{
		return mpt_radau_set(this, pr, src);
	}
	int report(int what, PropertyHandler out, void *opar)
	{
		return mpt_radau_report(this, what, out, opar);
	}
	int step(double *tend)
	{
		int ret;
		if (!tend) {
			if (!fcn && (ret = mpt_radau_ufcn(this, &_fcn)) < 0) {
				return ret;
			}
			return mpt_radau_prepare(this);
		}
		ret = mpt_radau_step(this, *tend);
		*tend = t;
		return ret;
	}
	void *functions(int type)
	{
		switch (type) {
		  case odefcn::Type: break;
		  case daefcn::Type: return ivp.pint ? 0 : &_fcn;
		  case pdefcn::Type: return ivp.pint ? &_fcn : 0;
		  default: return 0;
		}
		if (ivp.pint) {
			return 0;
		}
		_fcn.mas = 0;
		return &_fcn;
	}
	double *initstate()
	{
		return y;
	}
protected:
	daefcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* !defined(_MPT_RADAU_H) */

