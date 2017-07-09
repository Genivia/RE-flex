#!/bin/sh
echo
echo "Building reflex"
cd lib; make -f Make; cd -
cd src; make -f Make; cd -
echo
echo "OK"
