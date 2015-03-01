/*!
 * interface to MINPACK solver family
 */

#ifndef _MPT_MINPACK_H
#define _MPT_MINPACK_H	201405

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void hybrd_fcn_t(int *, double *, double *, int *);
typedef void hybrj_fcn_t(int *, double *, double *, double *, int *, int *);

typedef void lmdif_fcn_t(int *, int *, double *, double *, int *);
typedef void lmder_fcn_t(int *, int *, double *, double *, double *, int *, int *);
typedef void lmstr_fcn_t(int *, int *, double *, double *, double *, int *);

enum MPT_ENUM(MinpackSolver) {
	MPT_ENUM(MinpackHybrd) = 1,
	MPT_ENUM(MinpackHybrj),
	MPT_ENUM(MinpackLmDif),
	MPT_ENUM(MinpackLmDer),
	MPT_ENUM(MinpackLmStr)
};

MPT_SOLVER_STRUCT(minpack)
{
#ifdef __cplusplus
public:
	minpack();
	~minpack();
#endif
	MPT_SOLVER_STRUCT(nlspar) nls; /* inherit nonlinear system parameter */
	const MPT_SOLVER_STRUCT(nlsfcn) *ufcn;
	
	struct iovec diag,  /* scale factors for the variables */
	             work;  /* unified work vektor */
	
	char solv;    /* solver selection */
	char mode;    /* mode of operation */
	char nprint;  /* print iteration */
	char info;    /* status information */
		
	int mu, ml;   /* hybrd: upper/lower bandwith */
	
	int maxfev;   /* max. function evaluations */
	
	int nfev, njev;  /* function/jacobian evaluations */
	
	double xtol;  /* desired relative error in solution */
	double ftol;  /* desired maximal residual */
	double gtol;  /* desired orthogonality (fcn/jaccol) */
	
	double factor;  /* initial step bound */
	double epsfcn;  /* initial forward-difference step length */
	
	union {
		hybrd_fcn_t *hd;
		hybrj_fcn_t *hj;
		lmdif_fcn_t *dif;
		lmder_fcn_t *der;
		lmstr_fcn_t *str;
	} fcn;  /* residual calculation */
};

__MPT_EXTDECL_BEGIN

/* simple C-Wrapper for hybr[dj]1_-call */
extern int hybrd1(hybrd_fcn_t *fcn, int n, double *x, double *fvec,
                  double tol, double *wa, int lwa);
extern int hybrj1(hybrj_fcn_t *fcn, int n, double *x, double *fvec,
                  double *fjac, int ldfjac, double tol, double *wa, int lwa);

/* simple C-Wrapper for lm{dif,der,str}1_-call */
extern int lmdif1(lmdif_fcn_t *fcn, int m, int n, double *x, double *fvec,
                  double tol, int *iwa, double *wa, int lwa);
extern int lmder1(lmder_fcn_t *fcn, int m, int n, double *x, double *fvec,
                  double *fjac, int ldfjac, double tol, int *iwa, double *wa, int lwa);
extern int lmstr1(lmstr_fcn_t *fcn, int m, int n, double *x, double *fvec,
                  double *fjac, int ldfjac, double tol, int *ipvt, double *wa, int lwa);

/* set hybrd parameter */
extern int mpt_minpack_property(MPT_SOLVER_STRUCT(minpack) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *src);

/* call minpack solver routine */
extern int mpt_minpack_step(MPT_SOLVER_STRUCT(minpack) *mpack, double *x, double *f);
extern int mpt_minpack_step_(MPT_SOLVER_STRUCT(minpack) *mpack, double *x, double *f);

/* initialize/terminate structured data */
extern void mpt_minpack_init(MPT_SOLVER_STRUCT(minpack) *mpack);
extern void mpt_minpack_fini(MPT_SOLVER_STRUCT(minpack) *mpack);

/* set wrapper for user functions */
extern int mpt_minpack_ufcn_hybrid(MPT_SOLVER_STRUCT(minpack) *mpack);
extern int mpt_minpack_ufcn_lmderv(MPT_SOLVER_STRUCT(minpack) *mpack);

/* set wrapper for user functions */
extern int mpt_minpack_prepare(MPT_SOLVER_STRUCT(minpack) *mpack, int n, int m);

/* minpack status information */
extern int mpt_minpack_report(const MPT_SOLVER_STRUCT(minpack) *mpack, int what, MPT_TYPE(PropertyHandler) out, void *data);

/* assign minpack solver to interface */
#ifndef __cplusplus
extern MPT_SOLVER_INTERFACE *mpt_minpack_create(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline minpack::minpack()
{ mpt_minpack_init(this); }
inline minpack::~minpack()
{ mpt_minpack_fini(this); }

class MinPack : public Nls, minpack
{
    public:
	MinPack() : _fcn(0)
	{ ufcn = &_fcn; }
	virtual ~MinPack()
	{ }
	MinPack *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return mpt_minpack_property(this, pr, src); }
	int report(int what, PropertyHandler out, void *data) const
	{ return mpt_minpack_report(this, what, out, data); }
	int step(double *x, double *f)
	{ return mpt_minpack_step_(this, x, f); }
	
	operator const nlspar *() const
	{ return static_cast<const nlspar *>(&this->nls); }
	
	operator nlsfcn *() const
	{ return const_cast<nlsfcn *>(&_fcn); }
	
    protected:
	nlsfcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_MINPACK_H */

