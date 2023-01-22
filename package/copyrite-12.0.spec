Summary: CopyRight utility
%define AppProgram copyrite
%define AppLibrary td_lib
%define AppVersion 12.x
%define AppRelease 20230122
%define LibRelease 20230122
# $Id: copyrite-12.0.spec,v 1.15 2023/01/22 23:56:07 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: %{AppRelease}
License: MIT-X11
Group: Development/Tools
URL: https://invisible-island.net/ded
Source0: https://invisible-island.net/archives/ded/%{AppProgram}-%{AppRelease}.tgz
BuildRequires: td_lib <= %{AppRelease}
Vendor: Thomas Dickey <dickey@invisible-island.net>

%description
Copyrite selectively comments source-files to prepend a copyright
notice.  This is used with US-ASCII text files, e.g., C source and
other languages.

%prep

# no need for debugging symbols...
%define debug_package %{nil}

%setup -q -n %{AppProgram}-%{AppRelease}

%build

./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir} \
		--datadir=%{_datadir} \
		--disable-echo
make

%install

[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/copyrite
%{_bindir}/copyrite.txt
%{_mandir}/man1/copyrite.*

%changelog
# each patch should add its ChangeLog entries here

* Thu Jan 19 2023 Thomas Dickey
- build against td_lib package rather than side-by-side configuration

* Sat Mar 24 2018 Thomas Dickey
- disable debug-package

* Sat Jul 03 2010 Thomas Dickey
- code cleanup

* Wed Jun 30 2010 Thomas Dickey
- initial version
