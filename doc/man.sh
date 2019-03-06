#!/bin/sh

# Generates man page from reflex -h
# Usage: man.sh REFLEX MANFILE VERSION
#   where:
#     REFLEX is a reflex executable
#     MANFILE is the manfile to be generated
#     VERSION is the version string, ie major.minor.micro
#
# Robert van Engelen, Genivia Inc. All rights reserved.


if [ "$#" = 3 ]
then

REFLEX=${1}
MANFILE=${2}
VERSION=${3}


if [ -x $REFLEX ]
then

echo
echo "Creating reflex man page"
echo '.TH REFLEX "1" "'`date '+%B %d, %Y'`'" "reflex '${VERSION}'" "User Commands"' > ${MANFILE}
cat >> ${MANFILE} << END
.SH NAME
reflex \- the regex\-centric fast and flexible lexical analyzer generator
.SH SYNOPSIS
.B reflex
[\fIOPTIONS\fR] [\fIFILE\fR]
.SH DESCRIPTION
Generates C++ source code programs that perform pattern\-matching on text.
\fIFILE\fR is a lexer specification source file, usually with extension .l.
Generates lex.yy.cpp unless option \fB-o\fR specifies otherwise.
.SH OPTIONS
END
${REFLEX} -h \
| sed -e 's/\([^\\]\)\\/\1\\\\/g' \
| sed \
  -e '/^$/ d' \
  -e '/^Usage:/ d' \
  -e 's/^                //' \
  -e $'s/^        /.TP\\\n/' \
  -e 's/^   \(.*\)$/.SS "\1"/' \
  -e 's/\(--[-+0-9A-Za-z_]*\)/\\fB\1\\fR/g' \
  -e 's/\([^-0-9A-Za-z_]\)\(-.\)/\1\\fB\2\\fR/g' \
  -e 's/\[=\([-0-9A-Za-z_]*\)\]/[=\\fI\1\\fR]/g' \
  -e 's/=\([-0-9A-Za-z_]*\)/=\\fI\1\\fR/g' \
| sed -e 's/-/\\-/g' >> ${MANFILE}
cat >> ${MANFILE} << END
.SH DEPENDENCIES
None, except when option \fB-m\fR specifies an external pattern\-matcher engine
such as boost that requires the Boost.Regex library installed.
.SH "DOCUMENTATION"
The full documentation for \fBreflex\fR is maintained as a Doxygen\-generated
manual reflex/doc/html/index.html located in the source directory of the reflex
installation.  Or visit:
.IP
.B http://re-flex.sourceforge.net
.PP
to browse the reflex user guide.
.SH BUGS
REJECT is not functional and not available.

Report bugs at:
.IP
.B https://github.com/Genivia/RE-flex/issues
.PP
.SH AUTHOR
Robert van Engelen <engelen@acm.org> was the original author of \fBreflex\fR.
.SH LICENSE
\fBreflex\fR is released under the BSD\-3 license.  All parts of the software
have reasonable copyright terms permitting free redistribution.  This includes
the ability to reuse all or parts of the reflex source tree.
.SH "SEE ALSO"
lex(1), flex(1), flex++(1).
END

echo "reflex manual page created from ${REFLEX} and saved in ${MANFILE}"

else

echo "${REFLEX} not found"
exit 1

fi

else

echo "Usage: ./man.sh REFLEX MANFILE VERSION"
exit 1

fi
