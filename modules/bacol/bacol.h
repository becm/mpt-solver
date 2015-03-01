/*!
 * interface to BACOL solver
 */

#ifndef _MPT_BACOL_H
#define _MPT_BACOL_H  201502

#include "../solver.h"

#include <sys/uio.h>

#ifdef __cplusplus
# include <stdlib.h>
# include <iostream>
#endif

__MPT_SOLVER_BEGIN

MPT_SOLVER_STRUCT(bacolfcn)
{
	void (*f)(double *, double *, double *, double *, double *, double *, int *);
	void (*derivf)(double *, double *, double *, double *, double *, double *, double *, double *, int *);
	
	void (*bndxa)(double *, double *, double *, double *, int *);
	void (*bndxb)(double *, double *, double *, double *, int *);
	
	void (*difbxa)(double *, double *, double *, double *, double *, double *, int *);
	void (*difbxb)(double *, double *, double *, double *, double *, double *, int *);
	
	void (*uinit)(double *, double *, int *);
};

MPT_SOLVER_STRUCT(bacol)
{
#ifdef __cplusplus
public:
	bacol(int type = 'd');
	~bacol();
#endif
	MPT_SOLVER_STRUCT(ivppar) ivp; /* inherit IVP parameter */
	MPT_TYPE(dvecpar) atol, rtol;  /* tolerances */
	
	const short backend; /* solver backend */
	short       kcol;    /* collocation points per subinterval [1..10] */
	int         nintmx,  /* maximal internal intervals */
	            nint;    /* current internal intervals */
	
	struct {
		int noinit,  /* no initial call */
		    tvec,    /* tolerances are vectors */
		    tstop,   /* tstop = rpar[0] > tend */
		    mstep,   /* set max. steps per call in ipar[7] */
		    bdir,    /* set if both boundary contitions are dirichlet */
		    step,    /* user specified initial step size on rpar[1] */
		    dbmax;   /* max. number of bdf methods for dassl on ipar[14] */
	} mflag;
	
	
	double initstep;  /* initial stepsize */
	
	double *x,  /* x-values for internal grid points (nintmx+1) */
	       *y;  /* bspline coefficients for y */
	
	struct iovec rpar,  /* real parameters */
	             ipar,  /* integer parameters */
	             owrk;  /* work space for output creation */
	
	/* bacol user functions, set global by bacol_default() */
	MPT_SOLVER_STRUCT(bacolfcn) *ufcn;
	
	/* initialize grid (nval, xsrc, max. intv, x, intv) */
	int (*xinit)(int , const double *, int , double *, int );
	
	/* adapt output grid (time, intv, x, max. dim, xout) */
	int (*xgrid)(double , int , const double *, int , double *);
	
	union {
		struct iovec cpar;  /* radau complex parameter */
		double tstop;       /* dassl tstop value */
	} bd;
};

#define MPT_BACOL_NCONTI  2

__MPT_EXTDECL_BEGIN

/* c definition for bacol fortran call */
extern void bacol_(double *, double *, double *, double *, int *, int *, int *, int *,
                   double *, int *, double *, int *, int *, int *, double *, int *);
extern void bacolr_(double *, double *, double *, double *, int *, int *, int *, int *,
                    double *, int *, double *, int *, int *, int *,
                    void *, int *, double *, int *);

/* c definition for bacol output interpolation */
extern void values_(int *, double *, int *, double *, int *, int *,
                    int *, double *, double *, double *);

/* execute next step on supplied bacol instance */
extern int mpt_bacol_sstep(MPT_SOLVER_STRUCT(bacol) *, double );
extern int mpt_bacol_step(MPT_SOLVER_STRUCT(bacol) *, double *, double , double *);
extern int mpt_bacol_step_(MPT_SOLVER_STRUCT(bacol) *, double *, double *, double *);

/* generate output values from current bacol state */
extern int mpt_bacol_values(MPT_SOLVER_STRUCT(bacol) *, double *, int , double *);

/* initialize/clear bacol(r) integrator memory */
extern void mpt_bacol_init(MPT_SOLVER_STRUCT(bacol) *);
extern void mpt_bacol_fini(MPT_SOLVER_STRUCT(bacol) *);

/* set bacol parameter */
extern int mpt_bacol_property(MPT_SOLVER_STRUCT(bacol) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *src);
/* validate settings and working space for use */
extern int mpt_bacol_prepare(MPT_SOLVER_STRUCT(bacol) *, int , int );

/* bacol status information */
extern int mpt_bacol_report(const MPT_SOLVER_STRUCT(bacol) *, int , MPT_TYPE(PropertyHandler) , void *);

/* default helper functions */
extern int mpt_bacol_grid_init(int , const double *, int , double *, int );

#ifndef __cplusplus
/* open/close handler for generic solver type */
extern MPT_SOLVER_INTERFACE *mpt_bacol_create(void);
#endif

__MPT_EXTDECL_END

#ifdef __cplusplus
inline bacol::bacol(int type) : backend(type)
{
	mpt_bacol_init(this);
	switch (type) {
		case 'r': case 'R': *((short *) &backend) = 'r'; break;
		default:;
	}
}
inline bacol::~bacol()
{ mpt_bacol_fini(this); }

class Bacol : public Ivp, bacol
{
public:
	Bacol(const char *t = 0) : bacol(t ? *t : 'd')
	{ }
	virtual ~Bacol()
	{ }
	Bacol *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return mpt_bacol_property(this, pr, src); }
	int report(int what, PropertyHandler out, void *opar) const
	{ return mpt_bacol_report(this, what, out, opar); }
	int step(double *u, double *tend, double *x)
	{ return mpt_bacol_step_(this, u, tend, x); }
	operator ivppar *() const
	{ return (ivppar *) &this->ivp; }
};
#endif
__MPT_SOLVER_END

#endif /* _MPT_BACOL_H */

