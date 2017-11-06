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

MPT_SOLVER_STRUCT(bacol_fcn)
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
MPT_SOLVER_STRUCT(bacol_out)
{
#ifdef __cplusplus
public:
	bacol_out();
	~bacol_out();
	
	inline const double *x() const
	{ return nint ? static_cast<double *>(_val.iov_base) : 0; }
	const double *y() const
	{
		const double *val = static_cast<double *>(_val.iov_base);
		if (!val || !nint || deriv) return 0;
		return val + nint + 1;
	}
	inline int intervals() const
	{ return nint; }
	inline int equotations() const
	{ return neqs; }
	inline void invalidate()
	{ nint = 0; }
	
	bool set(const MPT_SOLVER_STRUCT(bacol) *);
protected:
#endif
	/* adapt grid data */
	int (*update)(int , const double *, int , double *);
	
	struct iovec _val,  /* output values */
	             _wrk;  /* work space */
	
	uint32_t  nint;     /* interval count */
	uint16_t  neqs;     /* equotation count */
	uint8_t   deriv;    /* derivation count */
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
	MPT_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	double  t,           /* reached time step */
	       *xy;          /* internal grid points (nintmx+1) and bspline coefficients for y */
	
	MPT_SOLVER_TYPE(dvecpar) atol, rtol; /* tolerances */
	
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
	
	struct iovec rpar,   /* floating point state data */
	             ipar;   /* integer state data */
	union {
	struct iovec cpar;   /* radau complex parameter */
	double tstop;        /* dassl tstop value */
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
extern int mpt_bacol_set(MPT_SOLVER_STRUCT(bacol) *, const char *, const MPT_INTERFACE(metatype) *);
/* validate settings and working space for use */
extern int mpt_bacol_prepare(MPT_SOLVER_STRUCT(bacol) *);

/* bacol status information */
extern int mpt_bacol_report(const MPT_SOLVER_STRUCT(bacol) *, int , MPT_TYPE(PropertyHandler) , void *);

/* default helper functions */
extern int mpt_bacol_grid_init(int , const double *, int , double *);

#ifndef __cplusplus
/* create generic solver type */
extern MPT_SOLVER(interface) *mpt_bacol_create(void);
#endif

/* init bacol output data */
extern void mpt_bacol_output_init(MPT_SOLVER_STRUCT(bacol_out) *);
/* finish bacol output data */
extern void mpt_bacol_output_fini(MPT_SOLVER_STRUCT(bacol_out) *);
/* report output state */
extern int mpt_bacol_output_report(const MPT_SOLVER_STRUCT(bacol_out) *, double , int (*)(void *, MPT_STRUCT(value)), void *);

/* generate output values from current bacol state */
extern const double *mpt_bacol_values(MPT_SOLVER_STRUCT(bacol_out) *, const MPT_SOLVER_STRUCT(bacol) *);

__MPT_EXTDECL_END

#ifdef __cplusplus
inline bacol::bacol()
{ mpt_bacol_init(this); }
inline bacol::~bacol()
{ mpt_bacol_fini(this); }

inline bacol_out::bacol_out()
{ mpt_bacol_output_init(this); }
inline bacol_out::~bacol_out()
{ mpt_bacol_output_fini(this); }

class Bacol : public IVP, bacol
{
public:
	inline Bacol(const char *t = 0)
	{
		mpt_bacol_backend(this, t);
	}
	~Bacol() __MPT_OVERRIDE
	{ }
	int step(double t) __MPT_OVERRIDE
	{
		return mpt_bacol_step(this, t);
	}
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_bacol_get(this, pr);
	}
	int setProperty(const char *pr, const metatype *src = 0) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_bacol_prepare(this);
		}
		_values.invalidate();
		return mpt_bacol_set(this, pr, src);
	}
	int report(int what, PropertyHandler out, void *opar) __MPT_OVERRIDE
	{
		if (what & Values) {
			update();
			if (out) {
				struct context {
					PropertyHandler out;
					void *par;
					static int setProp(void *ptr, value val)
					{
						const context *ctx = static_cast<context *>(ptr);
						struct property pr;
						pr.desc = "BACOL solver state";
						pr.val = val;
						return ctx->out(ctx->par, &pr);
					}
				};
				struct context ctx;
				ctx.out = out;
				ctx.par = opar;
				mpt_bacol_output_report(&_values, t, context::setProp, &ctx);
			}
			what &= ~Values;
		}
		return mpt_bacol_report(this, what, out, opar);
	}
	int setFunctions(int , const void *) __MPT_OVERRIDE
	{
		return BadValue;
	}
	
	inline const double *x() const
	{
		return _values.x();
	}
	inline const double *y() const
	{
		return _values.y();
	}
	inline long update()
	{
		long len = _values.intervals();
		if (!len) {
			if (!mpt_bacol_values(&_values, this)
			 || !(len = _values.intervals())) {
				return 0;
			}
		}
		return len + 1;
	}
protected:
	struct bacol_out _values;
};
#endif
__MPT_SOLVER_END

#endif /* _MPT_BACOL_H */

