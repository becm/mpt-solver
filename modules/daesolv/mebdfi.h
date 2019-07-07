/*!
 * interface to MEBDFI solver
 */

#ifndef _MPT_MEBDFI_H
#define _MPT_MEBDFI_H  @INTERFACE_VERSION@

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void mebdfi_fcn_t(int *, double *, double *, double *, double *, int *, double *, int *);
typedef void mebdfi_jac_t(double *, double *, double *, int *, double *, int *, double *, int *, double *, int *);

MPT_SOLVER_STRUCT(mebdfi)
{
#ifdef __cplusplus
public:
	mebdfi();
	~mebdfi();
#endif
	MPT_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	double t;        /* reference time */
	
	double *y,       /* state data */
	       *yp,      /* (initia)l values of DY/DT */
	       *dmas;    /* temporal partial mass matrix */
	
	MPT_SOLVER_TYPE(dvecpar) rtol, atol; /* tolerances */
	
	struct iovec rwork, iwork; /* mebdfi work vectors */
	
	int    *ipar;    /* integer user parameters */
	double *rpar;    /* real user parameters */
	
	char jnum, jbnd, /* numeric/banded jacobian */
	     type,       /* iteration type */
	     state;      /* solver state */
	
	short lout,      /* output channel */
	      maxder;    /* maximum order */
	int   mbnd[4];   /* banded matrix parameter */
	
	double h;        /* initial/previous step size */
	
	mebdfi_fcn_t *fcn;  /* calculate right-hand side */
	mebdfi_jac_t *jac;  /* user supplied jacobian */
	
};

__MPT_EXTDECL_BEGIN

/* c definition for mebdfi fortran call */
extern void mebdfi_(int *, double *, double *, double *, double *, double *, double *,
                    int *, int *, int *, int *,
                    double *, int *, int *, int *, int *,
                    int *, double *, double *, double *, int *,
                    mebdfi_jac_t *, mebdfi_fcn_t *, int *);

/* execute step on supplied mebdfi instance */
extern int mpt_mebdfi_step(MPT_SOLVER_STRUCT(mebdfi) *, double);

/* set mebdfi parameter */
extern int mpt_mebdfi_get(const MPT_SOLVER_STRUCT(mebdfi) *, MPT_STRUCT(property) *);
extern int mpt_mebdfi_set(MPT_SOLVER_STRUCT(mebdfi) *, const char *, MPT_INTERFACE(convertable) *);

/* validate settings and working space for use */
extern int mpt_mebdfi_prepare(MPT_SOLVER_STRUCT(mebdfi) *);

/* initialize/free mebdfi integrator descriptor */
extern void mpt_mebdfi_init(MPT_SOLVER_STRUCT(mebdfi) *);
extern void mpt_mebdfi_fini(MPT_SOLVER_STRUCT(mebdfi) *);
/* set wrapper for user functions */
extern int mpt_mebdfi_ufcn(MPT_SOLVER_STRUCT(mebdfi) *, MPT_IVP_STRUCT(daefcn) *, int , const void *);

/* mebdfi status information */
extern int mpt_mebdfi_report(const MPT_SOLVER_STRUCT(mebdfi) *, int , MPT_TYPE(property_handler) , void *);

/* setup generic solver to use mebfi */
#ifndef __cplusplus
extern MPT_INTERFACE(metatype) *mpt_mebdfi_create(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline mebdfi::mebdfi()
{ mpt_mebdfi_init(this); }
inline mebdfi::~mebdfi()
{ mpt_mebdfi_fini(this); }

class Mebdfi : public IVP, mebdfi
{
public:
	inline Mebdfi() : _fcn(0)
	{ }
	~Mebdfi() __MPT_OVERRIDE
	{ }
	/* object operations */
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_mebdfi_get(this, pr);
	}
	int set_property(const char *pr, convertable *src) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_mebdfi_prepare(this);
		}
		return mpt_mebdfi_set(this, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return mpt_mebdfi_step(this, _t);
	}
	int report(int what, property_handler_t out, void *opar) __MPT_OVERRIDE
	{
		return mpt_mebdfi_report(this, what, out, opar);
	}
	int set_functions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return mpt_mebdfi_ufcn(this, &_fcn, type, ptr);
	}
protected:
	struct daefcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* !defined(_MPT_MEBDFI_H) */

