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

config_DATA =			\
	profile-manager.y2cc

desktop_DATA =			\
	profile-manager.desktop

module_DATA =			\
	ProfileManager.ycp

EXTRA_DIST = $(client_DATA) $(ynclude_DATA) $(config_DATA) $(module_DATA) $(desktop_DATA)

include $(top_srcdir)/Makefile.am.common
