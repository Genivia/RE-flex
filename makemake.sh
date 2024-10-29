#!/bin/sh

# This script is used to prepare a new release update by software maintainers
# - update version in src/reflex.h
# - rebuild reflex to verify
# - generate man page
# - update version in configure.ac
# - run autoconf and automake

fgrep -r FIXME include lib src unicode && ( echo "FIXME in code base" ; exit 1 )

if [ "$#" = 1 ]
then

echo
echo "Bumping reflex version to $1"

sed "s/REFLEX_VERSION \"[^\"]*\"/REFLEX_VERSION \"$1\"/" src/reflex.h > src/reflex.tmp && mv -f src/reflex.tmp src/reflex.h || exit 1

# this may be needed to reconfigure for glibtoolize for example
# autoreconf -fvi

./clean.sh
./build.sh || exit 1
./man.sh $1
./clean.sh

sed "s/^\(AC_INIT(\[re-flex\],\[\)[0-9.]*/\1$1/" configure.ac > configure.tmp && mv -f configure.tmp configure.ac || exit 1

aclocal
autoheader
rm -f config.guess config.sub ar-lib compile depcomp install-sh missing
automake --add-missing --foreign
autoconf
automake
touch config.h.in
./configure

echo OK

# removed: visit GitHub releases to create a tag instead
# pushd $HOME/GitHub/RE-flex; git tag -a reflex-$1 -m "reflex version $1"; popd

else

echo "Usage: ./makemake.sh 5.v.v"
exit 1

fi
