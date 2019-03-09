#  Copyright (c) 2018, Michael B Tegtmeyer
#  Copyright (c) 2016, Robert van Engelen, Genivia Inc.
#
#  All rights reserved.
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#      * Neither the name of the author nor the names of its contributors may
#        be used to endorse or promote products derived from this software
#        without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
#  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
#  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

ACLOCAL_AMFLAGS = --install -I m4

if EXAMPLES
  MAYBE_EXAMPLES=examples
endif

if HAVE_DOXYGEN
  MAYBE_DOCS=doc
endif

SUBDIRS = src include tests $(MAYBE_DOCS) $(MAYBE_EXAMPLES)
DIST_SUBDIRS = src include tests doc examples


# Make sure the top level files and the files necessary to build the
# documentation are included in the distribution. Generated man files are
# included in the documentation but the Doxygen content is not.
EXTRA_DIST= \
  CODE_OF_CONDUCT.md \
  CONTRIBUTING.md \
  INSTALL.md \
  LICENSE.txt \
  README.md

install-data-hook:
		@echo " ______________________________________________________ "; \
		 echo "|                                                      |"; \
		 echo "| You have successfully built and installed reflex.    |"; \
		 echo "|                                                      |"; \
		 echo "| You can use the reflex command to generate scanners. |"; \
		 echo "| Link your generated scanner with -lreflex.           |"; \
		 echo "|                                                      |"; \
		 echo "| Thanks for using reflex.                             |"; \
		 echo "|                                                      |"; \
		 echo "|             https://github.com/Genivia/RE-flex       |"; \
		 echo "|             https://sourceforge.net/projects/re-flex |"; \
		 echo "|______________________________________________________|";
