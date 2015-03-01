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
	MPT_TYPE(dvecpar) rtol, atol;   /* tolerances */
	
	const MPT_SOLVER_STRUCT(ivpfcn) *ufcn; /* user functions */
	
	double h;  /* initial (suggested next) step size */
	
	double ropt[5];    /* double option array (min size 5) */
	int    iopt[30];   /* integer option array (min size 30) */
	
	struct iovec ipos, /* set to check coresponding position non-zero */
	              ys; /* derivates of solution at tstart */
	
	union {
		int raw[3];
		struct {
		int code,   /* limex error code */
		    dgxtrf, /* subroutine dg[be]trf error code */
		    civ;    /* indicate error while CIV computation */
		} st;
	} ifail;
	
	limex_fcn_t *fcn;   /* calculate right-hand side */
	limex_jac_t *jac;   /* user supplied jacobian */
};

__MPT_EXTDECL_BEGIN

/* c definition for limex fortran call */
extern void limex_(int *, limex_fcn_t *, limex_jac_t *, double *, double *, double *, double *,
                   double *, double *, double *, int *, double *, int *, int *);

/* execute step on supplied limex instance */
extern int mpt_limex_step(MPT_SOLVER_STRUCT(limex) *, double * , double );
extern int mpt_limex_step_(MPT_SOLVER_STRUCT(limex) *, double * , double *);

/* set limex parameter */
extern int mpt_limex_property(MPT_SOLVER_STRUCT(limex) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *);

/* validate settings and working space for use */
extern int mpt_limex_prepare(MPT_SOLVER_STRUCT(limex) *, int , int );

/* initialize/clear/reset limex integrator descriptor */
extern void mpt_limex_init(MPT_SOLVER_STRUCT(limex) *);
extern void mpt_limex_fini(MPT_SOLVER_STRUCT(limex) *);
extern void mpt_limex_reset(MPT_SOLVER_STRUCT(limex) *);

/* set wrapper for user functions */
extern int mpt_limex_ufcn(MPT_SOLVER_STRUCT(limex) *, const MPT_SOLVER_STRUCT(ivpfcn) *);

/* limex report information */
extern int mpt_limex_report(const MPT_SOLVER_STRUCT(limex) *, int , MPT_TYPE(PropertyHandler) , void *);

/* setup generic solver to use limex */
#ifndef __cplusplus
extern MPT_SOLVER_INTERFACE *mpt_limex_create(void);
#else
extern MPT_SOLVER_STRUCT(limex) *mpt_limex_global(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline limex::limex()
{ mpt_limex_init(this); }
inline limex::~limex()
{ mpt_limex_fini(this); }

class Limex : public Ivp
{
public:
	Limex() : _fcn(0)
	{ if ((_lx = mpt_limex_global())) _lx->ufcn = &_fcn; }
	virtual ~Limex()
	{ if (_lx) mpt_limex_fini(_lx); }
	Limex *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return _lx ? mpt_limex_property(_lx, pr, src) : -1; }
	int report(int what, PropertyHandler out, void *opar) const
	{ return _lx ? mpt_limex_report(_lx, what, out, opar) : -1; }
	int step(double *u, double *end, double *x)
	{ return _lx ? mpt_limex_step_(_lx, u, end) : 0; }
	operator ivpfcn *() const
	{ return _lx ? const_cast<ivpfcn *>(&_fcn) : 0; }
protected:
	ivpfcn _fcn;
	limex *_lx;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_LIMEX_H */

