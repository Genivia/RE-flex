#!/bin/sh
echo
echo "Building reflex"
cd lib; make -j -f Make || exit 1; cd -
cd src; make -j -f Make || exit 1; cd -
echo
echo "OK"
