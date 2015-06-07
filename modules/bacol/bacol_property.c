/*!
 * set BACOL solver parameter from input source
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>

#include "release.h"
#include "version.h"

#include "bacol.h"

static int setIvp(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	if (!src) {
		return mpt_ivppar_set(&data->ivp, src);
	}
	else {
		MPT_SOLVER_STRUCT(ivppar) ivp = data->ivp;
		int ret;
		
		if ((ret =  mpt_ivppar_set(&ivp, src)) < 0) {
			return ret;
		}
		mpt_bacol_fini(data);
		mpt_bacol_init(data);
		data->ivp = ivp;
		return ret;
	}
}
static int setStepInit(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->mflag.step;
	
	if (!(len = src->_vptr->conv(src, 'd', &data->initstep))) data->mflag.step = 0;
	else if (len > 0) data->mflag.step = 1;
	
	return len;
}
static int setDirichlet(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	if (!src) return data->mflag.bdir;
	
	return src->_vptr->conv(src, 'i', &data->mflag.bdir);
}
static int setKollocations(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	int len, kcol = 2;
	
	if (!src) return data->kcol;
	if ((len = src->_vptr->conv(src, 'i', &kcol)) < 0) return len;
	if (kcol < 2 || kcol > 10) return -2;
	data->kcol = kcol;
	return len;
}
static int setIntervals(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	int len, nint = 10;
	
	if (!src) return data->nint;
	if ((len = src->_vptr->conv(src, 'i', &nint)) < 0) return len;
	if (nint < 2 || nint > data->nintmx) return -2;
	data->nint = nint;
	return len;
}
static int setMaxIntervals(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	int len, nintmx = 32;
	
	if (!src) return data->nintmx;
	if ((len = src->_vptr->conv(src, 'i', &nintmx)) < 0) return len;
	if (nintmx < data->nint) return -2;
	data->nintmx = nintmx;
	return len;
}
static int setBackend(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	char *val;
	int len;
	
	if (!src) return data->backend;
	
	if (!(len = src->_vptr->conv(src, 'k', &val))) val = "d";
	if (len < 0) return len;
	switch (*val) {
#ifdef MPT_BACOL_DASSL
	  case 'd': case 'D': if (tolower(data->backend) == tolower(*val)) return 0;
	    *((short *) &data->backend) = 'd';
	    if (data->bd.cpar.iov_base) free(data->bd.cpar.iov_base);
	    break;
#endif
#ifdef MPT_BACOL_RADAU
	  case 'r': case 'R': if (tolower(data->backend) == tolower(*val)) return 0;
	    *((short *) &data->backend) = 'r';
	    data->bd.cpar.iov_base = 0; data->bd.cpar.iov_len = 0;
	    break;
#endif
	  default: return -2;
	}
	return len;
}
#ifdef MPT_BACOL_DASSL
static int setTStop(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->mflag.tstop;
	if (!(len = src->_vptr->conv(src, 'd', &data->bd.tstop))) data->mflag.tstop = 0;
	else if (len > 0) data->mflag.tstop = 1;
	return len;
}
static int setMaxStep(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->mflag.mstep;
	if (!(len = src->_vptr->conv(src, 'i', &data->mflag.mstep))) data->mflag.mstep = 0;
	return len;
}
static int setDasslBdf(MPT_SOLVER_STRUCT(bacol) *data, MPT_INTERFACE(source) *src)
{
	int len;
	
	if (!src) return data->mflag.dbmax;
	if (!(len = src->_vptr->conv(src, 'i', &data->mflag.dbmax))) data->mflag.dbmax = 0;
	return len;
}
#endif

extern int mpt_bacol_property(MPT_SOLVER_STRUCT(bacol) *data, MPT_STRUCT(property) *prop, MPT_INTERFACE(source) *src)
{
	const char *name;
	intptr_t pos = -1, id;
	
	if (!prop) return (src && data) ? setIvp(data, src) : MPT_ENUM(TypeSolver);
	
	if (!(name = prop->name)) {
		if (src || ((pos = (intptr_t) prop->desc)) < 0) {
			errno = EINVAL;
			return -3;
		}
	}
	else if (!*name) {
		id = MPT_SOLVER_ENUM(PDE);
		if (data && src && (id = setIvp(data, src)) < 0) return id;
		prop->name = "bacol"; prop->desc = "SPline PDE solver";
		prop->val.fmt = "iid"; prop->val.ptr = &data->ivp;
		return id;
	}
	if (name && !strcasecmp(name, "version")) {
		static const char version[] = MPT_VERSION"\0";
		prop->name = "version"; prop->desc = "solver release information";
		prop->val.fmt = 0; prop->val.ptr = version;
		return 0;
	}
	
	id = 0;
	if (name ? !strcasecmp(name, "atol") : pos == id++) {
		if (data && (id = mpt_vecpar_property(&data->atol, prop, src)) < 0) return id;
		prop->name = "atol"; prop->desc = "absolute tolerances";
		return id;
	}
	if (name ? !strcasecmp(name, "rtol") : pos == id++) {
		if (data && (id = mpt_vecpar_property(&data->rtol, prop, src)) < 0) return id;
		prop->name = "rtol"; prop->desc = "relative tolerances";
		return id;
	}
	/* bacol parameters */
	if (name ? !strcasecmp(name, "kcol") : pos == id++) {
		if (data && (id = setKollocations(data, src)) < 0) return id;
		prop->name = "kcol"; prop->desc = "collocations per interval";
		prop->val.fmt = "h"; prop->val.ptr = &data->kcol;
		return id;
	}
	if (name ? !strcasecmp(name, "nint") : pos == id++) {
		if (data && (id = setIntervals(data, src)) < 0) return id;
		prop->name = "nint"; prop->desc = "number of internal intervals";
		prop->val.fmt = "i"; prop->val.ptr = &data->nint;
		return id;
	}
	if (name ? !strcasecmp(name, "nintmx") : pos == id++) {
		if (data && (id = setMaxIntervals(data, src)) < 0) return id;
		prop->name = "nintmx"; prop->desc = "maximum internal intervals";
		prop->val.fmt = "i"; prop->val.ptr = &data->nintmx;
		return id;
	}
	if (name ? !strcasecmp(name, "dirichlet") : pos == id++) {
		if (data && (id = setDirichlet(data, src)) < 0) return id;
		prop->name = "dirichlet"; prop->desc = "boundaries of dirichlet type";
		prop->val.fmt = "i"; prop->val.ptr = &data->mflag.bdir;
		return id;
	}
	/* ode parameter */
	if (name ? !strcasecmp(name, "stepinit") : pos == id++) {
		if (data && (id = setStepInit(data, src)) < 0) return id;
		prop->name = "stepinit"; prop->desc = "expl. initial stepsize";
		prop->val.fmt = "d"; prop->val.ptr = &data->initstep;
		return id;
	}
	/* solving backend */
	if (name ? !strcasecmp(name, "backend") : pos == id++) {
		if (data && (id = setBackend(data, src)) < 0) return id;
		prop->name = "backend"; prop->desc = "solver step backend";
		prop->val.fmt = 0; prop->val.ptr = 0;
		if (!data) return id;
		switch (data->backend) {
#ifdef MPT_BACOL_DASSL
		  case 'd': case 'D': prop->val.ptr = "dassl"; break;
#endif
#ifdef MPT_BACOL_RADAU
		  case 'r': case 'R': prop->val.ptr = "radau"; break;
#endif
		  default: prop->val.ptr = "<unknown>";
		}
		return id;
	}
#ifdef MPT_BACOL_DASSL
	/* dassl parameter */
	if (data->backend == 'd' || data->backend == 'D') {
	if (name ? !strcasecmp(name, "tstop") : pos == id++) {
		if (data && (id = setTStop(data, src)) < 0) return id;
		prop->name = "tstop"; prop->desc = "max. time allowed";
		prop->val.fmt = "d"; prop->val.ptr = &data->mflag.bdir;
		return id;
	}
	/* dassl parameter */
	if (name ? !strcasecmp(name, "maxstep") : pos == id++) {
		if (data && (id = setMaxStep(data, src)) < 0) return id;
		prop->name = "maxstep"; prop->desc = "max. steps per call";
		prop->val.fmt = "d"; prop->val.ptr = &data->bd.tstop;
		return id;
	}
	if (name ? (!strcasecmp(name, "dasslbdf") || !strcasecmp(name, "bdf")) : pos == id++) {
		if (data && (id = setDasslBdf(data, src)) < 0) return id;
		prop->name = "dasslbdf"; prop->desc = "max. number of BDF order";
		prop->val.fmt = "i"; prop->val.ptr = &data->mflag.bdir;
		return id;
	}
	}
#endif
	errno = EINVAL;
	return -3;
}
