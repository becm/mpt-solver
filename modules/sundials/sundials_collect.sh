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

# printf '%s\n' "collect SUNDIALS objects" 1>&2

# initialize object buffers
OBJECTS=
COLLISIONS=

# loop over sundials modules
(cd "${DIR_SUNDIALS}/src/" && for d in "$@"; do \
  case "$d" in
    nvec_ser)    s="nvecserial";;
    nvec_*)      s="nvec${d#nvec_}";;
    sunlinsol_*) s="sunlinsol${d#sunlinsol_}";;
    *) s="$d";;
  esac
  # loop over module objects
  for f in `find ${d}/CMakeFiles/sundials_${s}_${LIBRARY_TYPE}.dir -name '*.c.o'`; do \
    # object name and hash
    o="${f##*/}"
    h=
    ho=
    hv=
    for c in ${OBJECTS}; do \
      # compare object names
      [ "${o}" != "${c##*/}" ] && continue
      # full path match
      [ "${f}" = "${c}" ] && { printf '%s\n' "module '${d}' already processed" 1>&2; exit 1; }
      # hash on name-only match
      [ -n "${h}" ] || {
        h=`sha256sum "${f}"`
        ho="${h##*/}"
        hv=${h%% *}
      }
      # look for existing collisions
      for s in ${COLLISIONS}; do \
        so=${s%%:*}
        sv=${s%:*}
        sv=${sv##*:}
        # check object name
        [ ${so} != ${ho} ] && continue
        # matching hashes, object has name collision entry
# echo ${sv},${hv} 1>&2
        [ "${sv}" = "${hv}" ] && { f=; break; }
        # object hash collision
        printf '%s: %s %s\n' "collision" "${f}" "${s##*:}"  1>&2
        exit 2
      done
      # add if new name collision found
      [ -n "${f}" ] && COLLISIONS="${COLLISIONS} ${ho}:${hv}:${f}"
      f=
      break
#       exit 2
    done
    [ -n "${f}" ] || continue
    # add new object
    OBJECTS="${OBJECTS} ${f}"
    printf '%s\n' "${DIR_SUNDIALS}/src/${f}"
  done
done)
