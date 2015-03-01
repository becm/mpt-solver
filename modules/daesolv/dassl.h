/*!
 * interface to dDASSL solver
 */

#ifndef _MPT_DASSL_H
#define _MPT_DASSL_H  201502

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
	MPT_TYPE(dvecpar) rtol, atol;  /* tolerances */
	
	struct iovec rwork, /* dassl real work vector */
	             iwork, /* dassl integer work vector */
	             yp;    /* (initial) values of DY/DT */
	
	int    *ipar;  /* integer user parameter */
	double *rpar;  /* real user data */
	
	void *dmas;    /* temporal data for (partial) mas matrix */
	
	int info[15];  /* task parameters */
	
	dassl_fcn_t *fcn;  /* calculate right-hand side */
	dassl_jac_t *jac;  /* user supplied jacobian */
};

__MPT_EXTDECL_BEGIN

/* c definition for dassl fortran call */
extern void ddassl_(dassl_fcn_t *, int *, double *, double *, double *, double *, int *, double *, double *,
		    int *, double *, int *, int *, int *, double *, int *, dassl_jac_t *);

/* execute next step on supplied limex instance */
extern int mpt_dassl_step(MPT_SOLVER_STRUCT(dassl) *, double * , double );
extern int mpt_dassl_step_(MPT_SOLVER_STRUCT(dassl) *, double *, double *, const MPT_SOLVER_STRUCT(ivpfcn) *);

/* set dassl parameter */
extern int mpt_dassl_property(MPT_SOLVER_STRUCT(dassl) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *);

/* validate settings and working space for use */
extern int mpt_dassl_prepare(MPT_SOLVER_STRUCT(dassl) *, int , int );

/* initialize/clear dassl integrator descriptor */
extern int mpt_dassl_init(MPT_SOLVER_STRUCT(dassl) *);
extern void mpt_dassl_fini(MPT_SOLVER_STRUCT(dassl) *);
/* set wrapper for user functions */
extern int mpt_dassl_ufcn(MPT_SOLVER_STRUCT(dassl) *, const MPT_SOLVER_STRUCT(ivpfcn) *);

/* dassl status information */
extern int mpt_dassl_report(const MPT_SOLVER_STRUCT(dassl) *, int , MPT_TYPE(PropertyHandler) , void *);

/* handle for generic solver type */
#ifndef __cplusplus
extern MPT_SOLVER_INTERFACE *mpt_dassl_create(void);
#endif

__MPT_EXTDECL_END
#ifdef __cplusplus
inline dassl::dassl()
{ mpt_dassl_init(this); }
inline dassl::~dassl()
{ mpt_dassl_fini(this); }

class Dassl : public Ivp, dassl
{
public:
	Dassl() : _fcn(0)
	{ }
	virtual ~Dassl()
	{ }
	Dassl *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return mpt_dassl_property(this, pr, src); }
	int report(int what, PropertyHandler out, void *opar) const
	{ return mpt_dassl_report(this, what, out, opar); }
	int step(double *u, double *end, double *x)
	{ return mpt_dassl_step_(this, u, end, &_fcn); }
	operator ivpfcn *() const
	{ return const_cast<ivpfcn *>(&_fcn); }
protected:
	ivpfcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* !defined(_MPT_DASSL_H) */

