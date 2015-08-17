#!/bin/sh
# collect sundials objects created by CMake

# sundials objects base
if [ -z "${DIR_SUNDIALS}" ]; then \
  printf '%s\n' 'missing SUNDIALS base directory' 1>&2
  exit 1
fi

# target type
[ -z "${LIBRARY_TYPE}" ] && LIBRARY_TYPE="shared"

# elements to include
if [ $# -lt 1 ]; then \
  printf '%s\n' 'missing SUNDIALS elements' 1>&2
  exit 2
fi

# printf '%s\n' "collect SUNDIALS objects" 1>&2

# hash matches
objs=`tempfile --prefix='sun_'`

# loop over objects to include
for d in "$@"; do \
  case "$d" in
    nvec_ser)      s="nvecserial";;
    nvec_openmp)   s="nvecopenmp";;
    nvec_pthreads) s="nvecopenmp";;
    *) s="$d";;
  esac
  for f in `find "${DIR_SUNDIALS}/src/${d}/CMakeFiles/sundials_${s}_${LIBRARY_TYPE}.dir" -name '*.c.o'`; do \
# no objects hash file
    if [ ! -r "${objs}" ]; then \
      sha256sum "$f" > "${objs}"
      continue
    fi
# find hash for object
    o=`grep \`basename "$f"\` "${objs}"`
    if [ -z "$o" ]; then \
      sha256sum "$f" >> "${objs}"
      continue
    fi
# check for object collisions
    fhash=`sha256sum "$f" | cut -f 1 -d ' '`
    ohash=`printf '%s' "$o" | cut -f 1 -d ' '`
    if [ "x${fhash}" != "x${ohash}" ]; then \
      printf '%s:\n%s >\n%s  %s\n' 'conflicting objects' "$o" "${fhash}" "$f" 1>&2
      exit 1
    fi
  done
done

# output unique objects
cut -f3 -d ' ' "${objs}"

# remove hash file
unlink "${objs}"
