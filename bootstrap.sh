#!/bin/sh
echo
echo "Bootstrapping reflex Unicode scripts"
./build.sh || exit 1
mv -f unicode/block_scripts.cpp unicode/block_scripts.cpp.bak
mv -f unicode/language_scripts.cpp unicode/language_scripts.cpp.bak
mv -f unicode/letter_scripts.cpp unicode/letter_scripts.cpp.bak
cd unicode
make -f Make || exit 1
cd ..
./build.sh || exit 1
rm -f unicode/block_scripts.cpp.bak
rm -f unicode/language_scripts.cpp.bak
rm -f unicode/letter_scripts.cpp.bak
echo
echo "OK"
