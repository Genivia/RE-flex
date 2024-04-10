SUBDIRS = lib src . tests @EXAMPLESDIR@

man1_MANS = doc/man/reflex.1

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = reflex.pc reflexmin.pc

EXTRA_DIST = README.md LICENSE.txt CONTRIBUTING.md CODE_OF_CONDUCT.md reflex.pc.in reflexmin.pc.in

all-local:	cp2bin

.PHONY:		cp2bin

# to copy the reflex binary to reflex/bin
cp2bin:		$(top_builddir)/src/reflex
		-mkdir -p $(top_builddir)/bin
		-cp -f $< $(top_builddir)/bin
		@echo
		@echo "OK"
		@echo

# to generate the documentation: make doc/html
libdocdir = @DOXYGEN_OUTPUT_DIRECTORY@
DOXYGEN = @DOXYGEN@
$(libdocdir):	$(top_builddir)/doc/Doxyfile $(top_builddir)/doc/index.md
		if test -d $(libdocdir); then \
		  rm -Rf $(libdocdir); \
		fi
		if test "$(DOXYGEN)" != NO_DOXYGEN; then \
		  cd doc; \
		  $(DOXYGEN); \
		else \
		  mkdir $(libdocdir); \
		  echo "No doxygen available -- library documentation not generated" >$(libdocdir)/stamp; \
		fi

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

.PHONY:		test

test:		$(top_builddir)/src/reflex
		-cd tests; $(MAKE) && ./rtest
