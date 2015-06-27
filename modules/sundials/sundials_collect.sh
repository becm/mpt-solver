#!/bin/sh
# collect sundials objects created by CMake

# sundials objects base
sundials_base="$1"
if [ -z "$sundials_base" ]; then \
  printf '%s\n' 'missing SUNDIALS base directory' 1>&2
  exit 1
fi
shift

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
  for f in `find ${sundials_base}/src/${d} -name '*.c.o'`; do \
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
    ohash=`sha256sum "$f" | cut -f 1 -d ' '`
    fhash=`printf '%s' "$o" | cut -f 1 -d ' '`
    if [ "x${fhash}" != "x${ohash}" ]; then \
      printf '%s\n%s\n' "${ohash}" "${fhash}"
      printf '%s: %s\n' 'conflicting objects' "$o" 1>&2
      exit 1
    fi
  done
done

# output unique objects
cut -f3 -d ' ' "${objs}"

# remove hash file
unlink "${objs}"
