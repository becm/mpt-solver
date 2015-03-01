/*!
 * interface to Port N2 solver family
 */

#ifndef _MPT_PORTDN2_H
#define _MPT_PORTDN2_H	201405

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void portn2_fcn_t(const int *, const int *, const double *, int *, double *, int *, double *, void (*)());
typedef void portn2p_fcn_t(const int *, const int *, const int *, const int *, const int *, const double *, int *, double *, int *, double *, void (*)());

MPT_SOLVER_STRUCT(portn2)
{
#ifdef __cplusplus
public:
	portn2();
	~portn2();
#endif
	MPT_SOLVER_STRUCT(nlspar) nls; /* inherit nonlinear system parameter */
	
	struct iovec bnd, /* parameter restrictions */
	             rv,  /* double working space */
	             iv;  /* integer working space */
	
	int nd, pad;      /* partial residual dimension, no jacobian if < 0 */
	
	union {
	portn2_fcn_t  *res;
	portn2p_fcn_t *pres;
	} res;            /* residual function */
	
	union {
	portn2_fcn_t  *jac;
	portn2p_fcn_t *pjac;
	} jac;            /* jacobian function */
	
	int    *ui;       /* user supplied integer vector */
	double *ur;       /* user supplied double vector */
	
	void (*uf)();     /* user supplied subroutine */
};

__MPT_EXTDECL_BEGIN

/* numeric jacobian, no restrictions */
extern int dn2f_(const int *n, const int *p,
                 const double *x, portn2_fcn_t *calcr,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());
/* numeric jacobian, with restrictions */
extern int dn2fb_(const int *n, const int *p, const double *b,
                  const double *x, portn2_fcn_t *calcr,
                  int *iv, const int *liv, const int *lv, double *v,
                  int *ui, double *ur, void (*uf)());

/* analytic jacobian, no restrictions */
extern int dn2g_(const int *n, const int *p,
                 const double *x, portn2_fcn_t *calcr, portn2_fcn_t *calcj,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());
/* analytic jacobian, with restrictions */
extern int dn2gb_(const int *n, const int *p, const double *b,
                 const double *x, portn2_fcn_t *calcr, portn2_fcn_t *calcj,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());

/* partial analytic jacobian, partial residual, no restrictions */
extern int dn2p_(const int *n, const int *nd, const int *p,
                 const double *x, portn2p_fcn_t *calcr, portn2p_fcn_t *calcj,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());
/* partial analytic jacobian, partial residual, with restrictions */
extern int dn2pb_(const int *n, const int *nd, const int *p, const double *b,
                 const double *x, portn2p_fcn_t *calcr, portn2p_fcn_t *calcj,
                 int *iv, const int *liv, const int *lv, double *v,
                 int *ui, double *ur, void (*uf)());


/* set solver parameter */
extern int mpt_portn2_property(MPT_SOLVER_STRUCT(portn2) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *);

/* call solver routine */
extern int mpt_portn2_step(MPT_SOLVER_STRUCT(portn2) *, double *, double *);
extern int mpt_portn2_step_(MPT_SOLVER_STRUCT(portn2) *, double *, double *, const MPT_SOLVER_STRUCT(nlsfcn) *);

/* initialize/terminate structured data */
extern int mpt_portn2_init(MPT_SOLVER_STRUCT(portn2) *);
extern void mpt_portn2_fini(MPT_SOLVER_STRUCT(portn2) *);

/* set wrapper for user functions */
extern int mpt_portn2_ufcn(MPT_SOLVER_STRUCT(portn2) *, const MPT_SOLVER_STRUCT(nlsfcn) *);

/* set wrapper for user functions */
extern int mpt_portn2_prepare(MPT_SOLVER_STRUCT(portn2) *, int , int );

/* portdn2 status information */
extern int mpt_portn2_report(const MPT_SOLVER_STRUCT(portn2) *, int , MPT_TYPE(PropertyHandler) , void *);

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
	PortN2 *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return mpt_portn2_property(this, pr, src); }
	int report(int what, PropertyHandler out, void *data) const
	{ return mpt_portn2_report(this, what, out, data); }
	int step(double *x, double *f)
	{ return mpt_portn2_step_(this, x, f, &_fcn); }
	
	operator const nlspar *() const
	{ return &this->nls; }
	
	operator nlsfcn *() const
	{ return const_cast<nlsfcn *>(&_fcn); }
	
    protected:
	nlsfcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_PORTDN2_H */

