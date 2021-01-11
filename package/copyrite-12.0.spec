Summary: CopyRight utility
%define AppProgram copyrite
%define AppLibrary td_lib
%define AppVersion 12.x
%define AppRelease 20210110
%define LibRelease 20210110
# $Id: copyrite-12.0.spec,v 1.11 2021/01/10 18:26:18 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: %{AppRelease}
License: MIT-X11
Group: Development/Tools
URL: ftp://ftp.invisible-island.net/ded
Source0: %{AppLibrary}-%{LibRelease}.tgz
Source1: %{AppProgram}-%{AppRelease}.tgz
Vendor: Thomas Dickey <dickey@invisible-island.net>

%description
Copyrite selectively comments source-files to prepend a copyright
notice.  This is used with US-ASCII text files, e.g., C source and
other languages.

%prep

# no need for debugging symbols...
%define debug_package %{nil}

# -a N (unpack Nth source after cd'ing into build-root)
# -b N (unpack Nth source before cd'ing into build-root)
# -D (do not delete directory before unpacking)
# -q (quiet)
# -T (do not do default unpacking, is used with -a or -b)
rm -rf %{AppProgram}-%{AppVersion}
mkdir %{AppProgram}-%{AppVersion}
%setup -q -D -T -a 1
mv %{AppProgram}-%{AppRelease}/* .
%setup -q -D -T -a 0

%build

cd %{AppLibrary}-%{LibRelease}

./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir} \
		--datadir=%{_datadir} \
		--disable-echo
make

cd ..
./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir} \
		--datadir=%{_datadir}
make

%install

[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/copyrite
%{_bindir}/copyrite.txt
%{_mandir}/man1/copyrite.*

%changelog
# each patch should add its ChangeLog entries here

* Sat Mar 24 2018 Thomas Dickey
- disable debug-package

* Sat Jul 03 2010 Thomas Dickey
- code cleanup

* Wed Jun 30 2010 Thomas Dickey
- initial version
