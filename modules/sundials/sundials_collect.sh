#!/bin/sh
# collect sundials objects created by CMake
SUNDIALS_BUILD="${1}"
shift

# sundials objects base
if [ ! -d "${SUNDIALS_BUILD}" ]; then \
  printf '%s\n  %s\n' 'invalid SUNDIALS build directory' "${0} <SUNDIALS_BUILD> [modules]" 1>&2
  exit 1
fi

# target type
[ -z "${LIBRARY_TYPE}" ] && LIBRARY_TYPE="shared"

# need SOME elements
if [ $# -lt 1 ]; then \
  printf '%s\n' 'missing SUNDIALS elements' 1>&2
  exit 2
fi

# loop over sundials modules
(cd "${SUNDIALS_BUILD}/src/" && for d in "$@"; do \
  case "$d" in
    nvector_*)  s="nvec${d#nvector_}"; d="nvector/${d#nvector_}";;
    sun*)       s="${d%%_*}${d#*_}";   d="${d%%_*}/${d#*_}";;
    generic)    s="generic";           d="sundials";;
    *)          s="$d";;
  esac
  # loop over module objects
  for f in `find ${d}/CMakeFiles/sundials_${s}_obj_${LIBRARY_TYPE}.dir -name '*.c.o'`; do \
    printf '%s\n' "${SUNDIALS_BUILD}/src/${f}"
  done
done)
