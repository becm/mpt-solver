/*!
 * MPT solver interfaces
 *  generic solver, loadable from library
 */

#ifndef _MPT_SOLVER_H
#define _MPT_SOLVER_H  201502

#include <sys/time.h>

#include "core.h"

#ifdef __cplusplus
# include <stdlib.h>
#endif

struct iovec;
struct rusage;

__MPT_NAMESPACE_BEGIN

MPT_INTERFACE(client);
MPT_INTERFACE(output);

MPT_STRUCT(dispatch);
MPT_STRUCT(message);
MPT_STRUCT(event);

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
# define MPT_SOLVER_TYPE(i)   MPT_SOLVER_##i
# define MPT_SOLVER_ENUM(i)   MPT_SOLVER_##i
# define __MPT_SOLVER_BEGIN
# define __MPT_SOLVER_END
# define MPT_SOLVER_INTERFACE struct mpt_solver_generic
#else
# define MPT_SOLVER_STRUCT(i) struct i
# define MPT_SOLVER_TYPE(i)   i
# define MPT_SOLVER_ENUM(i)   i
# define __MPT_SOLVER_BEGIN   namespace mpt { namespace solver {
# define __MPT_SOLVER_END     } }
# define MPT_SOLVER_INTERFACE class generic

namespace solver {

#endif

/* ODE solver functions */
typedef int (*MPT_TYPE(IvpFcn))(void *upar, double t, const double *y, double *f);
typedef int (*MPT_TYPE(IvpJac))(void *upar, double t, const double *y, double *jac, int ldjac);
/* extension for DAE solvers */
typedef int (*MPT_TYPE(IvpMas))(void *upar, double t, const double *y, double *band, int *ir, int *ic);
/* extensions for PDE solvers */
typedef int (*MPT_TYPE(RsideFcn))(void *upar, double x, const double *y, double t, double *f, double *dx, double *dy);
typedef int (*MPT_TYPE(PdeFcn))(void *upar, MPT_TYPE(RsideFcn), int pts, const double *x, double t, const double *y, double *f);

/* non-linear solver functions */
typedef int (*MPT_TYPE(NlsFcn))(void *rpar, const double *p, double *res, const int *lres);
typedef int (*MPT_TYPE(NlsJac))(void *jpar, const double *p, double *jac, const int *ld, const double *res);

MPT_SOLVER_STRUCT(nlspar);
typedef int (*MPT_TYPE(NlsOut))(void *opar, MPT_SOLVER_STRUCT(nlspar) *np, const double *p, const double *res);


enum MPT_SOLVER_ENUM(Flags)
{
	MPT_SOLVER_ENUM(ODE)        = 0x11,
	MPT_SOLVER_ENUM(DAE)        = 0x12,
	MPT_SOLVER_ENUM(PDE)        = 0x14,
	MPT_SOLVER_ENUM(CapableIvp) = 0x10,
	
	MPT_SOLVER_ENUM(NlsVector)  = 0x21,
	MPT_SOLVER_ENUM(NlsOverDet) = 0x22,
	MPT_SOLVER_ENUM(CapableNls) = 0x20,
	
	MPT_SOLVER_ENUM(Header)     = 0x1,
	MPT_SOLVER_ENUM(Status)     = 0x2,
	MPT_SOLVER_ENUM(Report)     = 0x4,
	MPT_SOLVER_ENUM(Values)     = 0x8
};

/*! generic IVP functions */
MPT_SOLVER_STRUCT(odefcn)
{
	MPT_TYPE(IvpFcn) fcn;   /* unified right side calculation */
	void            *param; /* user parameter for handler functions */
	MPT_TYPE(IvpJac) jac;   /* (full/banded) jacobi matrix */
};
MPT_SOLVER_STRUCT(daefcn)
{
	MPT_TYPE(IvpFcn)  fcn;   /* unified right side calculation */
	void             *param; /* user parameter for handler functions */
	MPT_TYPE(IvpJac)  jac;   /* (full/banded) jacobi matrix */
	MPT_TYPE(IvpMas)  mas;   /* (sparse) mass matrix */
};
MPT_SOLVER_STRUCT(pdefcn)
{
	MPT_TYPE(PdeFcn)    fcn;   /* right side function */
	void               *param; /* user parameter for handler functions */
	MPT_TYPE(RsideFcn)  rside; /* right side function */
};
typedef union
{
#ifndef __cplusplus
# define MPT_IVPFCN_INIT(d)  ((MPT_SOLVER_TYPE(ivpfcn) *) memset(d, 0, sizeof(MPT_SOLVER_TYPE(ivpfcn))))
#endif
	MPT_SOLVER_STRUCT(odefcn) ode;
	MPT_SOLVER_STRUCT(daefcn) dae;
	MPT_SOLVER_STRUCT(pdefcn) pde;
} MPT_SOLVER_TYPE(ivpfcn);

/*! general IVP parameter */
MPT_SOLVER_STRUCT(ivppar)
{
#ifdef __cplusplus
	inline ivppar() : neqs(0), pint(0), last(0)
	{ }
#else
# define MPT_IVPPAR_INIT(d)  ((d)->neqs = (d)->pint = (d)->last = 0)
#endif
	int32_t neqs;  /* number of equotations */
	int32_t pint;  /* number pde intervals */
	
	double  last;  /* reached time */
};
typedef int (*MPT_TYPE(DataInitializer))(void *, double *, size_t);

/*! generic nonlinear system functions */
MPT_SOLVER_STRUCT(nlsfcn)
{
#ifdef __cplusplus
	inline nlsfcn(NlsFcn f, void *u = 0) : res(f), jac(0), rpar(u), jpar(0), out(0), opar(0)
	{ }
#else
# define MPT_NLSFCN_INIT(d)  ((MPT_SOLVER_STRUCT(nlsfcn) *) memset(d, 0, sizeof(MPT_SOLVER_STRUCT(nlsfcn))))
#endif
	MPT_TYPE(NlsFcn) res;  /* calculate residuals */
	MPT_TYPE(NlsJac) jac;  /* calculate residual jacobian */
	void *rpar, *jpar;     /* residual/jacobian function parameter */
	
	MPT_TYPE(NlsOut) out;  /* output current parameter/residuals */
	void *opar;            /* output function parameter */
};
/*! general nonlinear system parameter */
MPT_SOLVER_STRUCT(nlspar)
{
#ifdef __cplusplus
	inline nlspar() : nval(0), nres(0)
	{ }
#else
# define MPT_NLSPAR_INIT(d)  ((d)->nres = (d)->nval = 0)
#endif
	int32_t nval,  /* parameters to optimize */
	        nres;  /* number of residuals */
};

MPT_SOLVER_STRUCT(data)
#ifdef _MPT_ARRAY_H
{
#ifdef __cplusplus
	inline data() : iter(0), npar(0), nval(0)
	{ for (size_t i = 0; i < sizeof(mask)/sizeof(*mask); ++i) mask[i] = 0; }
#endif
	MPT_INTERFACE(iterator) *iter;     /* step iterator */
	MPT_STRUCT(array)        param,    /* parameter matrix */
	                         val;      /* input/output (matrix) data */
	int                      npar,     /* leading dimension of parameter data */
	                         nval;     /* length of value matrix row */
	uint8_t                  mask[16]; /* masked dimensions (0..255) */
	struct timeval           ru_usr,   /* user time in solver backend */
	                         ru_sys;   /* system time in solver backend */
}
#endif
;

#ifdef __cplusplus
/*! generic solver interface */
class generic : public object
{
protected:
	~generic() {}
public:
	enum { Type = TypeSolver };
	
	virtual int report(int what, PropertyHandler out, void *opar) = 0;
};
/*! extension for Initial Value Problems */
class Ivp : public generic
{
protected:
	~Ivp() {}
public:
	virtual int step(double *end) = 0;
	virtual ivpfcn *functions(int type = ODE) const;
	
	operator const struct ivppar *();
};

/*! extension for NonLinear Systems */
class Nls : public generic
{
protected:
	~Nls() {}
    public:
	virtual int step(double *res) = 0;
	virtual operator nlsfcn *() const;
	
	operator const struct nlspar *();
};

inline Ivp::operator const ivppar *()
{ struct property p; return property(&p) < 0 ? 0 : (struct ivppar *) p.val.ptr; }
inline ivpfcn *Ivp::functions(int) const
{ return 0; }

inline Nls::operator const nlspar *()
{ struct property p; return property(&p) < 0 ? 0 : (struct nlspar *) p.val.ptr; }
inline Nls::operator nlsfcn *() const
{ return 0; }

template <typename T>
struct vecpar
{
	inline vecpar(T const &val) : base(0)
	{ d.val = val; }
	inline vecpar(void) : base(0)
	{ d.val = T(); }
	inline ~vecpar()
	{ if (base) resize(0); }
	
	inline Slice<const T> data() const
	{
		if (base) return Slice<const T>(base, d.len / sizeof(T));
		return Slice<const T>(&d.val, 1);
	}
	inline int type(void)
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
		else if (!(t = (T*) realloc(t, elem*sizeof(*base)))) return false;
		
		/* initialize new elements */
		for (T &val = base ? t[l-1] : d.val; l < elem; ++l) t[l] = val;
		
		/* save new data */
		if (elem) {
			if (!base) d.val.~T(); /* scalar>vector transition */
			d.len = elem*sizeof(*base);
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
		if (base) {
			static const char fmt[2] = { vectorIdentifier<T>(), 0 };
			return value(fmt, this);
		} else {
			static const char fmt[2] = { typeIdentifier<T>(), 0 };
			return value(fmt, &d.val);
		}
	}
protected:
	T *base;
	union {
		size_t len;
		T val;
	} d;
};

typedef vecpar<double> dvecpar;
typedef vecpar<int> ivecpar;
#else
# define MPT_VECPAR_INIT(p,v)  ((p)->base = 0, (p)->d.val = (v))
typedef struct
{
	double *base;
	union { size_t len; double val; } d;
} MPT_TYPE(dvecpar);
typedef struct
{
	int *base;
	union { size_t len; int val; } d;
} MPT_TYPE(ivecpar);

MPT_SOLVER_INTERFACE;
MPT_INTERFACE_VPTR(solver)
{
	/* metatype operations */
	MPT_INTERFACE_VPTR(object) obj;
	/* call output function with data to report for type */
	int (*report)(MPT_SOLVER_INTERFACE *, int , MPT_TYPE(PropertyHandler) , void *opar);
};
MPT_SOLVER_INTERFACE
{ const MPT_INTERFACE_VPTR(solver) *_vptr; };

MPT_INTERFACE_VPTR(Ivp)
{
	MPT_INTERFACE_VPTR(solver) gen;
	int (*step)(MPT_SOLVER_INTERFACE *, double *end);
	MPT_SOLVER_TYPE(ivpfcn) *(*ufcn) (const MPT_SOLVER_INTERFACE *, int);
};
MPT_INTERFACE_VPTR(Nls)
{
	MPT_INTERFACE_VPTR(solver) gen;
	int (*step)(MPT_SOLVER_INTERFACE *s, double *res);
	MPT_SOLVER_STRUCT(nlsfcn) *(*ufcn) (const MPT_SOLVER_INTERFACE *);
};
#endif

__MPT_EXTDECL_BEGIN


/* create interface to specific solver type */
extern MPT_INTERFACE(client) *mpt_client_ode(int (*)(MPT_SOLVER_STRUCT(odefcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *));
extern MPT_INTERFACE(client) *mpt_client_dae(int (*)(MPT_SOLVER_STRUCT(daefcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *));
extern MPT_INTERFACE(client) *mpt_client_pde(int (*)(MPT_SOLVER_STRUCT(pdefcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *));
extern MPT_INTERFACE(client) *mpt_client_nls(int (*)(MPT_SOLVER_STRUCT(nlsfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *));

/* register events on notifier */
extern int mpt_solver_events(MPT_STRUCT(dispatch) *, MPT_INTERFACE(client) *);

/* read files, initialize and prepare solver, set default handler */
extern int mpt_solver_start(MPT_INTERFACE(client) *, MPT_STRUCT(event) *, MPT_STRUCT(node) *);

/* read files to configuration */
extern const char *mpt_solver_read(MPT_STRUCT(node) *, MPT_INTERFACE(metatype) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* initialize solver memory */
extern void mpt_data_init(MPT_SOLVER_STRUCT(data) *);
extern void mpt_data_fini(MPT_SOLVER_STRUCT(data) *);
extern void mpt_data_clear(MPT_SOLVER_STRUCT(data) *);


/* add usage time difference */
extern void mpt_data_timeradd(MPT_SOLVER_STRUCT(data) *, const struct rusage *, const struct rusage *);


/* set problem specific parameters */
extern int mpt_conf_nls(MPT_SOLVER_STRUCT(data) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *);
extern int mpt_conf_pde(MPT_SOLVER_STRUCT(data) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *);
extern int mpt_conf_ode(MPT_SOLVER_STRUCT(data) *, const MPT_STRUCT(node) *, MPT_INTERFACE(logger) *);

/* get/initialize solver data parameters */
extern double *mpt_data_grid (MPT_SOLVER_STRUCT(data) *, int);
extern double *mpt_data_param(MPT_SOLVER_STRUCT(data) *);

/* execute specific problem solver step */
extern int mpt_step_pde(MPT_SOLVER_INTERFACE *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *);
extern int mpt_step_ode(MPT_SOLVER_INTERFACE *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *);
extern int mpt_step_nls(MPT_SOLVER_INTERFACE *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *);

/* inner node residuals with central differences */
extern int mpt_residuals_cdiff(MPT_TYPE(RsideFcn) , double , const double *, int , const double *, int , double *);

/* check solver capabilities */
extern int mpt_solver_check(MPT_SOLVER_INTERFACE *, int);
/* generate library description form short form */
extern const char *mpt_solver_alias(const char *);

/* set solver parameter */
extern int  mpt_solver_pset (MPT_INTERFACE(object) *, const MPT_STRUCT(node) *, int , MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern void mpt_solver_param(MPT_INTERFACE(object) *, const MPT_STRUCT(node) *, MPT_INTERFACE(metatype) *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));

/* generic solver output */
extern int mpt_solver_info  (const MPT_SOLVER_INTERFACE *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_solver_status(const MPT_SOLVER_INTERFACE *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
extern int mpt_solver_report(const MPT_SOLVER_INTERFACE *, MPT_INTERFACE(logger) *__MPT_DEFPAR(logger::defaultInstance()));
/* final solver statistics */
extern void mpt_solver_statistics(const MPT_SOLVER_INTERFACE *, MPT_INTERFACE(logger) *, const struct timeval *, const struct timeval *);

/* get solver time info from report */
extern int mpt_solver_time(const MPT_SOLVER_INTERFACE *, double *);

/* solver module data management */
extern void *mpt_vecpar_alloc(struct iovec *, size_t len, size_t size);

extern int mpt_vecpar_cktol(MPT_TYPE(dvecpar) *, int len, int repeat, double def);
extern int mpt_vecpar_get(const MPT_TYPE(dvecpar) *, MPT_STRUCT(value) *);
extern int mpt_vecpar_set(MPT_TYPE(dvecpar) *, MPT_INTERFACE(metatype) *);

extern int mpt_ivppar_set(MPT_SOLVER_STRUCT(ivppar) *, MPT_INTERFACE(metatype) *);
extern int mpt_nlspar_set(MPT_SOLVER_STRUCT(nlspar) *, MPT_INTERFACE(metatype) *);

__MPT_EXTDECL_END

#ifdef __cplusplus
} /* namespace solver */
#endif

__MPT_NAMESPACE_END

#endif /* _MPT_SOLVER_H */
