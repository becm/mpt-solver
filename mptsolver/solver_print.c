/*!
 * \file
 * print structured solver output
 */

#include <stdio.h>
#include <string.h>

#include <sys/uio.h>

#include "message.h"
#include "convert.h"

#include "solver.h"

#include "client.h"

struct wrap_fmt {
	int (*log)(void *, const char *, ... );
	void *par;
};

static int appendString(void *to, const char *data, size_t len)
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
	ret = log->_vptr->log(log, 0, MPT_ENUM(LogMessage), fmt, va);
	va_end(va);
	
	return ret;
}

static int wrapInfo(void *ptr, MPT_STRUCT(property) *pr)
{
	struct wrap_fmt *wp = ptr;
	struct iovec vec;
	char buf[256];
	size_t len;
	
	if (!pr->name) return 0;
	
	vec.iov_base = buf;
	vec.iov_len  = sizeof(buf) - 1;
	
	if (mpt_tostring(&pr->val, appendString, &vec) < 0) return -2;
	if (!(len = sizeof(buf) - vec.iov_len)) return 0;
	++vec.iov_len;
	appendString(&vec, "", 1);
	wp->log(wp->par, "%s: %s", pr->name, buf);
	return 1;
}

/*!
 * \ingroup mptClient
 * \brief solver state
 * 
 * Output solver state information
 * 
 * \param gen solver descriptor
 * \param log logging descriptor
 */
extern int mpt_solver_info(const MPT_SOLVER_INTERFACE *gen, MPT_INTERFACE(logger) *out)
{
	struct wrap_fmt wr;
	
	if (!(wr.par = out)) {
		wr.log = wrap_write;
		wr.par = stdout;
	}
	else wr.log = wrap_logger;
	
	return gen->_vptr->report(gen, MPT_SOLVER_ENUM(Header), wrapInfo, &wr);
}


static int wrapStatus(void *ptr, MPT_STRUCT(property) *pr)
{
	struct iovec *vec = ptr;
	
	if (!pr->name) return 0;
	
	/* save property name */
	appendString(vec, pr->name, strlen(pr->name));
	appendString(vec, " = ", 3);
	
	/* print property data */
	mpt_tostring(&pr->val, appendString, vec);
	
	/* add delimiter */
	appendString(vec, ", ", 2);
	
	return 1;
}
/*!
 * \ingroup mptClient
 * \brief solver status
 * 
 * Output current solver status
 * 
 * \param gen solver descriptor
 * \param out output descriptor
 */
extern int mpt_solver_status(const MPT_SOLVER_INTERFACE *gen, MPT_INTERFACE(logger) *out)
{
	char buf[1024];
	struct iovec io;
	
	io.iov_base = buf;
	io.iov_len = sizeof(buf);
	
	if (gen->_vptr->report(gen, MPT_SOLVER_ENUM(Status), wrapStatus, &io) < 0) {
		return -1;
	}
	if ((io.iov_len = sizeof(buf) - io.iov_len) < 2) return -2;
	
	/* remove last delimiter */
	((char *) io.iov_base)[-2] = 0;
	
	if (!out) {
		fputs(buf, stdout); fputc('\n', stdout);
	} else {
		wrap_logger(out, "%s", buf);
	}
	return 1;
}

static int wrapReport(void *ptr, MPT_STRUCT(property) *pr)
{
	struct wrap_fmt *wp = ptr;
	struct iovec vec;
	char buf[256];
	int len;
	
	if (!pr->name) return 0;
	
	vec.iov_base = buf;
	vec.iov_len  = sizeof(buf) - 1;
	
	if ((len = mpt_tostring(&pr->val, appendString, &vec)) < 0) {
		return len;
	}
	++vec.iov_len;
	appendString(&vec, "", 1);
	
	wp->log(wp->par, "%31s: %s", pr->desc, buf);
	return 1;
}
/*!
 * \ingroup mptClient
 * \brief solver report
 * 
 * Output solver report
 * 
 * \param gen solver descriptor
 * \param out output descriptor
 */
extern int mpt_solver_report(const MPT_SOLVER_INTERFACE *gen, MPT_INTERFACE(logger) *out)
{
	struct wrap_fmt wr;
	
	if (!(wr.par = out)) {
		wr.log = wrap_write;
		wr.par = stdout;
	}
	else wr.log = wrap_logger;
	
	return gen->_vptr->report(gen, MPT_SOLVER_ENUM(Report), wrapReport, &wr);
}
