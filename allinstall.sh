#!/bin/sh
echo
echo "Installing reflex"
cd lib; make -j -f Make install; cd -
cd src; make -j -f Make install; cd -
echo
echo "Installing reflex man page"
mkdir -p /usr/local/share/man/man1
cp -f doc/man/reflex.1 /usr/local/share/man/man1
echo
echo "OK"
