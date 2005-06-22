#
# Makefile.am for yast2-profile-manager/src
#

yncludedir = @yncludedir@/profile-manager

client_DATA =			\
	profile-manager.ycp		\
	profile_manager.ycp

ynclude_DATA =			\
	helps.ycp		\
	wizards.ycp		\
	dialogs.ycp		\
	complex.ycp

desktop_DATA =			\
	profile-manager.desktop

module_DATA =			\
	ProfileManager.ycp

EXTRA_DIST = $(client_DATA) $(ynclude_DATA) $(module_DATA) $(desktop_DATA)

include $(top_srcdir)/Makefile.am.common
