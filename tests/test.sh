#!/bin/bash

################################################################################
#
#	Run interactive program to render regex DFA graphs
#	Requires graphviz dot tool
#
################################################################################

make -f Make test

history -r test_regex_history

while true; do
  read -r -p "regex> " -e regex
  case $regex in
    bye|exit|"")
      break
      ;;
    history)
      history
      ;;
    *)
      history -s -- "$regex"
      ./test "$regex"
      if [ $? -eq 0 ]; then
        dot -Tpdf dump.gv > dump.pdf
        open dump.pdf
	echo OK
      fi
      ;;
  esac
done
history -w test_regex_history
echo Bye
