/*!
 * interface to Port N2 solver family
 */

#ifndef _MPT_PORTDN2_H
#define _MPT_PORTDN2_H	201405

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void MPT_SOLVER_TYPE(PortN2Fcn) (const int *, const int *, const double *, int *, double *, int *, double *, void (*)());
typedef void MPT_SOLVER_TYPE(PortN2pFcn)(const int *, const int *, const int *, const int *, const int *, const double *, int *, double *, int *, double *, void (*)());

MPT_SOLVER_STRUCT(portn2)
{
#ifdef __cplusplus
public:
	portn2();
	~portn2();
#endif
	MPT_SOLVER_STRUCT(nlspar) nls; /* inherit nonlinear system parameter */
	
	int          nd,  /* partial residual dimension, no jacobian if < 0 */
	             bnd; /* apply parameter boundaries */
	
	struct iovec pv,  /* parameter and restrictions */
	             rv,  /* double working space */
	             iv;  /* integer working space */
	
	union {
	MPT_SOLVER_TYPE(PortN2Fcn)  *res;
	MPT_SOLVER_TYPE(PortN2pFcn) *pres;
	} res;            /* residual function */
	
	union {
	MPT_SOLVER_TYPE(PortN2Fcn)  *jac;
	MPT_SOLVER_TYPE(PortN2pFcn) *pjac;
	} jac;            /* jacobian function */
	
	int    *ui;       /* user supplied integer vector */
	double *ur;       /* user supplied double vector */
	
	void (*uf)();     /* user supplied subroutine */
};

__MPT_EXTDECL_BEGIN

/* numeric jacobian, no restrictions */
extern int dn2f_(const int *n, const int *p, double *x,
                 MPT_SOLVER_TYPE(PortN2Fcn) *calcr,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());
/* numeric jacobian, with restrictions */
extern int dn2fb_(const int *n, const int *p, double *x, const double *b,
                  MPT_SOLVER_TYPE(PortN2Fcn) *calcr,
                  int *iv, const int *liv, const int *lv, double *v,
                  int *ui, double *ur, void (*uf)());

/* analytic jacobian, no restrictions */
extern int dn2g_(const int *n, const int *p, double *x,
                 MPT_SOLVER_TYPE(PortN2Fcn) *calcr, MPT_SOLVER_TYPE(PortN2Fcn) *calcj,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());
/* analytic jacobian, with restrictions */
extern int dn2gb_(const int *n, const int *p, double *x, const double *b,
                 MPT_SOLVER_TYPE(PortN2Fcn) *calcr, MPT_SOLVER_TYPE(PortN2Fcn) *calcj,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());

/* partial analytic jacobian, partial residual, no restrictions */
extern int dn2p_(const int *n, const int *nd, const int *p, double *x,
                 MPT_SOLVER_TYPE(PortN2pFcn) *calcr, MPT_SOLVER_TYPE(PortN2pFcn) *calcj,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());
/* partial analytic jacobian, partial residual, with restrictions */
extern int dn2pb_(const int *n, const int *nd, const int *p, double *x, const double *b,
                 MPT_SOLVER_TYPE(PortN2pFcn) *calcr, MPT_SOLVER_TYPE(PortN2pFcn) *calcj,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());


/* set solver parameter */
extern int mpt_portn2_get(const MPT_SOLVER_STRUCT(portn2) *, MPT_STRUCT(property) *);
extern int mpt_portn2_set(MPT_SOLVER_STRUCT(portn2) *, const char *, MPT_INTERFACE(metatype) *);

/* call solver routine */
extern int mpt_portn2_solve(MPT_SOLVER_STRUCT(portn2) *);

/* initialize/terminate structured data */
extern int mpt_portn2_init(MPT_SOLVER_STRUCT(portn2) *);
extern void mpt_portn2_fini(MPT_SOLVER_STRUCT(portn2) *);

/* set wrapper for user functions */
extern int mpt_portn2_ufcn(MPT_SOLVER_STRUCT(portn2) *, const MPT_SOLVER_STRUCT(nlsfcn) *);

/* set wrapper for user functions */
extern int mpt_portn2_prepare(MPT_SOLVER_STRUCT(portn2) *, int , int);

/* portdn2 status information */
extern int mpt_portn2_report(const MPT_SOLVER_STRUCT(portn2) *, int , MPT_TYPE(PropertyHandler) , void *);

/* get residuals for current parameters */
extern const double *mpt_portn2_residuals(const MPT_SOLVER_STRUCT(portn2) *);

/* assign portdn2 solver to interface */
#ifndef __cplusplus
extern MPT_SOLVER_INTERFACE *mpt_portn2_create(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline portn2::portn2()
{ mpt_portn2_init(this); }
inline portn2::~portn2()
{ mpt_portn2_fini(this); }

class PortN2 : public Nls, portn2
{
    public:
	PortN2() : _fcn(0)
	{ }
	virtual ~PortN2()
	{ }
	uintptr_t addref()
	{
		return 0;
	}
	void unref()
	{
		delete this;
	}
	int property(struct property *pr) const
	{
		return mpt_portn2_get(this, pr);
	}
	int setProperty(const char *pr, metatype *src = 0)
	{
		return mpt_portn2_set(this, pr, src);
	}
	
	int report(int what, PropertyHandler out, void *data)
	{
		return mpt_portn2_report(this, what, out, data);
	}
	int solve()
	{
		int ret;
		if (_fcn.res && (ret = mpt_portn2_ufcn(this, &_fcn)) < 0) {
			return ret;
		}
		if (mpt_portn2_prepare(this, nls.nval, nls.nres) < 0) {
			return ret;
		}
		return mpt_portn2_solve(this);
	}
	operator nlsfcn *() const
	{
		return const_cast<nlsfcn *>(&_fcn);
	}
	
	inline operator const nlspar *() const
	{
		return &nls;
	}
    protected:
	nlsfcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_PORTDN2_H */

