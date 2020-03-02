#!/bin/sh
echo
echo "Installing reflex..."
echo

# was reflex build?
if ! test -f lib/libreflex.a ; then
echo "Cannot find reflex, please build reflex first, then try again"
exit 1
fi

# install the reflex tool and libreflex library at the designated locations
cd lib; make -j -f Make install || exit 1; cd -
cd src; make -j -f Make install || exit 1; cd -
echo

# install the man page at the designated location
echo "Installing the reflex man page"
echo
mkdir -p /usr/local/share/man/man1
cp -f doc/man/reflex.1 /usr/local/share/man/man1
echo
echo "OK"
