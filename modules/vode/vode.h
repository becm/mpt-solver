/*!
 * interface to dVODE solver
 */

#ifndef _MPT_VODE_H
#define _MPT_VODE_H	201405

#include "../solver.h"

#include <sys/uio.h>

__MPT_SOLVER_BEGIN

typedef void vode_fcn_t(int *, double *, double *, double *, double *, int *);
typedef void vode_jac_t(int *, double *, double *, int *, int *, double *, int *, double *, int *);

MPT_SOLVER_STRUCT(vode)
{
#ifdef __cplusplus
public:
	~vode();
	vode();
#endif
	MPT_SOLVER_STRUCT(ivppar) ivp; /* inherit IVP parameter */
	MPT_TYPE(dvecpar) rtol, atol;  /* tolerances */
	
	char meth, miter, /* method flags */
	     jsv,         /* save jacobian iteration */
	     iopt;        /* flag for optional input */
	
	short	istate;   /* dvode state */
	short	itask;    /* type of integration step */
	
	struct iovec rwork, /* vode real work vector */
	             iwork; /* vode integer work vector */
	
	double *rpar;     /* real parameters */
	int    *ipar;     /* integer parameters */
	
	vode_fcn_t *fcn;  /* calculate right-hand side */
	vode_jac_t *jac;  /* user supplied jacobian */
};

__MPT_EXTDECL_BEGIN

/* c definition for vode fortran call */
extern void dvode_(vode_fcn_t *, int *, double *, double *, double *, int *, double *, double *,
		   int *, int *, int *, double *, int *, int *, int *,
		   vode_jac_t *, int *, double *, int *);

/* execute next step on supplied vode instance */
extern int mpt_vode_step(MPT_SOLVER_STRUCT(vode) *, double *, double );
extern int mpt_vode_step_(MPT_SOLVER_STRUCT(vode) *, double *, double *, MPT_SOLVER_STRUCT(ivpfcn) *);

/* set vode parameter */
extern int mpt_vode_property(MPT_SOLVER_STRUCT(vode) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *);

/* validate settings and working space for use */
extern int mpt_vode_prepare(MPT_SOLVER_STRUCT(vode) *__vd, int __neqs, int __pdim);

/* initialize/clear vode integrator descriptor */
extern int mpt_vode_init(MPT_SOLVER_STRUCT(vode) *__vd);
extern void mpt_vode_fini(MPT_SOLVER_STRUCT(vode) *__vd);
/* set wrapper for user functions */
extern int mpt_vode_ufcn(MPT_SOLVER_STRUCT(vode) *__vd, MPT_SOLVER_STRUCT(ivpfcn) *__uf);

/* vode status information */
extern int mpt_vode_report(const MPT_SOLVER_STRUCT(vode) *, int , MPT_TYPE(PropertyHandler) , void *);

/* setup generic solver to use vode */
#ifndef __cplusplus
extern MPT_SOLVER_INTERFACE *mpt_vode_create();
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline vode::vode()
{ mpt_vode_init(this); }
inline vode::~vode()
{ mpt_vode_fini(this); }

class Vode : public Ivp, vode
{
public:
	Vode() : _fcn(0)
	{ }
	virtual ~Vode()
	{ }
	Vode *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return mpt_vode_property(this, pr, src); }
	int report(int what, PropertyHandler out, void *opar) const
	{ return mpt_vode_report(this, what, out, opar); }
	int step(double *u, double *end, double *x)
	{ return mpt_vode_step_(this, u, end, &_fcn); }
	operator ivpfcn *() const
	{ return const_cast<ivpfcn *>(&_fcn); }
protected:
	ivpfcn _fcn;
};
#endif

__MPT_SOLVER_END

#endif /* _MPT_VODE_H */

