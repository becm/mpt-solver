#!/bin/sh
# collect sundials objects created by CMake

# sundials objects base
if [ -z "${DIR_SUNDIALS}" ]; then \
  printf '%s\n' 'missing SUNDIALS base directory' 1>&2
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
(cd "${DIR_SUNDIALS}/src/" && for d in "$@"; do \
  case "$d" in
    nvector_*)  s="nvec${d#nvector_}"; d="nvector/${d#nvector_}";;
    sun*)       s="${d%%_*}${d#*_}";   d="${d%%_*}/${d#*_}";;
    generic)    s="generic";           d="sundials";;
    *)          s="$d";;
  esac
  # loop over module objects
  for f in `find ${d}/CMakeFiles/sundials_${s}_obj_${LIBRARY_TYPE}.dir -name '*.c.o'`; do \
    printf '%s\n' "${DIR_SUNDIALS}/src/${f}"
  done
done)
