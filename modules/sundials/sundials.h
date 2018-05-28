/*!
 * interface to SUNDIALS solver family
 */

#ifndef _MPT_SUNDIALS_H
#define _MPT_SUNDIALS_H  @INTERFACE_VERSION@

#include "../solver.h"

#ifdef __cplusplus
# include <sundials/sundials_nvector.h>
# include <sundials/sundials_matrix.h>
# include <sundials/sundials_linearsolver.h>
#endif

__MPT_SOLVER_BEGIN

#ifndef _NVECTOR_H
typedef void * N_Vector;
#endif

#ifndef _SUNMATRIX_H
typedef void * SUNMatrix;
#endif

#ifndef _SUNLINEARSOLVER_H
typedef void * SUNLinearSolver;
#endif

#ifndef _SUNDIALSTYPES_H
MPT_SOLVER_STRUCT(sundials);
#else
MPT_SOLVER_STRUCT(sundials)
{
# ifdef __cplusplus
	sundials();
	~sundials();
#  define MPT_SOLVER_SUNDIALS(x) x
# else
#  define MPT_SOLVER_SUNDIALS(x) MPT_SOLVER_ENUM(Sundials##x)
# endif
	N_Vector y;  /* output container */
	
	SUNMatrix A; /* linear solver matrix and backend */
	SUNLinearSolver LS;
	
	sunindextype mu, ml;   /* band matrix parameters */
	
	int8_t jacobian,  /* jacobian flags */
	       step,      /* step strategy control */
	       stype,     /* type of solver */
	       prec,      /* preconditioner mode */
	       kmax;      /* max. Krylov subspace size */
# ifndef __cplusplus
}; /* sundials */
# endif
#endif /* _SUNDIALSTYPES_H */

enum MPT_SOLVER_SUNDIALS(Type) {
#ifdef _SUNDIALSTYPES_H
# if defined(SUNDIALS_SINGLE_PRECISION)
	MPT_SOLVER_SUNDIALS(Realtype)  = 'f',
# elif defined(SUNDIALS_DOUBLE_PRECISION)
	MPT_SOLVER_SUNDIALS(Realtype)  = 'd',
# elif defined(SUNDIALS_EXTENDED_PRECISION)
	MPT_SOLVER_SUNDIALS(Realtype)  = 'e',
# endif
# ifdef SUNDIALS_INT64_T
	MPT_SOLVER_SUNDIALS(Indextype) = 'x',
# else
	MPT_SOLVER_SUNDIALS(Indextype) = 'i',
# endif
#endif
	/* iterative solvers */
	MPT_SOLVER_SUNDIALS(IterGMR)   = 0x1,
	MPT_SOLVER_SUNDIALS(IterBCG)   = 0x2,
	MPT_SOLVER_SUNDIALS(IterTFQMR) = 0x3,
	
	/* linear algebra setup */
	MPT_SOLVER_SUNDIALS(Direct)    = 0x10,
	MPT_SOLVER_SUNDIALS(Lapack)    = 0x11,
	MPT_SOLVER_SUNDIALS(Numeric)   = 0x20
};

#ifdef __cplusplus
}; /* sundials */
#endif

MPT_SOLVER_STRUCT(cvode)
#ifdef _SUNDIALSTYPES_H
{
# ifdef __cplusplus
public:
	cvode();
	~cvode();
# endif
	MPT_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	MPT_SOLVER_TYPE(dvecpar) rtol,  /* relative tolerance scalar/vector */
	                         atol;  /* absolute tolerance scalar/vector */
	
	MPT_SOLVER_STRUCT(sundials) sd; /* general sundials data */
	void *mem;                      /* CVode memory block */
	
	realtype t;     /* current time step */
	realtype hmax;  /* CVode only saves inverse */
	
	const MPT_IVP_STRUCT(odefcn) *ufcn;
}
#endif
;

MPT_SOLVER_STRUCT(ida)
#ifdef _SUNDIALSTYPES_H
{
# ifdef __cplusplus
public:
	ida();
	~ida();
protected:
# endif
	MPT_IVP_STRUCT(parameters) ivp; /* inherit IVP parameter */
	
	MPT_SOLVER_TYPE(dvecpar) rtol,  /* relative tolerance scalar/vector */
	                         atol;  /* absolute tolerance scalar/vector */
	
	MPT_SOLVER_STRUCT(sundials) sd; /* general sundials data */
	void *mem;                      /* IDA memory block */
	
	realtype t;     /* current time step */
	realtype hmax;  /* IDA only saves inverse */
	
	const MPT_IVP_STRUCT(daefcn) *ufcn;
	
	N_Vector yp;    /* deviation vector */
	
	struct {
		void   *base;
		size_t  size;
	} tmp;          /* temporary data buffer */
}
#endif
;

#define SUNDIALS_flags_jac(f,s)  (f = ((f) & ~0xfL) | ((s) & 0xfL))

__MPT_EXTDECL_BEGIN

/* calculate errors tolerances */
extern int mpt_sundials_ewtfcn(N_Vector , N_Vector , void *);

/* clear SUNDIALS data */
extern void mpt_sundials_init(MPT_SOLVER_STRUCT(sundials) *);
extern void mpt_sundials_fini(MPT_SOLVER_STRUCT(sundials) *);

/* initialize values */
extern int mpt_sundials_vector_set(N_Vector *, long , long , MPT_INTERFACE(iterator) *);

/* set CVode parameter */
extern int mpt_sundials_cvode_get(const MPT_SOLVER_STRUCT(cvode) *, MPT_STRUCT(property) *);
extern int mpt_sundials_cvode_set(MPT_SOLVER_STRUCT(cvode) *, const char *, const MPT_INTERFACE(metatype) *);
/* set IDA solver parameter */
extern int mpt_sundials_ida_get(const MPT_SOLVER_STRUCT(ida) *, MPT_STRUCT(property) *);
extern int mpt_sundials_ida_set(MPT_SOLVER_STRUCT(ida) *, const char *, const MPT_INTERFACE(metatype) *);

/* initialize/finalize Sundials CVode solver */
extern int  mpt_sundials_cvode_init(MPT_SOLVER_STRUCT(cvode) *);
extern void mpt_sundials_cvode_fini(MPT_SOLVER_STRUCT(cvode) *);
extern void mpt_sundials_cvode_reset(MPT_SOLVER_STRUCT(cvode) *);
/* initialize/finalize Sundials IDA integrator */
extern int  mpt_sundials_ida_init(MPT_SOLVER_STRUCT(ida) *);
extern void mpt_sundials_ida_fini(MPT_SOLVER_STRUCT(ida) *);
extern void mpt_sundials_ida_reset(MPT_SOLVER_STRUCT(ida) *);

/* prepare and report solver state */
extern int mpt_sundials_cvode_report(const MPT_SOLVER_STRUCT(cvode) *, int , MPT_TYPE(PropertyHandler) , void *);
/* report solver state */
extern int mpt_sundials_ida_report(const MPT_SOLVER_STRUCT(ida) *, int , MPT_TYPE(PropertyHandler) , void *);

/* prepare CVode */
extern int mpt_sundials_cvode_prepare(MPT_SOLVER_STRUCT(cvode) *);
/* prepare IDA */
extern int mpt_sundials_ida_prepare(MPT_SOLVER_STRUCT(ida) *);

/* temporary data for IDA */
extern void *mpt_sundials_ida_tmp(MPT_SOLVER_STRUCT(ida) *, size_t , size_t);

#ifdef _SUNDIALSTYPES_H
/* execute CVode step (cvdata, data, tend) */
extern int mpt_sundials_cvode_step(MPT_SOLVER_STRUCT(cvode) *, realtype);
/* execute IDA step(solver, x, tend) */
extern int mpt_sundials_ida_step(MPT_SOLVER_STRUCT(ida) *, realtype);

/* create Sundials NVector with data */
extern N_Vector mpt_sundials_nvector(sunindextype);
/* setup linear solver */
extern int mpt_sundials_linear(MPT_SOLVER_STRUCT(sundials) *, sunindextype);
#endif

#ifndef __cplusplus
extern MPT_INTERFACE(metatype) *mpt_sundials_cvode(void);
extern MPT_INTERFACE(metatype) *mpt_sundials_ida(void);
#endif

/* setup Sundials jacobian parameters */
extern int mpt_sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *, long , const MPT_INTERFACE(metatype) *);
/* output Sundials jacobian parameters */
extern int mpt_sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *, MPT_TYPE(PropertyHandler) out, void *usr);

#ifndef _SUNDIALS_GENERIC_TYPE
# define _SUNDIALS_GENERIC_TYPE(x) x
#endif

#ifdef _SUNDIALSTYPES_H
/* wrapper for CVode user functions */
extern int mpt_sundials_cvode_fcn(realtype , N_Vector , N_Vector , _SUNDIALS_GENERIC_TYPE(const MPT_SOLVER_STRUCT(cvode)) *);
/* wrapper for IDA user functions */
extern int mpt_sundials_ida_fcn(realtype , N_Vector , N_Vector , N_Vector , _SUNDIALS_GENERIC_TYPE(MPT_SOLVER_STRUCT(ida)) *);

# ifdef _SUNMATRIX_H
#  ifdef _SUNDIALS_DIRECT_H
/* Dense/Banded wrapper for CVode jacobian */
extern int mpt_sundials_cvode_jac(realtype , N_Vector , N_Vector ,
                                  SUNMatrix , _SUNDIALS_GENERIC_TYPE(const MPT_SOLVER_STRUCT(cvode)) *,
                                  N_Vector , N_Vector , N_Vector);

/* Dense/Banded wrapper for IDA jacobian */
extern int mpt_sundials_ida_jac(realtype , realtype , N_Vector , N_Vector , N_Vector ,
                                SUNMatrix , _SUNDIALS_GENERIC_TYPE(MPT_SOLVER_STRUCT(ida)) *,
                                N_Vector , N_Vector , N_Vector);
#  endif
# endif
#endif

__MPT_EXTDECL_END


#ifdef __cplusplus
inline sundials::sundials()
{ mpt_sundials_init(this); }
inline sundials::~sundials()
{ mpt_sundials_fini(this); }

class CVode : public IVP, cvode
{
public:
	CVode() : _fcn(0)
	{
		ufcn = &_fcn;
	}
	~CVode() __MPT_OVERRIDE
	{ }
	/* object operations */
	int property_get(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_sundials_cvode_get(this, pr);
	}
	int property_set(const char *pr, const metatype *src) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_sundials_cvode_prepare(this);
		}
		return mpt_sundials_cvode_set(this, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return mpt_sundials_cvode_step(this, _t);
	}
	int report(int what, PropertyHandler out, void *opar) __MPT_OVERRIDE
	{
		return mpt_sundials_cvode_report(this, what, out, opar);
	}
	int setFunctions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return mpt_solver_module_ufcn_ode(ivp.pint, &_fcn, type, ptr);
	}
protected:
	struct odefcn _fcn;
};
inline cvode::cvode()
{ mpt_sundials_cvode_init(this); }
inline cvode::~cvode()
{ mpt_sundials_cvode_fini(this); }

class IDA : public IVP, ida
{
public:
	IDA() : _fcn(0)
	{
		ufcn = &_fcn;
	}
	~IDA() __MPT_OVERRIDE
	{ }
	/* object operations */
	int property_get(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_sundials_ida_get(this, pr);
	}
	int property_set(const char *pr, const metatype *src) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_sundials_ida_prepare(this);
		}
		return mpt_sundials_ida_set(this, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return mpt_sundials_ida_step(this, _t);
	}
	int report(int what, PropertyHandler out, void *opar) __MPT_OVERRIDE
	{
		return mpt_sundials_ida_report(this, what, out, opar);
	}
	int setFunctions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return mpt_solver_module_ufcn_dae(ivp.pint, &_fcn, type, ptr);
	}
protected:
	struct daefcn _fcn;
};
inline ida::ida()
{ mpt_sundials_ida_init(this); }
inline ida::~ida()
{ mpt_sundials_ida_fini(this); }

#endif

__MPT_SOLVER_END

#endif /* _MPT_SUNDIALS_H */

