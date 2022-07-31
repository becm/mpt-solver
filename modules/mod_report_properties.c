/*!
 * MPT solver module helper function
 *   prepare vecpar data
 */

#include <string.h>

#include <sys/uio.h>

#include "../solver.h"

struct _mpt_solver_values
{
	MPT_INTERFACE(object) _obj;
	const MPT_STRUCT(property) *pr;
	int len;
};

static int valPropertyGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const struct _mpt_solver_values *val = (void *) obj;
	int i;
	if (!pr) {
		return 0;
	}
	if (!pr->name) {
		intmax_t pos = (uintptr_t) pr->desc;
		const MPT_STRUCT(property) *curr;
		if (pos < 0 || pos >= val->len) {
			return MPT_ERROR(BadValue);
		}
		curr = val->pr + pos;
		pr->name = curr->name;
		pr->desc = curr->desc;
		pr->val  = curr->val;
		
		return 2;
	}
	else if (!*pr->name) {
		pr->name = "object";
		pr->desc = MPT_tr("solver module output wrapper");
		MPT_value_set(&pr->val, 0, 0);
		return val->len;
	}
	for (i = 0; i < val->len; i++) {
		const MPT_STRUCT(property) *curr = val->pr + i;
		if (!strcmp(pr->name, curr->name)) {
			pr->name = curr->name;
			pr->desc = curr->desc;
			pr->val  = curr->val;
			
			return 1;
		}
	}
	return MPT_ERROR(BadArgument);
}

static int valPropertySet(MPT_INTERFACE(object) *obj, const char *prop, MPT_INTERFACE(convertable) *src)
{
	(void) obj;
	(void) prop;
	(void) src;
	return MPT_ERROR(BadOperation);
}


/*!
 * \ingroup mptSolver
 * \brief process nested properties
 * 
 * Use opbect interface to nest multiple properties in single value.
 * 
 * \param prop   properties to be nested
 * \param len    number of nested properties
 * \param name   name for wrapping propery
 * \param desc   description for wrapping propery
 * \param out    property handler to call
 * \param usr    handler user data
 * 
 * \return result of property handler call
 */
extern int mpt_solver_module_report_properties(const MPT_STRUCT(property) *prop, int len, const char *name, const char *desc, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) wrap = MPT_PROPERTY_INIT;
	static const MPT_INTERFACE_VPTR(object) ctl = {
		valPropertyGet,
		valPropertySet,
	};
	struct _mpt_solver_values *ptr, val = { { &ctl }, 0, 0 };
	
	if (!prop || len < 0 || !out) {
		return MPT_ERROR(BadArgument);
	}
	
	val.pr  = prop;
	val.len = len;
	
	ptr = &val;
	wrap.name = name;
	wrap.desc = desc;
	MPT_property_set_data(&wrap, MPT_ENUM(TypeObjectPtr), &ptr);
	
	return out(usr, &wrap);
}
