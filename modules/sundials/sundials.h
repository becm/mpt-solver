/*!
 * interface to SUNDIALS solver family
 */

#ifndef _MPT_SUNDIALS_H
#define _MPT_SUNDIALS_H  @INTERFACE_VERSION@

#include "../solver.h"

#ifdef __cplusplus
# include <sundials/sundials_nvector.h>
#endif

__MPT_SOLVER_BEGIN

#ifndef _NVECTOR_H
typedef void * N_Vector;
#endif

enum MPT_SOLVER_ENUM(SundialsFlags) {
	/* jacobian method */
	MPT_SOLVER_ENUM(SundialsJacNone)    = 0x00,
	MPT_SOLVER_ENUM(SundialsJacDense)   = 0x01,
	MPT_SOLVER_ENUM(SundialsJacBand)    = 0x02,
	MPT_SOLVER_ENUM(SundialsJacDiag)    = 0x03,
	MPT_SOLVER_ENUM(SundialsJacNumeric) = 0x10,
	
	/* preconditioner type */
	MPT_SOLVER_ENUM(SundialsSpilsGMR)   = 0x1,
	MPT_SOLVER_ENUM(SundialsSpilsBCG)   = 0x2,
	MPT_SOLVER_ENUM(SundialsSpilsTFQMR) = 0x3,
	MPT_SOLVER_ENUM(SundialsSpils)      = 0xf,
	
	/* linear algebra setup */
	MPT_SOLVER_ENUM(SundialsDls)        = 0x10,
	MPT_SOLVER_ENUM(SundialsLapack)     = 0x11,
	
#ifdef _SUNDIALSTYPES_H
# if defined(SUNDIALS_SINGLE_PRECISION)
	MPT_SOLVER_ENUM(SundialsRealtype)   = 'f',
# elif defined(SUNDIALS_DOUBLE_PRECISION)
	MPT_SOLVER_ENUM(SundialsRealtype)   = 'd',
# elif defined(SUNDIALS_EXTENDED_PRECISION)
	MPT_SOLVER_ENUM(SundialsRealtype)   = 'e',
# endif
#endif
	/* step strategy */
	MPT_SOLVER_ENUM(SundialsStepNormal) = 0x0,
	MPT_SOLVER_ENUM(SundialsStepSingle) = 0x1
};

MPT_SOLVER_STRUCT(sundials)
{
#ifdef _cplusplus
	inline sundials() : mem(0), y(0), jacobian(0), step(0), mu(-1), ml(-1)
	{ }
	inline ~sundials()
	{ N_VDestroy(y); }
#endif
	N_Vector y;  /* output container */
	
	int8_t jacobian,  /* jacobian flags */
	       step,      /* step strategy control */
	       linalg,    /* type of linear algebra */
	       prec;      /* preconditioner mode */
	
	int32_t mu, ml;   /* band matrix parameters */
};

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

/* create Sundials NVector with data */
extern N_Vector sundials_nvector_new(long);
/* calculate errors tolerances */
extern int sundials_ewtfcn(N_Vector , N_Vector , void *);

/* initialize values */
extern int sundials_vector_set(N_Vector *, long , long , MPT_INTERFACE(iterator) *);

/* set CVode parameter */
extern int sundials_cvode_get(const MPT_SOLVER_STRUCT(cvode) *, MPT_STRUCT(property) *);
extern int sundials_cvode_set(MPT_SOLVER_STRUCT(cvode) *, const char *, const MPT_INTERFACE(metatype) *);
/* set IDA solver parameter */
extern int sundials_ida_get(const MPT_SOLVER_STRUCT(ida) *, MPT_STRUCT(property) *);
extern int sundials_ida_set(MPT_SOLVER_STRUCT(ida) *, const char *, const MPT_INTERFACE(metatype) *);

/* initialize/finalize Sundials CVode solver */
extern int  sundials_cvode_init(MPT_SOLVER_STRUCT(cvode) *);
extern void sundials_cvode_fini(MPT_SOLVER_STRUCT(cvode) *);
extern void sundials_cvode_reset(MPT_SOLVER_STRUCT(cvode) *);
/* initialize/finalize Sundials IDA integrator */
extern int  sundials_ida_init(MPT_SOLVER_STRUCT(ida) *);
extern void sundials_ida_fini(MPT_SOLVER_STRUCT(ida) *);
extern void sundials_ida_reset(MPT_SOLVER_STRUCT(ida) *);

/* prepare and report solver state */
extern int sundials_cvode_report(const MPT_SOLVER_STRUCT(cvode) *, int , MPT_TYPE(PropertyHandler) , void *);
/* report solver state */
extern int sundials_ida_report(const MPT_SOLVER_STRUCT(ida) *, int , MPT_TYPE(PropertyHandler) , void *);

/* prepare CVode */
extern int sundials_cvode_prepare(MPT_SOLVER_STRUCT(cvode) *);
/* prepare IDA */
extern int sundials_ida_prepare(MPT_SOLVER_STRUCT(ida) *);

/* temporary data for IDA */
extern void *sundials_ida_tmp(MPT_SOLVER_STRUCT(ida) *, size_t , size_t);

#ifdef _SUNDIALSTYPES_H
/* execute CVode step (cvdata, data, tend) */
extern int sundials_cvode_step(MPT_SOLVER_STRUCT(cvode) *, realtype);
/* execute IDA step(solver, x, tend) */
extern int sundials_ida_step(MPT_SOLVER_STRUCT(ida) *, realtype);
#endif

#ifndef __cplusplus
extern MPT_INTERFACE(metatype) *sundials_cvode_create(void);
extern MPT_INTERFACE(metatype) *sundials_ida_create(void);
#endif

/* setup Sundials jacobian parameters */
extern int sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *, long , const MPT_INTERFACE(metatype) *);
/* output Sundials jacobian parameters */
extern int sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *, MPT_TYPE(PropertyHandler) out, void *usr);

#ifndef _SUNDIALS_GENERIC_TYPE
# define _SUNDIALS_GENERIC_TYPE(x) x
#endif

#ifdef _SUNDIALSTYPES_H
/* wrapper for CVode user functions */
extern int sundials_cvode_fcn(realtype , N_Vector , N_Vector , _SUNDIALS_GENERIC_TYPE(const MPT_SOLVER_STRUCT(cvode)) *);
/* wrapper for IDA user functions */
extern int sundials_ida_fcn(realtype , N_Vector , N_Vector , N_Vector , _SUNDIALS_GENERIC_TYPE(MPT_SOLVER_STRUCT(ida)) *);

# ifdef _SUNDIALS_DIRECT_H
/* Dense/Banded wrapper for CVode jacobian */
extern int sundials_cvode_jac_dense(long int , realtype ,
                                    N_Vector , N_Vector ,
                                    DlsMat , _SUNDIALS_GENERIC_TYPE(const MPT_SOLVER_STRUCT(cvode)) *,
                                    N_Vector , N_Vector , N_Vector);
extern int sundials_cvode_jac_band(long int , long int , long int , realtype ,
                                   N_Vector , N_Vector ,
                                   DlsMat , _SUNDIALS_GENERIC_TYPE(const MPT_SOLVER_STRUCT(cvode)) *,
                                   N_Vector , N_Vector , N_Vector);

/* Dense/Banded wrapper for IDA jacobian */
extern int sundials_ida_jac_dense(long int , realtype , realtype ,
                                  N_Vector , N_Vector , N_Vector ,
                                  DlsMat , _SUNDIALS_GENERIC_TYPE(MPT_SOLVER_STRUCT(ida)) *,
                                  N_Vector , N_Vector , N_Vector);
extern int sundials_ida_jac_band(long int , long int , long int ,
                                 realtype , realtype ,
                                 N_Vector , N_Vector , N_Vector ,
                                 DlsMat , _SUNDIALS_GENERIC_TYPE(MPT_SOLVER_STRUCT(ida)) *,
                                 N_Vector , N_Vector , N_Vector);
# endif
#endif

__MPT_EXTDECL_END


#ifdef __cplusplus
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
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return sundials_cvode_get(this, pr);
	}
	int setProperty(const char *pr, const metatype *src) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return sundials_cvode_prepare(this);
		}
		return sundials_cvode_set(this, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return sundials_cvode_step(this, _t);
	}
	int report(int what, PropertyHandler out, void *opar) __MPT_OVERRIDE
	{
		return sundials_cvode_report(this, what, out, opar);
	}
	int setFunctions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return mpt_solver_module_ufcn_ode(ivp.pint, &_fcn, type, ptr);
	}
protected:
	struct odefcn _fcn;
};
inline cvode::cvode()
{ sundials_cvode_init(this); }
inline cvode::~cvode()
{ sundials_cvode_fini(this); }

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
	int property(struct property *pr) const __MPT_OVERRIDE
	{
		return sundials_ida_get(this, pr);
	}
	int setProperty(const char *pr, const metatype *src) __MPT_OVERRIDE
	{
		if (!pr && !src) {
			return sundials_ida_prepare(this);
		}
		return sundials_ida_set(this, pr, src);
	}
	/* solver operations */
	int solve() __MPT_OVERRIDE
	{
		return sundials_ida_step(this, _t);
	}
	int report(int what, PropertyHandler out, void *opar) __MPT_OVERRIDE
	{
		return sundials_ida_report(this, what, out, opar);
	}
	int setFunctions(int type, const void *ptr) __MPT_OVERRIDE
	{
		return mpt_solver_module_ufcn_dae(ivp.pint, &_fcn, type, ptr);
	}
protected:
	struct daefcn _fcn;
};
inline ida::ida()
{ sundials_ida_init(this); }
inline ida::~ida()
{ sundials_ida_fini(this); }

#endif

__MPT_SOLVER_END

#endif /* _MPT_SUNDIALS_H */

