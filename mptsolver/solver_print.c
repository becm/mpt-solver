/*!
 * MPT solver library
 *   print structured solver output
 */

#include <stdio.h>
#include <string.h>

#include <sys/uio.h>

#include "client.h"

#include "convert.h"

#include "solver.h"

struct wrap_fmt {
	int (*log)(void *, const char *, ... );
	void *par;
};

int wrapText(const MPT_STRUCT(value) *val, ssize_t (*add)(void *, const char *, size_t ), void *ptr)
{
	if (MPT_value_isBaseType(val)
	 && val->_type == MPT_ENUM(TypeObjectPtr)) {
		const MPT_INTERFACE(object) *obj;
		uintptr_t i = 0;
		if (!val->_addr || !(obj = *((void * const *) val->_addr))) {
			return 0;
		}
		while (1) {
			MPT_STRUCT(property) tmp = MPT_PROPERTY_INIT;
			tmp.desc = (void *) i;
			if (obj->_vptr->property(obj, &tmp) < 0) {
				break;
			}
			if (i++ && add(ptr, " ", 1) < 0) {
				break;
			}
			if (mpt_print_value(&tmp.val, add, ptr) < 0) {
				break;
			}
		}
		return 2;
	}
	if (mpt_print_value(val, add, ptr) < 0) {
		return MPT_ERROR(BadValue);
	}
	return 1;
}

static ssize_t appendString(void *to, const char *data, size_t len)
{
	struct iovec *io = to;
	char *base = io->iov_base;
	size_t rem = io->iov_len;
	
	if (!len) return 0;
	
	if (len > rem) len = rem;
	
	/* print entry to buffer */
	(void) memcpy(base, data, len);
	
	io->iov_base = base + len;
	io->iov_len -= len;
	
	return len;
}

static int wrap_write(void *file, const char *fmt, ... )
{
	va_list va;
	int ret;
	
	if (!fmt || !file) return 0;
	
	va_start(va, fmt);
	ret = vfprintf(file, fmt, va);
	va_end(va);
	fputc('\n', file);
	return ret > 0 ? ret + 1 : ret;
}
static int wrap_logger(void *file, const char *fmt, ... )
{
	MPT_INTERFACE(logger) *log = file;
	va_list va;
	int ret;
	
	if (!fmt || !log) return 0;
	
	va_start(va, fmt);
	ret = log->_vptr->log(log, 0, MPT_LOG(Message), fmt, va);
	va_end(va);
	
	return ret;
}

static int wrapInfo(void *ptr, const MPT_STRUCT(property) *pr)
{
	struct wrap_fmt *wp = ptr;
	struct iovec vec;
	char buf[256];
	size_t len;
	int ret;
	
	if (!pr->name) return 0;
	
	vec.iov_base = buf;
	vec.iov_len  = sizeof(buf) - 1;
	
	if ((ret = wrapText(&pr->val, appendString, &vec)) < 0) {
		return MPT_ERROR(BadValue);
	}
	if (!(len = sizeof(buf) - vec.iov_len)) {
		return MPT_ERROR(MissingBuffer);
	}
	++vec.iov_len;
	appendString(&vec, "", 1);
	wp->log(wp->par, "%s: %s", pr->name, buf);
	return ret;
}

/*!
 * \ingroup mptSolver
 * \brief solver state
 * 
 * Output solver state information
 * 
 * \param gen solver descriptor
 * \param log logging descriptor
 */
extern int mpt_solver_info(MPT_SOLVER(interface) *gen, MPT_INTERFACE(logger) *out)
{
	struct wrap_fmt wr;
	
	if (!(wr.par = out)) {
		wr.log = wrap_write;
		wr.par = stdout;
	}
	else wr.log = wrap_logger;
	
	return gen->_vptr->report(gen, MPT_SOLVER_ENUM(Header), wrapInfo, &wr);
}

struct _wrapStatusCtx
{
	struct iovec vec;
	int (*vals)(void *, const MPT_STRUCT(value) *);
	void *ctx;
};
static int wrapStatus(void *ptr, const MPT_STRUCT(property) *pr)
{
	struct _wrapStatusCtx *out = ptr;
	int ret;
	
	if (!pr->name) {
		if (!out->vals) {
			return 0;
		}
		return out->vals(out->ctx, &pr->val);
	}
	/* save property name */
	appendString(&out->vec, pr->name, strlen(pr->name));
	appendString(&out->vec, " = ", 3);
	
	/* print property data */
	
	ret = wrapText(&pr->val, appendString, &out->vec);
	
	/* add delimiter */
	appendString(&out->vec, ", ", 2);
	
	return ret;
}
/*!
 * \ingroup mptSolver
 * \brief solver status
 * 
 * Output current solver status
 * 
 * \param gen solver descriptor
 * \param out output descriptor
 */
extern int mpt_solver_status(MPT_SOLVER(interface) *gen, MPT_INTERFACE(logger) *out, int (*vals)(void *, const MPT_STRUCT(value) *), void *ctx)
{
	char buf[1024];
	struct _wrapStatusCtx io;
	int what = MPT_SOLVER_ENUM(Status);
	
	io.vec.iov_base = buf;
	io.vec.iov_len = sizeof(buf);
	io.vals = vals;
	io.ctx = ctx;
	
	if (vals) what |= MPT_SOLVER_ENUM(Values);
	
	if (gen->_vptr->report(gen, what, wrapStatus, &io) < 0) {
		return -1;
	}
	if ((io.vec.iov_len = sizeof(buf) - io.vec.iov_len) < 2) return -2;
	
	/* remove last delimiter */
	((char *) io.vec.iov_base)[-2] = 0;
	
	if (!out) {
		fputs(buf, stdout); fputc('\n', stdout);
	} else {
		wrap_logger(out, "%s", buf);
	}
	return 1;
}

static int wrapReport(void *ptr, const MPT_STRUCT(property) *pr)
{
	struct wrap_fmt *wp = ptr;
	struct iovec vec;
	char buf[256];
	int len;
	
	if (!pr->name) return 0;
	
	vec.iov_base = buf;
	vec.iov_len  = sizeof(buf) - 1;
	
	if ((len = wrapText(&pr->val, appendString, &vec)) < 0) {
		return len;
	}
	++vec.iov_len;
	appendString(&vec, "", 1);
	
	wp->log(wp->par, "%31s: %s", pr->desc, buf);
	return 1;
}
/*!
 * \ingroup mptSolver
 * \brief solver report
 * 
 * Output solver report
 * 
 * \param gen solver descriptor
 * \param out output descriptor
 */
extern int mpt_solver_report(MPT_SOLVER(interface) *gen, MPT_INTERFACE(logger) *out)
{
	struct wrap_fmt wr;
	
	if (!(wr.par = out)) {
		wr.log = wrap_write;
		wr.par = stdout;
	}
	else wr.log = wrap_logger;
	
	
	
	return gen->_vptr->report(gen, MPT_SOLVER_ENUM(Report), wrapReport, &wr);
}
