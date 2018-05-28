/*!
 * interface to dDASSL solver
 */

#ifndef _MPT_DASSL_H
#define _MPT_DASSL_H  @INTERFACE_VERSION@

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void dassl_fcn_t(double *, double *, double *, double *, int *, double *, int *);
typedef void dassl_jac_t(double *, double *, double *, double *, double *, double *, int *);

MPT_SOLVER_STRUCT(dassl)
{
#ifdef __cplusplus
public:
	~dassl();
	dassl();
#endif
	MPT_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	double  t;     /* reference time */
	
	MPT_SOLVER_TYPE(dvecpar) rtol, atol;  /* tolerances */
	
	struct iovec rwork, /* realtype work vector */
	             iwork; /* integer work vector */
	
	double *y,     /* state data */
	       *yp;    /* (initial) values of DY/DT */
	void *dmas;    /* temporary data for mass matrix */
	
	int    *ipar;  /* integer user parameter */
	double *rpar;  /* real user data */
	
	dassl_fcn_t *fcn;  /* calculate right-hand side */
	dassl_jac_t *jac;  /* user supplied jacobian */
	
	int info[15];  /* task parameters */
};

__MPT_EXTDECL_BEGIN

/* c definition for dassl fortran call */
extern void ddassl_(dassl_fcn_t *, int *, double *, double *, double *, double *, int *, double *, double *,
                    int *, double *, int *, int *, int *, double *, int *, dassl_jac_t *);

/* execute next step on supplied limex instance */
extern int mpt_dassl_step(MPT_SOLVER_STRUCT(dassl) *, double);

/* set dassl parameter */
extern int mpt_dassl_get(const MPT_SOLVER_STRUCT(dassl) *, MPT_STRUCT(property) *);
extern int mpt_dassl_set(MPT_SOLVER_STRUCT(dassl) *, const char *, const MPT_INTERFACE(metatype) *);

/* validate settings and working space for use */
extern int mpt_dassl_prepare(MPT_SOLVER_STRUCT(dassl) *);

/* initialize/clear dassl integrator descriptor */
extern void mpt_dassl_init(MPT_SOLVER_STRUCT(dassl) *);
extern void mpt_dassl_fini(MPT_SOLVER_STRUCT(dassl) *);
/* set wrapper for user functions */

/* set deviation values */
extern int mpt_dassl_ufcn(MPT_SOLVER_STRUCT(dassl) *, MPT_IVP_STRUCT(daefcn) *, int , const void *);

/* dassl status information */
extern int mpt_dassl_report(const MPT_SOLVER_STRUCT(dassl) *, int , MPT_TYPE(PropertyHandler) , void *);

/* handle for generic solver type */
#ifndef __cplusplus
extern MPT_INTERFACE(metatype) *mpt_dassl_create(void);
#endif

__MPT_EXTDECL_END
#ifdef __cplusplus
inline dassl::dassl()
{ mpt_dassl_init(this); }
inline dassl::~dassl()
{ mpt_dassl_fini(this); }

class Dassl : public IVP, dassl
{
public:
	inline Dassl() : _fcn(0)
	{ }
	~Dassl() __MPT_OVERRIDE
	{ }
	/* object operations */
	int property_get(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_dassl_get(this, pr);
	}
	int property_set(const char *pr, const metatype *src = 0) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_dassl_prepare(this);
		}
		return mpt_dassl_set(this, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return mpt_dassl_step(this, _t);
	}
	int report(int what, PropertyHandler out, void *opar) __MPT_OVERRIDE
	{
		if (!what && !out && !opar) {
			return DAE | PDE;
		}
		return mpt_dassl_report(this, what, out, opar);
	}
	int setFunctions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return mpt_dassl_ufcn(this, &_fcn, type, ptr);
	}
protected:
	struct daefcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* !defined(_MPT_DASSL_H) */

