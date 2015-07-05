Summary: CopyRight utility
%define AppProgram copyrite
%define AppLibrary td_lib
%define AppVersion 12.x
%define AppRelease 20150704
%define LibRelease 20150704
# $Id: copyrite-12.0.spec,v 1.6 2015/07/04 20:02:12 tom Exp $
Name: %{AppProgram}
Version: %{AppVersion}
Release: %{AppRelease}
License: MIT-X11
Group: Development/Tools
URL: ftp://invisible-island.net/ded
Source0: %{AppLibrary}-%{LibRelease}.tgz
Source1: %{AppProgram}-%{AppRelease}.tgz
Vendor: Thomas Dickey <dickey@invisible-island.net>

%description
Copyrite selectively comments source-files to prepend a copyright
notice.  This is used with US-ASCII text files, e.g., C source and
other languages.

%prep

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

make install                    DESTDIR=$RPM_BUILD_ROOT

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/copyrite
%{_bindir}/copyrite.txt
%{_mandir}/man1/copyrite.*

%changelog
# each patch should add its ChangeLog entries here

* Sat Jul 03 2010 Thomas Dickey
- code cleanup

* Wed Jun 30 2010 Thomas Dickey
- initial version
