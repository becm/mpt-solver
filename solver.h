/*!
 * MPT solver interfaces
 *  generic solver, loadable from library
 */

#ifndef _MPT_SOLVER_H
#define _MPT_SOLVER_H  @INTERFACE_VERSION@

#include "object.h"
#include "array.h"
#include "meta.h"

#include <sys/uio.h>

#ifdef __cplusplus
# include <stdlib.h>
#endif

struct rusage;
struct timeval;

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(config);
MPT_INTERFACE(client);
MPT_INTERFACE(output);
MPT_INTERFACE(iterator);
MPT_INTERFACE(reply_context);

MPT_STRUCT(event);
MPT_STRUCT(proxy);
MPT_STRUCT(node);

MPT_STRUCT(solver_data)
{
#ifdef __cplusplus
	inline solver_data() : npar(0), nval(0)
	{ }
#else
# define MPT_SOLVER_DATA_INIT { MPT_ARRAY_INIT, MPT_ARRAY_INIT,  0, 0 }
#endif
	_MPT_ARRAY_TYPE(double) param;  /* parameter matrix */
	_MPT_ARRAY_TYPE(double) val;    /* input/output (matrix) data */
	
	long npar;  /* leading dimension of parameter data */
	long nval;  /* length of value matrix row */
};

MPT_STRUCT(solver_output)
{
#ifdef __cplusplus
	inline solver_output() : _data(0), _graphic(0), _info(0)
	{ }
	inline Slice<const uint8_t> pass() const
	{
		return _pass.slice();
	}
	bool setFlags(int flg, int pos = -1)
	{
		if (pos >= 0) {
			if (pos >= _pass.length()
			    && !_pass.resize(pos + 1)) {
				return false;
			}
			return _pass.set(pos, flg);
		}
		if (!(pos = _pass.length())) {
			return false;
		}
		for (uint8_t &v : _pass) {
			v |= flg;
		}
		return true;
	}
protected:
#else
# define MPT_SOLVER_OUTPUT_INIT { 0, 0, MPT_ARRAY_INIT }
#endif
	MPT_INTERFACE(output) *_data;
	MPT_INTERFACE(output) *_graphic;
	
	_MPT_ARRAY_TYPE(uint8_t) _pass;  /*  process flags for data dimensions */
};

#ifndef __cplusplus
# ifndef __MPT_IVP_RTOL
#  define __MPT_IVP_RTOL  1e-4
# endif
# ifndef __MPT_IVP_ATOL
#  define __MPT_IVP_ATOL  1e-4
# endif
# ifndef __MPT_NLS_TOL
#  define __MPT_NLS_TOL   1e-6
# endif
# define MPT_SOLVER_STRUCT(i) struct mpt_solver_##i
# define MPT_SOLVER_TYPE(i)   MptSolver##i
# define MPT_SOLVER_ENUM(i)   MPT_SOLVER_##i
# define __MPT_SOLVER_BEGIN   __MPT_NAMESPACE_BEGIN
# define __MPT_SOLVER_END     __MPT_NAMESPACE_END
# define MPT_SOLVER(i)        struct mpt_solver_##i

# define MPT_NLS_STRUCT(i)          struct mpt_solver_nls_##i
# define MPT_SOLVER_NLS(i)          MptSolverNls##i
# define _MPT_SOLVER_NLS_TYPEDEF(i) MptSolverNls##i

# define MPT_IVP_STRUCT(i)          struct mpt_solver_ivp_##i
# define MPT_SOLVER_IVP(i)          MptSolverIvp##i
# define _MPT_SOLVER_IVP_TYPEDEF(i) MptSolverIvp##i
#else
# define MPT_SOLVER_STRUCT(i) struct i
# define MPT_SOLVER_TYPE(i)   i
# define MPT_SOLVER_ENUM(i)   i
# define __MPT_SOLVER_BEGIN   __MPT_NAMESPACE_BEGIN namespace solver {
# define __MPT_SOLVER_END     } __MPT_NAMESPACE_END
# define MPT_SOLVER(i)        class i

# define MPT_NLS_STRUCT(i)          struct NLS::i
# define MPT_SOLVER_NLS(i)          NLS::i
# define _MPT_SOLVER_NLS_TYPEDEF(i) i

# define MPT_IVP_STRUCT(i)          struct IVP::i
# define MPT_SOLVER_IVP(i)          IVP::i
# define _MPT_SOLVER_IVP_TYPEDEF(i) i

namespace solver {

#endif

enum MPT_SOLVER_ENUM(Flags)
{
	MPT_SOLVER_ENUM(CapableIvp) = 0xf,
	MPT_SOLVER_ENUM(IvpRside)   = 0x1,
	MPT_SOLVER_ENUM(IvpJac)     = 0x2,
	MPT_SOLVER_ENUM(ODE)        = 0x3,
	MPT_SOLVER_ENUM(IvpMas)     = 0x4,
	MPT_SOLVER_ENUM(DAE)        = 0x7,
	MPT_SOLVER_ENUM(PDE)        = 0x8,
	
	MPT_SOLVER_ENUM(CapableNls) = 0xf00,
	MPT_SOLVER_ENUM(NlsVector)  = 0x100,
	MPT_SOLVER_ENUM(NlsOverdet) = 0x200,
	MPT_SOLVER_ENUM(NlsUser)    = 0x300,
	MPT_SOLVER_ENUM(NlsJac)     = 0x400,
	MPT_SOLVER_ENUM(NlsOut)     = 0x800
};
enum MPT_SOLVER_ENUM(Report) {
	MPT_SOLVER_ENUM(Header)     = 0x1,
	MPT_SOLVER_ENUM(Status)     = 0x2,
	MPT_SOLVER_ENUM(Report)     = 0x4,
	MPT_SOLVER_ENUM(Values)     = 0x8
};

/*! generic solver interface */
#ifdef __cplusplus
class interface : public metatype
{
protected:
	inline ~interface() {}
public:
	virtual int report(int , PropertyHandler , void *) = 0;
	virtual int setFunctions(int , const void *) = 0;
	virtual int solve() = 0;
}; /* generic solver end */
#else
MPT_SOLVER(interface);
MPT_INTERFACE_VPTR(solver)
{
	/* metatype operations */
	MPT_INTERFACE_VPTR(metatype) meta;
	/* call output function with data to report for type */
	int (*report)(MPT_SOLVER(interface) *, int , MPT_TYPE(PropertyHandler) , void *);
	int (*setFunctions)(MPT_SOLVER(interface) *, int , const void *);
	int (*solve)(MPT_SOLVER(interface) *);
};
MPT_SOLVER(interface)
{ const MPT_INTERFACE_VPTR(solver) *_vptr; };
#endif

#ifdef __cplusplus
/*! generic solver implementation */
class generic : public object
{
public:
	virtual ~generic()
	{}
	virtual int report(int , PropertyHandler , void *) = 0;
	
	virtual bool prepare()
	{
		return setProperty(0, 0) >= 0;
	}
protected:
	virtual int setFunctions(int , const void *) = 0;
};
/*! extension for Initial Value Problems */
class IVP : public generic
{
public:
	struct parameters;
	struct rside;
	struct jacobian;
	struct odefcn;
	struct daefcn;
	struct pdefcn;
	
	virtual int step(double) = 0;
	
	inline bool set(double t, Slice<double> x)
	{
		return mpt_object_set(this, 0, "dD", t, x) >= 0;
	}
	template <typename T>
	inline bool set(const T &fcn)
	{
		return setFunctions(functionType(fcn), &fcn) >= 0;
	}
#else
MPT_IVP_STRUCT(parameters);
#endif
/* right side function */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Fcn))(void *, double t, const double *, double *);
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Pde))(void *, double t, const double *, double *, const MPT_IVP_STRUCT(parameters) *);
/* extension for ODE/DAE solvers */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Jac))(void *, double , const double *, double *, int);
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Mas))(void *, double , const double *, double *, int *, int *);
/* extensions for PDE solvers */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Rside))(void *, double , const double *, double *, double , double *, double *);
#ifdef __cplusplus
};
#endif

/*! extension for NonLinear Systems */
#ifdef __cplusplus
class NLS : public generic
{
public:
	struct parameters;
	struct residuals;
	struct jacobian;
	struct output;
	struct functions;
	
	virtual int solve() = 0;
	
	inline bool set(Slice<const double> par)
	{
		return mpt_object_set(this, 0, "D", par) >= 0;
	}
	template <typename T>
	inline bool set(const T &fcn)
	{
		return setFunctions(functionType(fcn), &fcn) >= 0;
	}
#endif
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Fcn))(void *rpar, const double *p, double *res, const int *lres);
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Jac))(void *jpar, const double *p, double *jac, const int *ld, const double *res);
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Out))(void *opar, const MPT_STRUCT(value) *val);
#ifdef __cplusplus
};
#endif


/*! general IVP parameter */
MPT_IVP_STRUCT(parameters)
{
#ifdef __cplusplus
	inline parameters(int n = 0) : neqs(n), pint(0), grid(0)
	{ }
#else
# define MPT_IVPPAR_INIT  { 1, 0, 0 }
#endif
	int32_t   neqs;  /* number of equotations */
	uint32_t  pint;  /* PDE intervals */
	double   *grid;  /* PDE grid data */
};
/*! initial value problem functions */
MPT_IVP_STRUCT(rside)
{
#ifdef __cplusplus
	inline rside(Fcn f, void *p = 0) : fcn(f), par(p)
	{ }
	enum { Type = IvpRside };
#else
# define MPT_IVP_RSIDE_INIT { 0, 0 }
#endif
	MPT_SOLVER_IVP(Fcn) fcn;  /* unified right side calculation */
	void *par;
};
/*! initial value problem functions */
MPT_IVP_STRUCT(jacobian)
{
#ifdef __cplusplus
	inline jacobian(Jac f, void *p = 0) : fcn(f), par(p)
	{ }
	enum { Type = IvpJac };
#else
# define MPT_IVP_JAC_INIT { 0, 0 }
#endif
	MPT_SOLVER_IVP(Jac) fcn;  /* (full/banded) jacobi matrix */
	void  *par;
};
/*! initial value problem functions */
MPT_IVP_STRUCT(odefcn)
{
#ifdef __cplusplus
	inline odefcn(Fcn f, void *p = 0, Jac j = 0) : rside(f, p), jac(j, p)
	{ }
	enum { Type = ODE };
#else
# define MPT_IVP_ODE_INIT  { MPT_IVP_RSIDE_INIT, MPT_IVP_JAC_INIT }
#endif
	MPT_IVP_STRUCT(rside)    rside;
	MPT_IVP_STRUCT(jacobian) jac;
};
/*! initial value problem functions */
#ifdef __cplusplus
MPT_IVP_STRUCT(daefcn) : public IVP::odefcn
{
	inline daefcn(Fcn f, void *p = 0, Jac j = 0) : IVP::odefcn(f, p, j)
	{ mas.fcn = 0; mas.par = 0; }
	enum { Type = DAE };
#else
MPT_IVP_STRUCT(daefcn)
{
# define MPT_IVP_DAE_INIT  { MPT_IVP_RSIDE_INIT, MPT_IVP_JAC_INIT, { 0, 0 } }
	MPT_IVP_STRUCT(rside)    rside;
	MPT_IVP_STRUCT(jacobian) jac;
#endif
	struct {
		MPT_SOLVER_IVP(Mas) fcn;  /* (sparse) mass matrix */
		void *par;
	} mas;
};
MPT_IVP_STRUCT(pdefcn)
{
#ifdef __cplusplus
	inline pdefcn(Pde f, void *p = 0) :fcn(f), par(p)
	{ }
	enum { Type = IvpRside | PDE };
#else
# define MPT_IVP_PDE_INIT  { 0, 0 }
#endif
	MPT_SOLVER_IVP(Pde) fcn;  /* (sparse) mass matrix */
	void *par;
};

/*! general nonlinear system parameter */
MPT_NLS_STRUCT(parameters)
{
#ifdef __cplusplus
	inline parameters() : nval(0), nres(0)
	{ }
#else
# define MPT_NLSPAR_INIT  { 0, 0 }
#endif
	int32_t nval,  /* parameters to optimize */
	        nres;  /* number of residuals */
};
/*! generic nonlinear system functions */
MPT_NLS_STRUCT(residuals)
{
#ifdef __cplusplus
	inline residuals(Fcn f, void *p = 0) : fcn(f), par(p)
	{ }
#else
# define MPT_NLS_RES_INIT  { 0, 0 }
#endif
	MPT_SOLVER_NLS(Fcn) fcn;  /* calculate residuals */
	void *par;
};
/*! generic nonlinear system functions */
MPT_NLS_STRUCT(jacobian)
{
#ifdef __cplusplus
	inline jacobian(Jac f, void *p = 0) : fcn(f), par(p)
	{ }
	enum { Type = NlsJac };
#else
# define MPT_NLS_JAC_INIT  { 0, 0 }
#endif
	MPT_SOLVER_NLS(Jac) fcn;  /* calculate jacobian */
	void *par;
};
/*! generic nonlinear system functions */
MPT_NLS_STRUCT(functions)
{
#ifdef __cplusplus
	inline functions(Fcn f, void *u = 0, Jac j = 0) : res(f, u), jac(j, u)
	{ }
	enum { Type = NlsUser | NlsJac };
#else
# define MPT_NLSFCN_INIT  { { 0, 0 }, { 0, 0 } }
#endif
	MPT_NLS_STRUCT(residuals) res;
	MPT_NLS_STRUCT(jacobian)  jac;
};

MPT_NLS_STRUCT(output)
{
#ifdef __cplusplus
	inline output(Out f, void *u = 0) : fcn(f), par(u)
	{ }
	enum { Type = NlsOut };
#else
# define MPT_NLSOUT_INIT(r, p)  { (r), (p) }
#endif
	MPT_SOLVER_NLS(Out) fcn;  /* output current parameter/residuals */
	void *par;
};

#ifdef __cplusplus
inline __MPT_CONST_EXPR int functionType(const IVP::rside &)    { return IVP::rside::Type; }
inline __MPT_CONST_EXPR int functionType(const IVP::jacobian &) { return IVP::jacobian::Type; }
inline __MPT_CONST_EXPR int functionType(const IVP::odefcn &)   { return IVP::odefcn::Type; }
inline __MPT_CONST_EXPR int functionType(const IVP::daefcn &)   { return IVP::daefcn::Type; }
inline __MPT_CONST_EXPR int functionType(const IVP::pdefcn &)   { return IVP::pdefcn::Type; }

inline __MPT_CONST_EXPR int functionType(const NLS::jacobian &)  { return NLS::jacobian::Type; }
inline __MPT_CONST_EXPR int functionType(const NLS::functions &) { return NLS::functions::Type; }
inline __MPT_CONST_EXPR int functionType(const NLS::output &)    { return NLS::output::Type; }


template <typename T>
struct vecpar
{
	typedef T* iterator;
	
	inline vecpar(T const &val) : base(0)
	{ d.val = val; }
	inline vecpar() : base(0)
	{ d.val = T(); }
	inline ~vecpar()
	{ if (base) resize(0); }
	
	inline iterator begin() const
	{ return base ? base : const_cast<T *>(&d.val); }
	
	 inline iterator end() const
	{ return base ? base + d.len/sizeof(*base) : const_cast<T *>(&d.val) + 1; }
	
	inline Slice<const T> data() const
	{
		if (base) return Slice<const T>(base, d.len / sizeof(T));
		return Slice<const T>(&d.val, 1);
	}
	inline int type()
	{ return typeIdentifier<T>(); }
	
	bool resize(size_t elem) {
		T *t = base;
		size_t l = t ? d.len/sizeof(*base) : 0;
		
		if (l == elem) return true; /* noop */
		
		/* save/destruct vector data */
		if (!elem && t) d.val = t[0]; /* vector>scalar transition */
		for (size_t i = elem; i < l; ++i) t[i].~T();
		
		/* free/extend memory */
		if (!elem) { free(t); t = 0; }
		else if (!(t = static_cast<T *>(realloc(t, elem * sizeof(*base))))) return false;
		
		/* initialize new elements */
		for (T &val = base ? t[l - 1] : d.val; l < elem; ++l) t[l] = val;
		
		/* save new data */
		if (elem) {
			if (!base) d.val.~T(); /* scalar>vector transition */
			d.len = elem * sizeof(*base);
		}
		base = t;
		
		return true;
	}
	T &operator [](int pos)
	{
		static T def;
		int len = base ? d.len / sizeof(T) : 1;
		if (pos < 0) pos += len;
		if (!pos && !base) return d.val;
		return (pos >= 0 && pos < len) ? base[pos] : def;
	}
	struct value value() const
	{
		struct value v;
		if (base) {
			static const char fmt[2] = { static_cast<char>(vectorIdentifier<T>()), 0 };
			v.fmt = fmt;
			v.ptr = this;
		} else {
			static const char fmt[2] = { static_cast<char>(typeIdentifier<T>()), 0 };
			v.fmt = fmt;
			v.ptr = &d.val;
		}
		return v;
	}
protected:
	T *base;
	union {
		size_t len;
		T val;
	} d;
private:
	vecpar & operator =(const vecpar &);
};

# if __cplusplus >= 201103L
using dvecpar = vecpar<double>;
using ivecpar = vecpar<int>;
# else
typedef vecpar<double> dvecpar;
typedef vecpar<int> ivecpar;
# endif
#else
# define MPT_VECPAR_INIT(p,v)  ((p)->base = 0, (p)->d.val = (v))
typedef struct
{
	double *base;
	union { size_t len; double val; } d;
} MPT_SOLVER_TYPE(dvecpar);
typedef struct
{
	int *base;
	union { size_t len; int val; } d;
} MPT_SOLVER_TYPE(ivecpar);
#endif

typedef int (MPT_SOLVER_TYPE(UserInit))(MPT_SOLVER(interface) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *);


__MPT_EXTDECL_BEGIN


/* create interface to solver */
extern MPT_INTERFACE(client) *mpt_client_ivp(MPT_SOLVER_TYPE(UserInit) *);
extern MPT_INTERFACE(client) *mpt_client_nls(MPT_SOLVER_TYPE(UserInit) *);


/* initialize IVP solver states */
extern int mpt_init_ivp(MPT_INTERFACE(object) *, const _MPT_ARRAY_TYPE(double) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

extern int mpt_init_dae(MPT_SOLVER(interface) *, const MPT_IVP_STRUCT(daefcn) *, int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_init_ode(MPT_SOLVER(interface) *, const MPT_IVP_STRUCT(odefcn) *, int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_init_pde(MPT_SOLVER(interface) *, const MPT_IVP_STRUCT(pdefcn) *, int , const _MPT_ARRAY_TYPE(double) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* initialize NLS solver states */
extern int mpt_init_nls(MPT_SOLVER(interface) *, const MPT_NLS_STRUCT(functions) *, const MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));


/* register events on notifier */
extern int mpt_solver_dispatch(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);

/* get input for required config elements */
extern int mpt_solver_require(MPT_INTERFACE(config) *, MPT_INTERFACE(reply_context) *);

/* set solver (user data and solver parameter) config files */
extern int mpt_solver_config(MPT_INTERFACE(object) *, MPT_INTERFACE(iterator) *, MPT_INTERFACE(logger) *);

/* read files to configuration */
extern int mpt_solver_read(MPT_STRUCT(node) *, MPT_INTERFACE(iterator) *, MPT_INTERFACE(logger) *);

/* initialize solver memory */
extern void mpt_solver_data_fini(MPT_STRUCT(solver_data) *);
extern void mpt_solver_data_clear(MPT_STRUCT(solver_data) *);

/* copy state to solver data */
extern int mpt_solver_data_nls(MPT_STRUCT(solver_data) *, const MPT_STRUCT(value) *);

/* add usage time difference */
extern void mpt_timeradd_sys(struct timeval *, const struct rusage *, const struct rusage *);
extern void mpt_timeradd_usr(struct timeval *, const struct rusage *, const struct rusage *);


/* set problem specific parameters */
extern int mpt_conf_nls(MPT_STRUCT(solver_data) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_conf_pde(MPT_STRUCT(solver_data) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_conf_ode(MPT_STRUCT(solver_data) *, double , const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* configure graphic output and bindings */
extern int mpt_conf_graphic(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *);
/* configure history output and format */
extern int mpt_conf_history(MPT_INTERFACE(object) *, const MPT_STRUCT(node) *);

/* create profile data */
extern MPT_INTERFACE(metatype) *mpt_conf_profiles(const MPT_STRUCT(solver_data) *, double , const MPT_STRUCT(node) *, int , MPT_INTERFACE(logger) *);

/* append user data */
extern int mpt_conf_param(MPT_STRUCT(array) *, const MPT_STRUCT(node) *, int);
/* set grid data */
extern int mpt_conf_grid(MPT_STRUCT(array) *, const MPT_INTERFACE(metatype) *);


/* get/initialize solver data parameters */
extern double *mpt_solver_data_grid (MPT_STRUCT(solver_data) *);
extern double *mpt_solver_data_param(MPT_STRUCT(solver_data) *);

/* execute specific IVP solver step */
extern int mpt_steps_ode(MPT_SOLVER(interface) *, MPT_INTERFACE(iterator) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* inner node residuals with central differences */
extern int mpt_residuals_cdiff(void *, double , const double *, double *, const MPT_IVP_STRUCT(parameters) *, MPT_SOLVER_IVP(Rside));

/* generate library description form short form */
extern const char *mpt_solver_alias(const char *);
/* load solver of specific type */
extern MPT_SOLVER(interface) *mpt_solver_load(MPT_STRUCT(proxy) *, int , const char *, MPT_INTERFACE(logger) *);

/* set solver parameter */
extern void mpt_solver_pset(MPT_INTERFACE(object) *, const MPT_STRUCT(node) *, int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_solver_param(MPT_INTERFACE(object) *, MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
/* set solver parameter to value */
extern int mpt_solver_setvalue(MPT_INTERFACE(object) *, const char *, double);

/* generic solver output */
extern int mpt_solver_info  (MPT_SOLVER(interface) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_solver_status(MPT_SOLVER(interface) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()), int (*)(void *, const MPT_STRUCT(value) *) __MPT_DEFPAR(0), void *__MPT_DEFPAR(0));
extern int mpt_solver_report(MPT_SOLVER(interface) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
/* final solver statistics */
extern void mpt_solver_statistics(MPT_SOLVER(interface) *, MPT_INTERFACE(logger) *, const struct timeval *, const struct timeval *);


/* output for solvers */
extern int mpt_solver_output_nls(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(value) *, const MPT_STRUCT(solver_data) *);
extern int mpt_solver_output_pde(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(value) *, const MPT_STRUCT(solver_data) *);
extern int mpt_solver_output_ode(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(solver_data) *);
/* select solver log target */
extern int mpt_solver_output_query(MPT_STRUCT(solver_output) *, const MPT_INTERFACE(config) *);

/* push data to output */
extern int mpt_output_solver_data(MPT_INTERFACE(output) *, int , int , int , const double *, int);
extern int mpt_output_ivp_header(MPT_INTERFACE(output) *, int , int , const double *);
extern int mpt_output_solver_history(MPT_INTERFACE(output) *, const double *, int , const double *, int);


/* solver module tolerance handling */
extern int mpt_solver_module_tol_set(MPT_SOLVER_TYPE(dvecpar) *, const MPT_INTERFACE(metatype) *, double);
extern int mpt_solver_module_tol_get(const MPT_SOLVER_TYPE(dvecpar) *, MPT_STRUCT(value) *);
extern int mpt_solver_module_tol_check(MPT_SOLVER_TYPE(dvecpar) *, long , long , double);

/* solver module memory management */
extern void *mpt_solver_module_valloc(struct iovec *, size_t len, size_t size);

/* solver module parameter assignment */
extern int mpt_solver_module_ivpset(MPT_IVP_STRUCT(parameters) *, const MPT_INTERFACE(metatype) *);
extern int mpt_solver_module_nlsset(MPT_NLS_STRUCT(parameters) *, const MPT_INTERFACE(metatype) *);

/* solver module generic type conversion */
extern int mpt_solver_module_nextval(double *, double , const MPT_INTERFACE(metatype) *);

/* assign user functions */
extern int mpt_solver_module_ufcn_ode(long , MPT_IVP_STRUCT(odefcn) *, int , const void *);
extern int mpt_solver_module_ufcn_dae(long , MPT_IVP_STRUCT(daefcn) *, int , const void *);
extern int mpt_solver_module_ufcn_nls(const MPT_NLS_STRUCT(parameters) *, MPT_NLS_STRUCT(functions) *, int , const void *);


/* id for registered input metatype */
extern int mpt_solver_typeid(void);

__MPT_EXTDECL_END

#ifdef __cplusplus
template <class T>
class Solver : public interface, public T
{
public:
	virtual ~Solver()
	{ }
	void unref() __MPT_OVERRIDE
	{
		delete this;
	}
	int conv(int type, void *ptr) const __MPT_OVERRIDE
	{
		int me = mpt_solver_typeid();
		if (me < 0) {
			me = metatype::Type;
		}
		if (!type) {
			static const char fmt[] = { metatype::Type, object::Type, 0 };
			if (ptr) *static_cast<const char **>(ptr) = fmt;
			return me;
		}
		if (type == metatype::Type) {
			if (ptr) *static_cast<const metatype **>(ptr) = this;
			return object::Type;
		}
		if (type == object::Type) {
			if (ptr) *static_cast<const object **>(ptr) = this;
			return me;
		}
		if (type == me) {
			if (ptr) *static_cast<const interface **>(ptr) = this;
			return object::Type;
		}
		return BadType;
	}
	int report(int what, PropertyHandler prop, void *ptr) __MPT_OVERRIDE
	{
		if (!what && !prop && !ptr) {
			return T::property(0);
		}
		return T::report(what, prop, ptr);
	}
	int setFunctions(int what, const void *ptr) __MPT_OVERRIDE
	{
		return T::setFunctions(what, ptr);
	}
};
} /* namespace solver */
#endif

__MPT_NAMESPACE_END

#ifdef __cplusplus
template <typename T>
inline std::ostream &operator<<(std::ostream &o, mpt::solver::vecpar<T> &d)
{
	return o << d.data();
}
#endif

#endif /* _MPT_SOLVER_H */
