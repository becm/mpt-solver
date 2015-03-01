/*!
 * interface to SUNDIALS solver family
 */

#ifndef _MPT_SUNDIALS_H
#define _MPT_SUNDIALS_H 201405

#include "../solver.h"

__MPT_SOLVER_BEGIN

#ifndef _NVECTOR_H
typedef void * N_Vector;
#endif

enum MPT_ENUM(SundialsFlags) {
	/* jacobian method */
	MPT_ENUM(SundialsJacNone)    = 0x00,
	MPT_ENUM(SundialsJacDense)   = 0x01,
	MPT_ENUM(SundialsJacBand)    = 0x02,
	MPT_ENUM(SundialsJacDiag)    = 0x03,
	MPT_ENUM(SundialsJacNumeric) = 0x10,
	
	/* preconditioner type */
	MPT_ENUM(SundialsSpilsGMR)   = 0x1,
	MPT_ENUM(SundialsSpilsBCG)   = 0x2,
	MPT_ENUM(SundialsSpilsTFQMR) = 0x3,
	MPT_ENUM(SundialsSpils)      = 0xf,
	
	/* linear algebra setup */
	MPT_ENUM(SundialsDls)        = 0x10,
	MPT_ENUM(SundialsLapack)     = 0x11,
	
	/* step strategy */
	MPT_ENUM(SundialsStepNormal) = 0x0,
	MPT_ENUM(SundialsStepSingle) = 0x1
};

MPT_SOLVER_STRUCT(sundials)
{
#ifdef _cplusplus
	inline sundials() : mem(0), y(0), jacobian(0), step(0), mu(-1), ml(-1)
	{ }
	inline ~sundials()
	{ N_VSetArrayPointer(0, y); N_VDestroy(y); }
#endif
	N_Vector y;  /* output container */
	
	int8_t jacobian,  /* jacobian flags */
	       step,      /* step strategy control */
	       linalg,    /* type of linear algebra */
	       prec;      /* preconditioner mode */
	
	int32_t mu, ml;   /* band matrix parameters */
};

MPT_SOLVER_STRUCT(cvode)
{
#ifdef __cplusplus
public:
	cvode();
	~cvode();
#endif
	MPT_SOLVER_STRUCT(ivppar) ivp;  /* inherit IVP parameter */
	MPT_TYPE(dvecpar) rtol, atol;   /* tolerances */
	
	MPT_SOLVER_STRUCT(sundials) sd;    /* general sundials data */
	void *mem;  /* CVode memory block */
	
	const MPT_SOLVER_STRUCT(ivpfcn) *ufcn;
};

MPT_SOLVER_STRUCT(ida)
{
#ifdef __cplusplus
public:
	ida();
	~ida();
protected:
#endif
	MPT_SOLVER_STRUCT(ivppar) ivp;  /* inherit IVP parameter */
	MPT_TYPE(dvecpar) rtol, atol;   /* tolerances */
	
	MPT_SOLVER_STRUCT(sundials) sd; /* general sundials data */
	void *mem;    /* IDA memory block */
	
	const MPT_SOLVER_STRUCT(ivpfcn) *ufcn;
	
	N_Vector yp;  /* deviation vector */
	
	struct {
		void   *base;
		size_t  size;
	} tmp;
};

#define SUNDIALS_flags_jac(f,s)	(f = ((f) & ~0xfL) | ((s) & 0xfL))

__MPT_EXTDECL_BEGIN

/* create Sundials NVector body */
extern N_Vector sundials_nvector_empty(long);
/* calculate errors tolerances */
extern int sundials_ewtfcn(N_Vector , N_Vector , void *);


/* set CVode parameter */
extern int sundials_cvode_property(MPT_SOLVER_STRUCT(cvode) *, MPT_STRUCT(property) *, MPT_INTERFACE(source) *);
/* set IDA solver parameter */
extern int sundials_ida_property(MPT_SOLVER_STRUCT(ida) *,   MPT_STRUCT(property) *, MPT_INTERFACE(source) *);

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
extern int sundials_ida_report(const MPT_SOLVER_STRUCT(ida) *,   int , MPT_TYPE(PropertyHandler) , void *);

/* prepare CVode */
extern int sundials_cvode_prepare(MPT_SOLVER_STRUCT(cvode) *, double *);
/* prepare IDA */
extern int sundials_ida_prepare(MPT_SOLVER_STRUCT(ida) *, double *);
extern void *sundials_ida_tmp(MPT_SOLVER_STRUCT(ida) *, size_t , size_t);

/* execute CVode step (cvdata, data, tend) */
extern int sundials_cvode_step (MPT_SOLVER_STRUCT(cvode) *, double *, double);
extern int sundials_cvode_step_(MPT_SOLVER_STRUCT(cvode) *, double *, double *);
/* execute IDA step(solver, x, tend) */
extern int sundials_ida_step (MPT_SOLVER_STRUCT(ida) *, double *, double);
extern int sundials_ida_step_(MPT_SOLVER_STRUCT(ida) *, double *, double *);

#ifndef _cplusplus
/* setup Sundials CVode solver */
extern MPT_SOLVER_INTERFACE *sundials_cvode_create(void);
extern MPT_SOLVER_INTERFACE *sundials_ida_create(void);
#endif

/* setup Sundials jacobian parameters */
extern int sundials_jacobian(MPT_SOLVER_STRUCT(sundials) *, int , MPT_INTERFACE(source) *);
/* output Sundials jacobian parameters */
extern int sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *, MPT_TYPE(PropertyHandler) out, void *usr);

#ifdef _SUNDIALSTYPES_H
/* wrapper for CVode user functions */
extern int sundials_cvode_fcn(realtype , N_Vector , N_Vector , void *);
/* wrapper for IDA user functions */
extern int sundials_ida_fcn(realtype , N_Vector , N_Vector , N_Vector , void *);

# ifdef _SUNDIALS_DIRECT_H
/* Dense/Banded wrapper for CVode jacobian */
extern int sundials_cvode_jac_dense(long int , realtype ,
				    N_Vector , N_Vector ,
				    DlsMat , void *,
				    N_Vector , N_Vector , N_Vector);
extern int sundials_cvode_jac_band(long int , long int , long int , realtype ,
				   N_Vector , N_Vector ,
				   DlsMat , void *,
				   N_Vector , N_Vector , N_Vector);

/* Dense/Banded wrapper for IDA jacobian */
extern int sundials_ida_jac_dense(long int , realtype , realtype ,
				  N_Vector , N_Vector , N_Vector ,
				  DlsMat , void *,
				  N_Vector , N_Vector , N_Vector);
extern int sundials_ida_jac_band(long int , long int , long int ,
				 realtype , realtype ,
				 N_Vector , N_Vector , N_Vector ,
				 DlsMat , void *,
				 N_Vector , N_Vector , N_Vector);
# endif
#endif

__MPT_EXTDECL_END


#ifdef __cplusplus
class CVode : public Ivp, cvode
{
public:
	CVode() : _fcn(0)
	{ ufcn = &_fcn; _fcn.param = &ivp; }
	virtual ~CVode()
	{ }
	CVode *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return sundials_cvode_property(this, pr, src); }
	int report(int what, PropertyHandler out, void *opar) const
	{ return sundials_cvode_report(this, what, out, opar); }
	int step(double *u, double *end, double *)
	{ return sundials_cvode_step_(this, u, end); }
	operator ivpfcn *() const
	{ return const_cast<ivpfcn *>(&_fcn); }
protected:
	ivpfcn _fcn;
};
inline cvode::cvode()
{ sundials_cvode_init(this); }
inline cvode::~cvode()
{ sundials_cvode_fini(this); }

class IDA : public Ivp, ida
{
public:
	IDA() : _fcn(0)
	{ ufcn = &_fcn; _fcn.param = &ivp; }
	virtual ~IDA()
	{ }
	IDA *addref()
	{ return 0; }
	int unref()
	{ delete this; return 0; }
	int property(struct property *pr, source *src = 0)
	{ return sundials_ida_property(this, pr, src); }
	int report(int what, PropertyHandler out, void *opar) const
	{ return sundials_ida_report(this, what, out, opar); }
	int step(double *u, double *end, double *)
	{ return sundials_ida_step_(this, u, end); }
	operator ivpfcn *() const
	{ return const_cast<ivpfcn *>(&_fcn); }
protected:
	ivpfcn _fcn;
};
inline ida::ida()
{ sundials_ida_init(this); }
inline ida::~ida()
{ sundials_ida_fini(this); }

#endif

__MPT_SOLVER_END

#endif /* _MPT_SUNDIALS_H */

