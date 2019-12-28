#!/bin/sh
echo
echo "Building reflex"
cd lib; make -f Make || exit 1; cd -
cd src; make -f Make || exit 1; cd -
echo
echo "OK"
