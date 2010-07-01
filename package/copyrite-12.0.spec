Summary: CopyRight utility
%define AppVersion 20100630
%define LibVersion 20100624
# $Header: /users/source/archives/copyrite.vcs/package/RCS/copyrite-12.0.spec,v 1.1 2010/06/30 10:43:44 tom Exp $
Name: copyrite
Version: 12.x
# Base version is 12.x; rpm version corresponds to "Source1" directory name.
Release: %{AppVersion}
License: MIT-X11
Group: Applications/Editors
URL: ftp://invisible-island.net/ded
Source0: td_lib-%{LibVersion}.tgz
Source1: copyrite-%{AppVersion}.tgz
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
rm -rf copyrite-12.x
mkdir copyrite-12.x
%setup -q -D -T -a 1
mv copyrite-%{AppVersion}/* .
%setup -q -D -T -a 0

%build

cd td_lib-%{LibVersion}

./configure \
		--target %{_target_platform} \
		--prefix=%{_prefix} \
		--bindir=%{_bindir} \
		--libdir=%{_libdir} \
		--mandir=%{_mandir} \
		--datadir=%{_datadir}
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

* Wed Jun 30 2010 Thomas Dickey
- initial version
