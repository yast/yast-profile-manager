#
# Makefile.am for .../agent-scpm/doc/autodocs
#

htmldir = $(docdir)/agent-scpm/autodocs/

html_DATA = index.html $(wildcard *.html)
CLEANFILES = $(html_DATA)

index.html: $(srcdir)/../../src/*.h
	cd $(srcdir)/../../src && @ybindir@/ydoc agent-scpm *.h
	mv -f $(srcdir)/../../src/*.html .
