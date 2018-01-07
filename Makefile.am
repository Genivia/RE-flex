SUBDIRS = lib src

man1_MANS = doc/man/reflex.1

all-local:	cp2bin

.PHONY:		cp2bin

cp2bin:		$(abs_top_builddir)/src/reflex
		-mkdir -p $(top_builddir)/bin
		-cp -f $< $(top_builddir)/bin
		@echo
		@echo "OK"
		@echo

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

test:		$(abs_top_builddir)/src/reflex
		-cd tests; $(MAKE) && ./rtest
