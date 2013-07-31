# Sources for profile-manager

module_DATA = \
  modules/ProfileManager.rb

client_DATA = \
  clients/profile_manager.rb \
  clients/profile-manager.rb

yncludedir = @yncludedir@/profile-manager
ynclude_DATA = \
  include/profile-manager/wizards.rb \
  include/profile-manager/dialogs.rb \
  include/profile-manager/complex.rb \
  include/profile-manager/helps.rb

scrconf_DATA = \
  scrconf/cfg_scpm.scr

desktop_DATA = \
  desktop/profile-manager.desktop

EXTRA_DIST = $(module_DATA) $(client_DATA) $(ynclude_DATA) $(scrconf_DATA) $(desktop_DATA)

include $(top_srcdir)/Makefile.am.common