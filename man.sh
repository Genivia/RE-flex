#!/bin/sh

# Generates man page doc/man/reflex.1 from reflex -h
# Robert van Engelen, Genivia Inc. All rights reserved.

if [ "$#" = 1 ]
then

if [ -x bin/reflex ] 
then

echo
echo "Creating reflex man page"
mkdir -p doc/man
echo '.TH REFLEX "1" "'`date '+%B %d, %Y'`'" "reflex '$1'" "User Commands"' > doc/man/reflex.1
cat >> doc/man/reflex.1 << 'END'
.SH NAME
\fBreflex\fR -- regex\-centric, fast and flexible lexical analyzer generator
.SH SYNOPSIS
.B reflex
[\fIOPTIONS\fR] [\fIFILE\fR]
.SH DESCRIPTION
Generates C++ source code programs that perform pattern\-matching on text.
\fIFILE\fR is a lexer specification source file, usually with extension .l.
Generates lex.yy.cpp unless option \fB-o\fR specifies otherwise.
.SH OPTIONS
END
bin/reflex -h \
| sed -e 's/\([^\\]\)\\/\1\\\\/g' \
| sed \
  -e '/^$/ d' \
  -e '/^Usage:/ d' \
  -e 's/^                //' \
  -e $'s/^        \(.*\)$/.TP\\\n  \\1/' \
  -e $'s/^    \(.*\)$/.PP\\\n.B \\1/' \
  -e 's/\(--[-+0-9A-Za-z_]*\)/\\fB\1\\fR/g' \
  -e 's/\([^-0-9A-Za-z_]\)\(-.\)/\1\\fB\2\\fR/g' \
  -e 's/\[=\([-0-9A-Za-z_]*\)\]/[=\\fI\1\\fR]/g' \
  -e 's/=\([-0-9A-Za-z_]*\)/=\\fI\1\\fR/g' \
| sed -e 's/-/\\-/g' >> doc/man/reflex.1
cat >> doc/man/reflex.1 << 'END'
.SH DEPENDENCIES
None, except when option \fB-m\fR specifies an external pattern\-matcher engine
such as pcre2 or boost that requires the corresponding library.
.SH "DOCUMENTATION"
The full documentation for \fBreflex\fR is maintained as a Doxygen\-generated
manual reflex/doc/html/index.html located in the source directory of the reflex
installation.  Or visit:
.IP
<https://www.genivia.com/doc/reflex/html/index.html>
.PP
to browse the latest version of the reflex user guide.
.SH COPYRIGHT
Copyright (c) 2016-2024 Robert A. van Engelen <engelen@acm.org>
.PP
\fBreflex\fR is released under the BSD\-3 license.  All parts of the software
have reasonable copyright terms permitting free redistribution.  This includes
the ability to reuse all or parts of the reflex source tree.
.SH BUGS
REJECT is not functional and not available.
.PP
Report bugs at: <https://github.com/Genivia/RE-flex/issues>
.SH "SEE ALSO"
lex(1), flex(1), flex++(1).
END

echo "reflex $1 manual page created and saved in doc/man/reflex.1"

else

echo "bin/reflex is needed but was not found: build reflex first"
exit 1

fi

else

echo "Usage: ./man.sh 1.v.v"
exit 1

fi
