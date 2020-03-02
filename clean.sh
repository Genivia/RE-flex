#!/bin/bash
echo
echo "Cleaning reflex"
cd lib;      make -f Make distclean; cd -
cd src;      make -f Make distclean; cd -
cd unicode;  make -f Make clean; cd -
cd tests;    make -f Make clean; cd -
cd examples; make -f Make clean; cd -
rm -f Makefile config.h config.status config.log stamp-h1
echo
echo "OK"
