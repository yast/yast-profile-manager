#
# Makefile.am for .../agent-scpm/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-scpm\" -Wall -lscpm

plugin_LTLIBRARIES = libpy2ag_scpm.la
noinst_LTLIBRARIES = liby2ag_scpm.la

liby2ag_scpm_la_SOURCES =				\
	SCPMAgent.cc					\
	SCPMAgent.h scdb.h scpm.h scpm_types.h
liby2ag_scpm_la_LDFLAGS = -version-info 2:0

libpy2ag_scpm_la_SOURCES =				\
        $(liby2ag_scpm_la_SOURCES)			\
        Y2CCSCPMAgent.cc         Y2CCSCPMAgent.h
libpy2ag_scpm_la_LDFLAGS = -version-info 2:0

INCLUDES = -I$(includedir)
