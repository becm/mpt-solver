/*!
 * interface to MEBDFI solver
 */

#ifndef _MPT_MEBDFI_H
#define _MPT_MEBDFI_H  201502

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
	MPT_SOLVER_STRUCT(ivppar) ivp; /* inherit IVP parameter */
	
	MPT_TYPE(dvecpar) rtol, atol; /* tolerances */
	
	struct iovec rwork, /* mebdfi real work vector */
	             iwork, /* mebdfi integer work vector */
	             yp;    /* (initia)l values of DY/DT */
	
	int    *ipar;  /* integer user parameters */
	double *rpar;  /* real user parameters */
	
	char jnum, jbnd, /* numeric/banded jacobian */
	     type,       /* iteration type */
	     state;      /* solver state */
	
	short lout,      /* output channel */
	      maxder;    /* maximum order */
	int   mbnd[4];   /* banded matrix parameter */
	
	double h;  /* initial/previous step size */
	
	mebdfi_fcn_t *fcn;  /* calculate right-hand side */
	mebdfi_jac_t *jac;  /* user supplied jacobian */
	
	void *dmas;  /* temporal partial mass matrix */
};

__MPT_EXTDECL_BEGIN

/* c definition for mebdfi fortran call */
extern void mebdfi_(int *, double *, double *, double *, double *, double *, double *,
                    int *, int *, int *, int *,
                    double *, int *, int *, int *, int *,
                    int *, double *, double *, double *, int *,
                    mebdfi_jac_t *, mebdfi_fcn_t *, int *);

/* execute step on supplied mebdfi instance */
extern int mpt_mebdfi_step(MPT_SOLVER_STRUCT(mebdfi) *, double *, double );
extern int mpt_mebdfi_step_(MPT_SOLVER_STRUCT(mebdfi) *, double *, double *, const MPT_SOLVER_STRUCT(ivpfcn) *);

/* set mebdfi parameter */
extern int mpt_mebdfi_property(MPT_SOLVER_STRUCT(mebdfi) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *);

/* validate settings and working space for use */
extern int mpt_mebdfi_prepare(MPT_SOLVER_STRUCT(mebdfi) *, int , int );

/* initialize/free mebdfi integrator descriptor */
extern int mpt_mebdfi_init(MPT_SOLVER_STRUCT(mebdfi) *);
extern void mpt_mebdfi_fini(MPT_SOLVER_STRUCT(mebdfi) *);
/* set wrapper for user functions */
extern int mpt_mebdfi_ufcn(MPT_SOLVER_STRUCT(mebdfi) *, const MPT_SOLVER_STRUCT(ivpfcn) *);

/* mebdfi status information */
extern int mpt_mebdfi_report(const MPT_SOLVER_STRUCT(mebdfi) *, int , MPT_TYPE(PropertyHandler) , void *);

/* setup generic solver to use mebfi */
#ifndef __cplusplus
extern MPT_SOLVER_INTERFACE *mpt_mebdfi_create(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline mebdfi::mebdfi()
{ mpt_mebdfi_init(this); }
inline mebdfi::~mebdfi()
{ mpt_mebdfi_fini(this); }

class Mebdfi : public Ivp, mebdfi
{
    public:
	Mebdfi() : _fcn(0)
	{ }
	virtual ~Mebdfi()
	{ }
	Mebdfi *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return mpt_mebdfi_property(this, pr, src); }
	int report(int what, PropertyHandler out, void *opar) const
	{ return mpt_mebdfi_report(this, what, out, opar); }
	int step(double *u, double *end, double *x)
	{ return mpt_mebdfi_step_(this, u, end, &_fcn); }
	operator ivpfcn *() const
	{ return const_cast<ivpfcn *>(&_fcn); }
protected:
	ivpfcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* !defined(_MPT_MEBDFI_H) */

