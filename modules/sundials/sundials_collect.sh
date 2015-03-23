#!/bin/sh
# collect sundials objects created by CMake

# sundials objects base
sundials_base="$1"
shift
if [ -z "$sundials_base" ]; then \
  printf '%s\n' 'missing SUNDIALS base directory' > /dev/stderr
  exit 1
fi

# elements to include
if [ $# -lt 1 ]; then \
  printf '%s\n' 'missing SUNDIALS elements' > /dev/stderr
  exit 2
fi

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
      printf '%s: %s\n' 'conflicting objects' "$o" > /dev/stderr
      exit 1
    fi
  done
done

# output unique objects
cut -f3 -d ' ' "${objs}"

# remove hash file
unlink "${objs}"
