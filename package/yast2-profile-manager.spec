#
# spec file for package yast2-profile-manager
#
# Copyright (c) 2013 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


Name:           yast2-profile-manager
Version:        3.1.0
Release:        0

BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Source0:        %{name}-%{version}.tar.bz2

Group:          System/YaST
License:        GPL-2.0
BuildRequires:	gcc-c++ scpm-devel doxygen perl-XML-Writer yast2-core-devel yast2-users yast2-testsuite update-desktop-files libtool
BuildRequires:  yast2-devtools >= 3.0.6
# Wizard::SetDesktopTitleAndIcon
Requires:	yast2 >= 2.21.22
Requires:	yast2-users
Requires:	scpm >= 0.9.4

Requires:       yast2-ruby-bindings >= 1.0.0

Summary:	YaST2 - Profile Configuration

%description
This module is used to configure various settings for SCPM (System
Configuration Profile Management), which allows you to store different
profiles of your system configuration and switch between them.

%prep
%setup -n %{name}-%{version}

%build
%yast_build

%install
%yast_install


%files
%defattr(-,root,root)
%dir %{yast_yncludedir}/profile-manager
%{yast_yncludedir}/profile-manager/*
%{yast_clientdir}/profile-manager.rb
%{yast_clientdir}/profile_manager.rb
%{yast_moduledir}/ProfileManager.rb
%{yast_desktopdir}/profile-manager.desktop
%{yast_scrconfdir}/*.scr
%{yast_plugindir}/libpy2ag_scpm.so.*
%{yast_plugindir}/libpy2ag_scpm.so
%{yast_plugindir}/libpy2ag_scpm.la
%doc %{yast_docdir}
