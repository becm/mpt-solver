/*!
 * MPT solver interfaces
 *  generic solver, loadable from library
 */

#ifndef _MPT_SOLVER_H
#define _MPT_SOLVER_H  @INTERFACE_VERSION@

#include "types.h"
#include "object.h"
#include "array.h"

#ifdef __cplusplus
# include <cstdlib>
#endif

struct rusage;
struct timeval;
struct iovec;

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(config);
MPT_INTERFACE(client);
MPT_INTERFACE(output);
MPT_INTERFACE(iterator);

MPT_STRUCT(event);
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
	inline solver_output() : _data(0), _graphic(0)
	{ }
	inline span<const uint8_t> pass() const
	{
		return _pass.elements();
	}
	bool set_flags(int flg, int pos = -1)
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

enum MPT_SOLVER_ENUM(Functions)
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
class interface
{
protected:
	inline ~interface() {}
public:
	virtual int report(int , property_handler_t , void *) = 0;
	virtual int set_functions(int , const void *) = 0;
	virtual int solve() = 0;
};
#else
MPT_SOLVER(interface);
MPT_INTERFACE_VPTR(solver)
{
	int (*report)(MPT_SOLVER(interface) *, int , MPT_TYPE(property_handler) , void *);
	int (*set_functions)(MPT_SOLVER(interface) *, int , const void *);
	int (*solve)(MPT_SOLVER(interface) *);
}; MPT_SOLVER(interface) {
	const MPT_INTERFACE_VPTR(solver) *_vptr;
};
#endif

#ifdef __cplusplus
class generic : public object, public interface
{
public:
	virtual ~generic()
	{ }
	
	virtual bool prepare()
	{
		return set_property(0, 0) >= 0;
	}
	
	template <typename T>
	inline bool set(const T &fcn)
	{
		return set_functions(T::FunctionType, &fcn) >= 0;
	}
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
	
	inline IVP()
	{ }
	
	int step(double t)
	{
		value val;
		val = t;
		int ret = mpt_object_set_value(this, "t", &val);
		return ret < 0 ? ret : solve();
	}
	inline bool set(double t, span<const double> x)
	{
		return mpt_object_set(this, 0, "dD", t, x);
	}
	template <typename T>
	inline bool set(const T &fcn)
	{
		return generic::set(fcn);
	}
#else
MPT_IVP_STRUCT(parameters);
#endif
/* right side function */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Fcn))(void *, double , const double *, double *);
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Pde))(void *, double , const double *, double *, const MPT_IVP_STRUCT(parameters) *);
/* extension for ODE/DAE solvers */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Jac))(void *, double , const double *, double *, int);
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Mas))(void *, double , const double *, double *, int *, int *);
/* extensions for PDE solvers */
typedef int (*_MPT_SOLVER_IVP_TYPEDEF(Rside))(void *, double , const double *, double *, double , double *, double *);
#ifdef __cplusplus
protected:
	inline bool _is_time_property(const char *id)
	{
		return id && id[0] == 't' && id[1] == 0;
	}
};
#endif

/*! extension for NonLinear Systems */
#ifdef __cplusplus
class NLS : public generic
{
public:
	virtual ~NLS()
	{ }
	
	struct parameters;
	struct residuals;
	struct jacobian;
	struct output;
	struct functions;
	
	inline bool set(span<const double> par)
	{
		return mpt_object_set(this, 0, "D", par) >= 0;
	}
	template <typename T>
	inline bool set(const T &fcn)
	{
		return generic::set(fcn);
	}
#endif
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Fcn))(void *rpar, const double *p, double *res, const int *lres);
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Jac))(void *jpar, const double *p, double *jac, const int *ld, const double *res);
typedef int (*_MPT_SOLVER_NLS_TYPEDEF(Out))(void *opar, const MPT_STRUCT(value) *par, const MPT_STRUCT(value) *res);
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
	enum { FunctionType = IvpRside };
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
	enum { FunctionType = IvpJac };
#else
# define MPT_IVP_JAC_INIT { 0, 0 }
#endif
	MPT_SOLVER_IVP(Jac) fcn;  /* (full/banded) jacobi matrix */
	void *par;
};
/*! initial value problem functions */
MPT_IVP_STRUCT(odefcn)
{
#ifdef __cplusplus
	inline odefcn(Fcn f, void *p = 0, Jac j = 0) : rside(f, p), jac(j, p)
	{ }
	enum { FunctionType = ODE };
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
	{
		mas.fcn = 0;
		mas.par = 0;
	}
	enum { FunctionType = DAE };
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
	inline pdefcn(Pde f, void *p = 0) : fcn(f), par(p)
	{ }
	enum { FunctionType = IvpRside | PDE };
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
	enum { FunctionType = NlsJac };
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
	enum { FunctionType = NlsUser | NlsJac };
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
	enum { FunctionType = NlsOut };
#else
# define MPT_NLSOUT_INIT(r, p)  { (r), (p) }
#endif
	MPT_SOLVER_NLS(Out) fcn;  /* output current parameter/residuals */
	void *par;
};

#ifdef __cplusplus
template <typename T>
struct vecpar
{
	typedef T* iterator;
	
	inline vecpar(T const &val) : _base(0)
	{
		_d.val = val;
	}
	inline vecpar() : _base(0)
	{
		_d.val = T();
	}
	inline ~vecpar()
	{
		if (_base) resize(0);
	}
	inline iterator begin() const
	{
		return _base ? _base : const_cast<T *>(&_d.val);
	}
	inline iterator end() const
	{
		if (_base) {
			return _base + _d.len / sizeof(*_base);
		}
		return const_cast<T *>(&_d.val) + 1;
	}
	
	inline span<const T> data() const
	{
		if (_base) {
			return span<const T>(_base, _d.len / sizeof(T));
		}
		return span<const T>(&_d.val, 1);
	}
	inline int type()
	{
		return type_properties<T>::id(true);
	}
	bool resize(long elem) {
		T *t = _base;
		long l = t ? _d.len / sizeof(*_base) : 0;
		
		if (l == elem) {
			return true; /* noop */
		}
		/* save/destruct vector data */
		if (!elem && t) {
			_d.val = t[0]; /* vector>scalar transition */
		}
		for (long i = elem; i < l; ++i) t[i].~T();
		
		/* free/extend memory */
		if (!elem) {
			free(t);
			t = 0;
		}
		else if (!(t = static_cast<T *>(realloc(t, elem * sizeof(*_base))))) {
			return false;
		}
		/* initialize new elements */
		for (T &val = _base ? t[l - 1] : _d.val; l < elem; ++l) t[l] = val;
		
		/* save new data */
		if (elem) {
			if (!_base) _d.val.~T(); /* scalar>vector transition */
			_d.len = elem * sizeof(*_base);
		}
		_base = t;
		
		return true;
	}
	T &operator [](int pos)
	{
		static T def;
		int len = _base ? _d.len / sizeof(T) : 1;
		if (pos < 0) pos += len;
		if (!pos && !_base) return _d.val;
		return (pos >= 0 && pos < len) ? _base[pos] : def;
	}
	struct value value() const
	{
		struct value v;
		if (_base) {
			v.set(type_properties<span<const T> >::id(true), this);
		} else {
			v.set(type_properties<T>::id(true), &_d.val);
		}
		return v;
	}
protected:
	T *_base;
	union {
		size_t len;
		T val;
	} _d;
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
# define MPT_VECPAR_INIT(p,v)  ((p)->_base = 0, (p)->_d.val = (v))
typedef struct
{
	double *_base;
	union { size_t len; double val; } _d;
} MPT_SOLVER_TYPE(dvecpar);
typedef struct
{
	int *_base;
	union { size_t len; int val; } _d;
} MPT_SOLVER_TYPE(ivecpar);
#endif

typedef int (MPT_SOLVER_TYPE(UserInit))(MPT_INTERFACE(convertable) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *);


__MPT_EXTDECL_BEGIN


/* create interface to solver */
extern MPT_INTERFACE(client) *mpt_client_ivp(MPT_SOLVER_TYPE(UserInit) *);
extern MPT_INTERFACE(client) *mpt_client_nls(MPT_SOLVER_TYPE(UserInit) *);


/* initialize IVP solver states */
extern int mpt_ivp_data(MPT_INTERFACE(object) *, const _MPT_ARRAY_TYPE(double) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));

extern int mpt_init_dae(MPT_INTERFACE(convertable) *, const MPT_IVP_STRUCT(daefcn) *, int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));
extern int mpt_init_ode(MPT_INTERFACE(convertable) *, const MPT_IVP_STRUCT(odefcn) *, int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));
extern int mpt_init_pde(MPT_INTERFACE(convertable) *, const MPT_IVP_STRUCT(pdefcn) *, int , const _MPT_ARRAY_TYPE(double) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));

/* initialize NLS solver states */
extern int mpt_init_nls(MPT_INTERFACE(convertable) *, const MPT_NLS_STRUCT(functions) *, const MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));


/* register events on notifier */
extern int mpt_solver_dispatch(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);

/* get input for required config elements */
extern int mpt_solver_require(MPT_INTERFACE(config) *, MPT_INTERFACE(logger) *);

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

/* query iteration source */
extern MPT_INTERFACE(iterator) *mpt_conf_iter(MPT_INTERFACE(metatype) **, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));

/* set problem specific parameters */
extern int mpt_conf_nls(MPT_STRUCT(solver_data) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));
extern int mpt_conf_ivp(MPT_STRUCT(solver_data) *, MPT_STRUCT(node) *, const MPT_STRUCT(value) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));

/* configure graphic output and bindings */
extern int mpt_conf_graphic(MPT_INTERFACE(output) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *);
/* configure history output and format */
extern int mpt_conf_history(MPT_INTERFACE(object) *, const MPT_STRUCT(node) *);

/* create profile data */
extern MPT_INTERFACE(metatype) *mpt_conf_profiles(const MPT_STRUCT(solver_data) *, double , const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *);

/* append user data */
extern int mpt_conf_param(MPT_STRUCT(array) *, const MPT_STRUCT(node) *, int);
/* set grid data */
extern int mpt_conf_grid(MPT_STRUCT(array) *, MPT_INTERFACE(convertable) *);


/* get/initialize solver data parameters */
extern double *mpt_solver_data_grid (MPT_STRUCT(solver_data) *);
extern double *mpt_solver_data_param(MPT_STRUCT(solver_data) *);

/* execute specific IVP solver step */
extern int mpt_solver_steps_ode(MPT_INTERFACE(convertable) *, MPT_INTERFACE(iterator) *,  MPT_INTERFACE(logger) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));

/* inner node residuals with central differences */
extern int mpt_residuals_cdiff(void *, double , const double *, double *, const MPT_IVP_STRUCT(parameters) *, MPT_SOLVER_IVP(Rside));

/* generate library description form short form */
extern const char *mpt_solver_alias(const char *);
/* load solver of specific type */
extern MPT_SOLVER(interface) *mpt_solver_load(MPT_INTERFACE(metatype) **, int , const char *, MPT_INTERFACE(logger) *);
/* get solver interface from metatype */
extern MPT_SOLVER(interface) *mpt_solver_conv(MPT_INTERFACE(convertable) *, int , MPT_INTERFACE(logger) *);

/* set solver parameter */
extern int mpt_solver_param(MPT_INTERFACE(object) *, MPT_STRUCT(node) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));
/* set solver parameter to value */
extern int mpt_solver_setvalue(MPT_INTERFACE(object) *, const char *, double);

/* generic solver output */
extern int mpt_solver_info  (MPT_SOLVER(interface) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));
extern int mpt_solver_status(MPT_SOLVER(interface) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()), int (*)(void *, const MPT_STRUCT(value) *) __MPT_DEFPAR(0), void *__MPT_DEFPAR(0));
extern int mpt_solver_report(MPT_SOLVER(interface) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::default_instance()));
/* final solver statistics */
extern void mpt_solver_statistics(MPT_SOLVER(interface) *, MPT_INTERFACE(logger) *, const struct timeval *, const struct timeval *);


/* output for solvers */
extern int mpt_solver_output_nls(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(value) *);
extern int mpt_solver_output_nlsdata(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(solver_data) *);
extern int mpt_solver_output_pde(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(value) *, const MPT_STRUCT(solver_data) *);
extern int mpt_solver_output_ode(const MPT_STRUCT(solver_output) *, int , const MPT_STRUCT(solver_data) *);
/* select solver log target */
extern int mpt_solver_output_query(MPT_STRUCT(solver_output) *, const MPT_INTERFACE(config) *);

/* push data to output */
extern int mpt_output_solver_data(MPT_INTERFACE(output) *, int , int , int , const double *, int);
extern int mpt_output_ivp_header(MPT_INTERFACE(output) *, int , int , const double *);
extern int mpt_output_solver_history(MPT_INTERFACE(output) *, const double *, int , const double *, int);


/* solver module tolerance handling */
extern int mpt_solver_module_tol_get(MPT_STRUCT(property) *, const MPT_SOLVER_TYPE(dvecpar) *);
extern int mpt_solver_module_tol_set(MPT_SOLVER_TYPE(dvecpar) *, MPT_INTERFACE(convertable) *, double);
extern int mpt_solver_module_tol_check(MPT_SOLVER_TYPE(dvecpar) *, long , long , double);

/* solver module memory management */
extern void *mpt_solver_module_valloc(struct iovec *, size_t , size_t);

/* solver module parameter assignment */
extern int mpt_solver_module_ivpset(MPT_IVP_STRUCT(parameters) *, MPT_INTERFACE(convertable) *);
extern int mpt_solver_module_nlsset(MPT_NLS_STRUCT(parameters) *, MPT_INTERFACE(convertable) *);

/* solver module generic type conversion */
extern int mpt_solver_module_nextval(double *, double , MPT_INTERFACE(convertable) *);

/* set value to buffered value */
extern int mpt_solver_module_value_set(MPT_STRUCT(property) *, int , const void *, size_t);
/* set value to equotation count */
extern int mpt_solver_module_value_ivp(MPT_STRUCT(property) *, const MPT_IVP_STRUCT(parameters) *);
extern int mpt_solver_module_value_nls(MPT_STRUCT(property) *, const MPT_NLS_STRUCT(parameters) *);
/* set value to vector data or element */
extern int mpt_solver_module_value_ivec(MPT_STRUCT(property) *, long , const struct iovec *);
extern int mpt_solver_module_value_rvec(MPT_STRUCT(property) *, long , const struct iovec *);
/* set value to scalar type */
extern int mpt_solver_module_value_int(MPT_STRUCT(property) *, const int *);
extern int mpt_solver_module_value_long(MPT_STRUCT(property) *, const long *);
extern int mpt_solver_module_value_double(MPT_STRUCT(property) *, const double *);
/* set value to string */
extern int mpt_solver_module_value_string(MPT_STRUCT(property) *, const char *);

/* assign user functions */
extern int mpt_solver_module_ufcn_ode(long , MPT_IVP_STRUCT(odefcn) *, int , const void *);
extern int mpt_solver_module_ufcn_dae(long , MPT_IVP_STRUCT(daefcn) *, int , const void *);
extern int mpt_solver_module_ufcn_nls(const MPT_NLS_STRUCT(parameters) *, MPT_NLS_STRUCT(functions) *, int , const void *);

/* wrap multiple proprties in intermediate object */
extern int mpt_solver_module_report_properties(const MPT_STRUCT(property) *, int , const char *, const char *, MPT_TYPE(property_handler) , void *);
/* consume value from iterator, allow primitive copy on non-zero size */
extern int mpt_solver_module_consume_value(MPT_INTERFACE(iterator) *it, MPT_TYPE(type) , void * , size_t);

/* id for registered solver metatype */
extern const MPT_STRUCT(named_traits) *mpt_solver_type_traits(void);

__MPT_EXTDECL_END

#ifdef __cplusplus
} /* namespace solver */

template<> inline __MPT_CONST_TYPE int type_properties<solver::interface *>::id(bool) {
	return TypeSolverPtr;
}
template <> inline const struct type_traits *type_properties<solver::interface *>::traits() {
	return type_traits::get(id(true));
}
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
