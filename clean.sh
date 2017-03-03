#!/bin/sh
cd lib; make distclean; cd -
cd src; make distclean; cd -
cd unicode; make clean; cd -
cd examples; make clean; cd -
cd test; make clean; cd -
