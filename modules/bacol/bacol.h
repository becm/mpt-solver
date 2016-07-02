/*!
 * interface to BACOL solver
 */

#ifndef _MPT_BACOL_H

#include "../solver.h"
#define _MPT_BACOL_H  _MPT_SOLVER_H

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

MPT_SOLVER_STRUCT(bacol);
MPT_SOLVER_STRUCT(bacolout)
{
#ifdef __cplusplus
public:
	bacolout();
	~bacolout();
#endif
	/* adapt grid data */
	int (*update)(int , const double *, int , double *);
	
	struct iovec xy, wrk;  /* buffer and work space for output creation */
	
	uint32_t     nint;     /* interval count */
	uint16_t     neqs;     /* equotation count */
	uint8_t      deriv;    /* derivation count */
};

MPT_SOLVER_STRUCT(bacol)
{
#ifdef __cplusplus
public:
	bacol();
	~bacol();
#else
# define MPT_BACOL_NIMAXDEF 127
# define MPT_BACOL_NCONTI   2
#endif
	MPT_SOLVER_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	double  t,           /* reached time step */
	       *xy;          /* internal grid points (nintmx+1) and bspline coefficients for y */
	
	MPT_SOLVER_TYPE(dvecpar) atol, rtol; /* tolerances */
	
	MPT_SOLVER_STRUCT(bacolout) *_out; /* BACOL output data */
	
	double initstep;     /* initial stepsize */
	
	struct {
		int noinit,  /* no initial call */
		    tvec,    /* tolerances are vectors */
		    tstop,   /* tstop = rpar[0] > tend */
		    mstep,   /* set max. steps per call in ipar[7] */
		    bdir,    /* set if both boundary contitions are dirichlet */
		    step,    /* user specified initial step size on rpar[1] */
		    dbmax;   /* max. number of bdf methods for dassl on ipar[14] */
	} mflag;
	
	int      nint,       /* current internal intervals */
	         nintmx;     /* maximum internal intervals */
	int16_t  kcol;       /* collocation points per subinterval [1..10] */
	char    _backend;    /* solver backend */
	
	struct iovec rpar,          /* floating point state data */
	             ipar;          /* integer state data */
	union {
		struct iovec cpar;  /* radau complex parameter */
		double tstop;       /* dassl tstop value */
	} bd;
};

__MPT_EXTDECL_BEGIN

/* c definition for bacol fortran call */
extern void bacol_(double *, double *, double *, double *, int *, int *, int *, int *,
                   double *, int *, double *, int *, int *, int *, double *, int *);
extern void bacolr_(double *, double *, double *, double *, int *, int *, int *, int *,
                    double *, int *, double *, int *, int *, int *,
                    void *, int *, double *, int *);

/* c definition for bacol output interpolation */
extern void values_(const int *, const double *, const int *, const double *, const int *, const int *,
                    const int *, double *, const double *, double *);

/* execute next step on supplied bacol instance */
extern int mpt_bacol_step(MPT_SOLVER_STRUCT(bacol) *, double);

/* initialize/clear bacol(r) integrator memory */
extern void mpt_bacol_init(MPT_SOLVER_STRUCT(bacol) *);
extern void mpt_bacol_fini(MPT_SOLVER_STRUCT(bacol) *);

/* set backend for solver instance */
extern int mpt_bacol_backend(MPT_SOLVER_STRUCT(bacol) *, const char *);

/* get/set bacol parameter */
extern int mpt_bacol_get(const MPT_SOLVER_STRUCT(bacol) *, MPT_STRUCT(property) *);
extern int mpt_bacol_set(MPT_SOLVER_STRUCT(bacol) *, const char *, MPT_INTERFACE(metatype) *);
/* validate settings and working space for use */
extern int mpt_bacol_prepare(MPT_SOLVER_STRUCT(bacol) *);

/* bacol status information */
extern int mpt_bacol_report(MPT_SOLVER_STRUCT(bacol) *, int , MPT_TYPE(PropertyHandler) , void *);

/* default helper functions */
extern int mpt_bacol_grid_init(int , const double *, int , double *);

#ifndef __cplusplus
/* open/close handler for generic solver type */
extern MPT_SOLVER(IVP) *mpt_bacol_create(void);
#endif

/* init bacol output data */
extern void mpt_bacolout_init(MPT_SOLVER_STRUCT(bacolout) *);
/* finish bacol output data */
extern void mpt_bacolout_fini(MPT_SOLVER_STRUCT(bacolout) *);

/* generate output values from current bacol state */
extern double *mpt_bacol_values(MPT_SOLVER_STRUCT(bacolout) *, const MPT_SOLVER_STRUCT(bacol) *);

__MPT_EXTDECL_END

#ifdef __cplusplus
inline bacol::bacol()
{ mpt_bacol_init(this); }
inline bacol::~bacol()
{ mpt_bacol_fini(this); }

inline bacolout::bacolout()
{ mpt_bacolout_init(this); }
inline bacolout::~bacolout()
{ mpt_bacolout_fini(this); }

class Bacol : public IVP, bacol
{
public:
	Bacol(const char *t = 0)
	{
		mpt_bacol_backend(this, t);
	}
	virtual ~Bacol()
	{ }
	void unref()
	{
		delete this;
	}
	uintptr_t addref()
	{
		return 0;
	}
	int property(struct property *pr) const
	{
		return mpt_bacol_get(this, pr);
	}
	int setProperty(const char *pr, metatype *src = 0)
	{
		return mpt_bacol_set(this, pr, src);
	}
	int report(int what, PropertyHandler out, void *opar)
	{
		return mpt_bacol_report(this, what, out, opar);
	}
	int step(double *tend)
	{
		if (!tend) return mpt_bacol_prepare(this);
		int ret = mpt_bacol_step(this, *tend);
		*tend = t;
		return ret;
	}
	inline const double *grid() const
	{
		return mpt_bacol_values(_out, this) ? (double *) _out->xy.iov_base : 0;
	}
	inline const double *values() const
	{
		const double *v = mpt_bacol_values(_out, this);
		return v && !_out->deriv ? v : 0;
	}
	inline int updateOutput()
	{
		if (!_out) _out = new bacolout;
		return mpt_bacol_values(_out, this) ? _out->nint+1 : BadOperation;
	}
};
#endif
__MPT_SOLVER_END

#endif /* _MPT_BACOL_H */

