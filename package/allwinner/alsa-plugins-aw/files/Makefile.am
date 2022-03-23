SUBDIRS =

if HAVE_AWRATE
SUBDIRS += aw
endif

if HAVE_AWEQUAL
SUBDIRS += p-eq
endif

EXTRA_DIST = gitcompile version COPYING.GPL m4/attributes.m4
AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

dist-hook:
	-chmod -R a+r $(distdir)
	@if ! test -z "$(AMTAR)"; then \
		$(AMTAR) --create --verbose --file=- $(distdir) | bzip2 -c -9 > $(distdir).tar.bz2 ; \
	else \
		$(TAR) --create --verbose --file=- $(distdir) | bzip2 -c -9 > $(distdir).tar.bz2 ; \
	fi
