/*!
 * interface to SUNDIALS solver family
 */

#ifndef _MPT_SUNDIALS_H
#define _MPT_SUNDIALS_H  @INTERFACE_VERSION@

#include "../solver.h"

#ifdef __cplusplus
#  define MPT_SOLVER_SUNDIALS(x) x
# else
#  define MPT_SOLVER_SUNDIALS(x) MPT_SOLVER_ENUM(Sundials##x)
#endif

/* type definition flag pre SUNDIALS 6 */
#ifdef _SUNDIALSTYPES_H
# ifndef _SUNDIALS_TYPES_H
#  define _SUNDIALS_TYPES_H _SUNDIALSTYPES_H
# endif
#endif

__MPT_SOLVER_BEGIN

#ifdef _SUNDIALS_TYPES_H
# ifndef _NVECTOR_H
typedef void * N_Vector;
# endif

# ifndef _SUNMATRIX_H
typedef void * SUNMatrix;
# endif

# ifndef _SUNLINEARSOLVER_H
typedef void * SUNLinearSolver;
# endif

# ifndef _SUNDIALS_CONTEXT_H
typedef void * SUNContext;
# endif
#endif /* _SUNDIALS_TYPES_H */

enum MPT_SOLVER_SUNDIALS(Type) {
#ifdef _SUNDIALS_TYPES_H
# if defined(SUNDIALS_SINGLE_PRECISION)
	MPT_SOLVER_SUNDIALS(Realtype)  = 'f',
# elif defined(SUNDIALS_DOUBLE_PRECISION)
	MPT_SOLVER_SUNDIALS(Realtype)  = 'd',
# elif defined(SUNDIALS_EXTENDED_PRECISION)
	MPT_SOLVER_SUNDIALS(Realtype)  = 'e',
# endif
# if defined(SUNDIALS_INT64_T)
	MPT_SOLVER_SUNDIALS(Indextype) = 'x',
# elif defined(SUNDIALS_INT32_T)
	MPT_SOLVER_SUNDIALS(Indextype) = 'i',
# endif
#endif /* _SUNDIALS_TYPES_H */
	/* iterative solvers */
	MPT_SOLVER_SUNDIALS(IterGMR)   = 0x1,
	MPT_SOLVER_SUNDIALS(IterBCG)   = 0x2,
	MPT_SOLVER_SUNDIALS(IterTFQMR) = 0x3,
	
	/* linear algebra setup */
	MPT_SOLVER_SUNDIALS(Direct)    = 0x10,
	MPT_SOLVER_SUNDIALS(Lapack)    = 0x11,
	MPT_SOLVER_SUNDIALS(Numeric)   = 0x20
};

MPT_SOLVER_STRUCT(sundials)
#ifdef _SUNDIALS_TYPES_H
{
# ifdef __cplusplus
	sundials();
	~sundials();
# endif
	N_Vector y;  /* output container */
	
	SUNLinearSolver LS;  /* linear solver backend */
	
# if SUNDIALS_VERSION_MAJOR >= 6
	SUNContext _sun_ctx; /* simulation context */
	void *_sun_ctx_ref;  /* context cookie */
# endif
	
	SUNMatrix A;          /* solver matrix */
	sunindextype ml, mu;  /* band matrix dimensions */
	
	int8_t jacobian,  /* jacobian flags */
	       linsol,    /* linear solver type */
	       prec,      /* preconditioner mode */
	       kmax;      /* max. Krylov subspace size */
}
#endif /* _SUNDIALS_TYPES_H */
;

MPT_SOLVER_STRUCT(sundials_step)
#ifdef _SUNDIALS_TYPES_H
{
# ifdef _MATH_H
#  ifdef __cplusplus
	inline sundials_step() : tstop(INFINITY), hin(0.0), hmin(0.0), hmax(0.0)
	{ }
#  else
#   define MPT_SOLVER_SUNDIALS_STEP_INIT { INFINITY, 0.0, 0.0, 0.0 }
#  endif
# endif
	realtype tstop; /* final limit of independent variable */
	
	realtype hin;   /* initial stepsize */
	
	realtype hmin;  /* minimal stapsize */
	realtype hmax;  /* maximal stapsize */
}
#endif  /* _SUNDIALS_TYPES_H */
;

MPT_SOLVER_STRUCT(cvode)
#ifdef _SUNDIALS_TYPES_H
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
	
	realtype t;      /* current time step */
	
	MPT_SOLVER_STRUCT(sundials_step) step;
	long    mxstep;  /* maximum iterations per solver step call */
	int     mxhnil;  /* threshold for (t + h = t) warnings */
	
	int     maxord;  /* maximum order of method */
	uint8_t method;  /* history array adjustment mode */
	
	const MPT_IVP_STRUCT(odefcn) *ufcn;
}
#endif /* _SUNDIALS_TYPES_H */
;

MPT_SOLVER_STRUCT(ida)
#ifdef _SUNDIALS_TYPES_H
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
	
	MPT_SOLVER_STRUCT(sundials_step) step;
	long mxstep;    /* maximum iterations per solver step call */
	
	int  maxord;    /* maximum order of method */
	
	const MPT_IVP_STRUCT(daefcn) *ufcn;
	
	N_Vector yp;    /* deviation vector */
	
	struct {
		void   *base;
		size_t  size;
	} tmp;          /* temporary data buffer */
}
#endif /* _SUNDIALS_TYPES_H */
;

#define SUNDIALS_flags_jac(f,s)  (f = ((f) & ~0xfL) | ((s) & 0xfL))

__MPT_EXTDECL_BEGIN

/* clear SUNDIALS data */
extern void mpt_sundials_init(MPT_SOLVER_STRUCT(sundials) *);
extern void mpt_sundials_fini(MPT_SOLVER_STRUCT(sundials) *);
#ifdef _SUNDIALS_TYPES_H
# if SUNDIALS_VERSION_MAJOR >= 6
extern SUNContext mpt_sundials_context(MPT_SOLVER_STRUCT(sundials) *);
# endif
#endif

#ifdef _SUNDIALS_TYPES_H
/* initialize values */
extern int mpt_sundials_vector_set(N_Vector *, long , long , MPT_INTERFACE(iterator) *);
/* calculate errors tolerances */
extern int mpt_sundials_ewtfcn(N_Vector , N_Vector , void *);
#endif

/* set CVode parameter */
extern int mpt_sundials_cvode_get(const MPT_SOLVER_STRUCT(cvode) *, MPT_STRUCT(property) *);
extern int mpt_sundials_cvode_set(MPT_SOLVER_STRUCT(cvode) *, const char *, MPT_INTERFACE(convertable) *);
/* set IDA solver parameter */
extern int mpt_sundials_ida_get(const MPT_SOLVER_STRUCT(ida) *, MPT_STRUCT(property) *);
extern int mpt_sundials_ida_set(MPT_SOLVER_STRUCT(ida) *, const char *, MPT_INTERFACE(convertable) *);

/* initialize/finalize Sundials CVode solver */
extern int  mpt_sundials_cvode_init(MPT_SOLVER_STRUCT(cvode) *);
extern void mpt_sundials_cvode_fini(MPT_SOLVER_STRUCT(cvode) *);
extern void mpt_sundials_cvode_reset(MPT_SOLVER_STRUCT(cvode) *);
/* initialize/finalize Sundials IDA integrator */
extern int  mpt_sundials_ida_init(MPT_SOLVER_STRUCT(ida) *);
extern void mpt_sundials_ida_fini(MPT_SOLVER_STRUCT(ida) *);
extern void mpt_sundials_ida_reset(MPT_SOLVER_STRUCT(ida) *);

/* prepare and report solver state */
extern int mpt_sundials_cvode_report(const MPT_SOLVER_STRUCT(cvode) *, int , MPT_TYPE(property_handler) , void *);
/* report solver state */
extern int mpt_sundials_ida_report(const MPT_SOLVER_STRUCT(ida) *, int , MPT_TYPE(property_handler) , void *);

/* prepare CVode */
extern int mpt_sundials_cvode_prepare(MPT_SOLVER_STRUCT(cvode) *);
/* prepare IDA */
extern int mpt_sundials_ida_prepare(MPT_SOLVER_STRUCT(ida) *);

/* temporary data for IDA */
extern void *mpt_sundials_ida_tmp(MPT_SOLVER_STRUCT(ida) *, size_t , size_t);

/* execute CVode step (cvdata, data, tend) */
extern int mpt_sundials_cvode_step(MPT_SOLVER_STRUCT(cvode) *, double);
/* execute IDA step(solver, x, tend) */
extern int mpt_sundials_ida_step(MPT_SOLVER_STRUCT(ida) *, double);

#ifdef _SUNDIALS_TYPES_H
/* create Sundials NVector with data */
# if SUNDIALS_VERSION_MAJOR >= 6
extern N_Vector mpt_sundials_nvector(sunindextype, SUNContext);
# else
extern N_Vector mpt_sundials_nvector(sunindextype);
# endif
/* setup linear solver */
extern int mpt_sundials_linear(MPT_SOLVER_STRUCT(sundials) *, sunindextype);
#endif /* _SUNDIALS_TYPES_H */

#ifndef __cplusplus
extern MPT_INTERFACE(metatype) *mpt_sundials_cvode(void);
extern MPT_INTERFACE(metatype) *mpt_sundials_ida(void);
#endif

/* wrappers to create/destroy opaque CVode solver data */
MPT_SOLVER_STRUCT(cvode) *_mpt_sundials_cvode_create(const MPT_IVP_STRUCT(odefcn) *);
const MPT_IVP_STRUCT(parameters) *_mpt_sundials_cvode_parameters(const MPT_SOLVER_STRUCT(cvode) *);
void _mpt_sundials_cvode_destroy(MPT_SOLVER_STRUCT(cvode) *);

/* wrappers to create/destroy opaque IDA solver data */
MPT_SOLVER_STRUCT(ida) *_mpt_sundials_ida_create(const MPT_IVP_STRUCT(daefcn) *);
const MPT_IVP_STRUCT(parameters) *_mpt_sundials_ida_parameters(const MPT_SOLVER_STRUCT(ida) *);
void _mpt_sundials_ida_destroy(MPT_SOLVER_STRUCT(ida) *);

/* setup Sundials jacobian parameters */
extern int mpt_sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *, MPT_INTERFACE(convertable) *);
/* output Sundials jacobian parameters */
extern int mpt_sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *, MPT_TYPE(property_handler) , void *);

#ifdef _SUNDIALS_TYPES_H
# ifndef _SUNDIALS_GENERIC_TYPE
#  define _SUNDIALS_GENERIC_TYPE(x) x
# endif
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
#endif /* _SUNDIALS_TYPES_H */

__MPT_EXTDECL_END


#ifdef __cplusplus
# ifdef _SUNDIALS_TYPES_H
inline sundials::sundials()  { mpt_sundials_init(this); }
inline sundials::~sundials() { mpt_sundials_fini(this); }

inline cvode::cvode()  { mpt_sundials_cvode_init(this); }
inline cvode::~cvode() { mpt_sundials_cvode_fini(this); }

inline ida::ida()  { mpt_sundials_ida_init(this); }
inline ida::~ida() { mpt_sundials_ida_fini(this); }
# endif /* _SUNDIALS_TYPES_H */

class CVode : public IVP
{
public:
	CVode() : _fcn(0)
	{
		_cvode = _mpt_sundials_cvode_create(&_fcn);
	}
	~CVode() __MPT_OVERRIDE
	{
		_mpt_sundials_cvode_destroy(_cvode);
	}
	/* object operations */
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_sundials_cvode_get(_cvode, pr);
	}
	int set_property(const char *pr, convertable *src) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_sundials_cvode_prepare(_cvode);
		}
		return mpt_sundials_cvode_set(_cvode, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return mpt_sundials_cvode_step(_cvode, _t);
	}
	int report(int what, property_handler_t out, void *opar) __MPT_OVERRIDE
	{
		return mpt_sundials_cvode_report(_cvode, what, out, opar);
	}
	int set_functions(int type, const void *ptr) __MPT_OVERRIDE
	{
		const parameters *ivp = _mpt_sundials_cvode_parameters(_cvode);
		return mpt_solver_module_ufcn_ode(ivp->pint, &_fcn, type, ptr);
	}
protected:
	struct odefcn _fcn;
	cvode *_cvode;
};

class IDA : public IVP
{
public:
	IDA() : _fcn(0)
	{
		_ida = _mpt_sundials_ida_create(&_fcn);
	}
	~IDA() __MPT_OVERRIDE
	{
		_mpt_sundials_ida_destroy(_ida);
	}
	/* object operations */
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return mpt_sundials_ida_get(_ida, pr);
	}
	int set_property(const char *pr, convertable *src) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return mpt_sundials_ida_prepare(_ida);
		}
		return mpt_sundials_ida_set(_ida, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return mpt_sundials_ida_step(_ida, _t);
	}
	int report(int what, property_handler_t out, void *opar) __MPT_OVERRIDE
	{
		return mpt_sundials_ida_report(_ida, what, out, opar);
	}
	int set_functions(int type, const void *ptr) __MPT_OVERRIDE
	{
		const parameters *ivp = _mpt_sundials_ida_parameters(_ida);
		return mpt_solver_module_ufcn_dae(ivp->pint, &_fcn, type, ptr);
	}
protected:
	struct daefcn _fcn;
	ida *_ida;
};

#endif

__MPT_SOLVER_END

#endif /* _MPT_SUNDIALS_H */

