#!/bin/sh
echo
echo "Cleaning reflex"
cd lib;      make -f Make distclean; cd -
cd src;      make -f Make distclean; cd -
cd unicode;  make -f Make clean; cd -
cd tests;    make -f Make clean; cd -
cd examples; make -f Make clean; cd -
echo
echo "OK"
