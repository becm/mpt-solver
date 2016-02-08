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
	MPT_SOLVER_STRUCT(ivppar) ivp; /* inherit IVP parameter */
	
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
extern int mpt_dassl_set(MPT_SOLVER_STRUCT(dassl) *, const char *, MPT_INTERFACE(metatype) *);

/* validate settings and working space for use */
extern int mpt_dassl_prepare(MPT_SOLVER_STRUCT(dassl) *);

/* initialize/clear dassl integrator descriptor */
extern void mpt_dassl_init(MPT_SOLVER_STRUCT(dassl) *);
extern void mpt_dassl_fini(MPT_SOLVER_STRUCT(dassl) *);
/* set wrapper for user functions */
extern int mpt_dassl_ufcn(MPT_SOLVER_STRUCT(dassl) *, const MPT_SOLVER_STRUCT(ivpfcn) *);

/* dassl status information */
extern int mpt_dassl_report(const MPT_SOLVER_STRUCT(dassl) *, int , MPT_TYPE(PropertyHandler) , void *);

/* handle for generic solver type */
#ifndef __cplusplus
extern MPT_SOLVER(IVP) *mpt_dassl_create(void);
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
	Dassl()
	{
		_fcn.dae.param = &ivp;
	}
	virtual ~Dassl()
	{ }
	void unref()
	{
		delete this;
	}
	int property(struct property *pr) const
	{
		return mpt_dassl_get(this, pr);
	}
	int setProperty(const char *pr, metatype *src = 0)
	{
		return mpt_dassl_set(this, pr, src);
	}
	int report(int what, PropertyHandler out, void *opar)
	{
		return mpt_dassl_report(this, what, out, opar);
	}
	int step(double *tend)
	{
		int ret;
		if (!tend) {
			if (!fcn && (ret = mpt_dassl_ufcn(this, &_fcn)) < 0) {
				return ret;
			}
			return mpt_dassl_prepare(this);
		}
		ret = mpt_dassl_step(this, *tend);
		*tend = t;
		return ret;
	}
	void *functions(int type)
	{
		return _fcn.functions(type, ivp);
	}
	double *initstate()
	{
		return y;
	}
protected:
	ivpfcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* !defined(_MPT_DASSL_H) */

