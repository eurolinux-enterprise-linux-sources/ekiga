Summary:	A Gnome based SIP/H323 teleconferencing application
Name:		ekiga
Version:	4.0.1
Release:	1.2%{?dist}
License:	GPLv2+
Group:		Applications/Communications
URL:		http://www.ekiga.org/
Source0:	ftp://ftp.gnome.org/pub/gnome/sources/ekiga/4.0/%{name}-%{version}.tar.xz

BuildRequires:	ptlib-devel = 2.10.10
BuildRequires:	opal-devel = 3.10.10
BuildRequires:	alsa-lib-devel
BuildRequires:	avahi-devel
BuildRequires:	avahi-glib-devel
BuildRequires:	boost-devel
BuildRequires:	dbus-glib-devel
BuildRequires:	evolution-data-server-devel
BuildRequires:	expat-devel
BuildRequires:	gtk2-devel
BuildRequires:	GConf2-devel
BuildRequires:	libnotify-devel
BuildRequires:	libxml2-devel
BuildRequires:	libXv-devel
BuildRequires:	openldap-devel
BuildRequires:	openssl-devel
BuildRequires:	SDL-devel
BuildRequires:	speex-devel

BuildRequires:	desktop-file-utils
BuildRequires:	gettext
BuildRequires:	gnome-doc-utils
BuildRequires:	intltool
BuildRequires:	pkgconfig
BuildRequires:	scrollkeeper

Requires:	evolution-data-server
Requires:	dbus
Requires:	GConf2
Requires(pre):	GConf2
Requires(post):	GConf2
Requires(preun):GConf2
Requires(post):	scrollkeeper
Requires(postun):scrollkeeper


%description
Ekiga is a tool to communicate with video and audio over the internet.
It uses the standard SIP and H323 protocols.

%prep
%setup -q

# force regeneration to drop translations
rm ekiga.schemas

%build
CXXFLAGS="$RPM_OPT_FLAGS -DLDAP_DEPRECATED=1 -fPIC"
%configure --disable-scrollkeeper --with-boost-libdir=%{_libdir}
make %{?_smp_mflags}

%install
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
make install DESTDIR=%{buildroot}
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

rm -rf %{buildroot}/var/scrollkeeper

# Replace identical images in the help by links.
# This reduces the RPM size by several megabytes.
helpdir=%{buildroot}%{_datadir}/gnome/help/%{name}
for f in $helpdir/C/figures/*.png; do
  b="$(basename $f)"
  for d in $helpdir/*; do
    if [ -d "$d" -a "$d" != "$helpdir/C" ]; then
      g="$d/figures/$b"
      if [ -f "$g" ]; then
        if cmp -s $f $g; then
          rm "$g"; ln -s "../../C/figures/$b" "$g"
        fi
      fi
    fi
  done
done

desktop-file-install --vendor gnome \
  --dir=%{buildroot}%{_datadir}/applications \
  --delete-original \
  %{buildroot}%{_datadir}/applications/ekiga.desktop

%find_lang ekiga --with-gnome

#Remove libtool archives.
find %{buildroot} -name '*.la' -exec rm -f {} ';'

%pre
if [ "$1" -gt 1 ] ; then
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-uninstall-rule \
%{_sysconfdir}/gconf/schemas/ekiga.schemas > /dev/null || :
fi

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule \
%{_sysconfdir}/gconf/schemas/ekiga.schemas > /dev/null || :

scrollkeeper-update -q -o %{_datadir}/omf/%{name} || :

touch --no-create %{_datadir}/icons/hicolor
if [ -x %{_bindir}/gtk-update-icon-cache ] ; then
  %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi

%preun
if [ "$1" -eq 0 ] ; then
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-uninstall-rule \
%{_sysconfdir}/gconf/schemas/ekiga.schemas > /dev/null || :
fi

touch --no-create %{_datadir}/icons/hicolor
if [ -x %{_bindir}/gtk-update-icon-cache ] ; then
  %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi

%postun
scrollkeeper-update -q || :

%files -f ekiga.lang
%defattr(-,root,root)
%doc COPYING AUTHORS FAQ NEWS
%{_bindir}/ekiga
%{_bindir}/ekiga-helper
%{_bindir}/ekiga-config-tool
%{_libdir}/ekiga
%{_datadir}/applications/gnome-ekiga.desktop
%{_datadir}/pixmaps/ekiga
%{_datadir}/man/*/*
%{_datadir}/sounds/ekiga
%{_datadir}/dbus-1/services/org.ekiga.*
%{_datadir}/icons/hicolor/*/apps/ekiga.png
%{_sysconfdir}/gconf/schemas/ekiga.schemas

%changelog
* Tue Apr 30 2013 Daniel Mach <dmach@redhat.com> - 4.0.1-1.2
- Rebuild for cyrus-sasl

* Wed Feb 20 2013 Peter Robinson <pbrobinson@fedoraproject.org> 4.0.1-1
- Ekiga 4.0.1 stable release - Changelog
  http://ftp.gnome.org/pub/gnome/sources/ekiga/4.0/ekiga-4.0.1.news

* Sat Feb 09 2013 Denis Arnaud <denis.arnaud_fedora@m4x.org> - 4.0.0-3
- Rebuild for Boost-1.53.0

* Tue Dec 25 2012 Bruno Wolff III <bruno@wolff.to> 4.0.0-2
- Rebuild for libcamel soname bump

* Mon Nov 26 2012 Peter Robinson <pbrobinson@fedoraproject.org> 4.0.0-1
- Ekiga 4.0.0 stable release - Changelog
  http://ftp.gnome.org/pub/gnome/sources/ekiga/4.0/ekiga-4.0.0.news

* Tue Nov 20 2012 Milan Crha <mcrha@redhat.com> - 3.9.90-3
- Rebuild against newer evolution-data-server

* Thu Oct 25 2012 Milan Crha <mcrha@redhat.com> - 3.9.90-2
- Rebuild against newer evolution-data-server

* Sat Aug 25 2012 Peter Robinson <pbrobinson@fedoraproject.org> - 3.9.90-1
- Ekiga 3.9.90 devel - Changelog
  ftp://ftp.gnome.org/pub/gnome/sources/ekiga/3.9/ekiga-3.9.90.news

* Wed Jul 18 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.3.2-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Sat Mar 24 2012 Peter Robinson <pbrobinson@fedoraproject.org> - 3.3.2-7
- Fix build with gcc 4.7

* Tue Feb 28 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.3.2-6
- Rebuilt for c++ ABI breakage

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.3.2-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Mon Nov 21 2011 Peter Robinson <pbrobinson@fedoraproject.org> - 3.3.2-4
- Rebuild for boost 1.48

* Sun Oct 30 2011 Bruno Wolff III <bruno@wolff.to> - 3.3.2-3
- Rebuild against newer evolution-data-server

* Mon Aug 29 2011 Milan Crha <mcrha@redhat.com> - 3.3.2-2
- Rebuild against newer evolution-data-server

* Tue Aug 23 2011 Peter Robinson <pbrobinson@fedoraproject.org> - 3.3.2-1
- Ekiga 3.3.2 devel - Changelog
  ftp://ftp.gnome.org/pub/gnome/sources/ekiga/3.3/ekiga-3.3.2.news

* Tue Aug 16 2011 Milan Crha <mcrha@redhat.com> - 3.3.1-3
- Rebuild against newer evolution-data-server

* Sun Jul 24 2011 Peter Robinson <pbrobinson@fedoraproject.org> - 3.3.1-2
- Rebuild for new boost and evolution-data-server

* Sat Jul 16 2011 Peter Robinson <pbrobinson@fedoraproject.org> - 3.3.1-1
- Ekiga 3.3.1 devel - Changelog
  ftp://ftp.gnome.org/pub/gnome/sources/ekiga/3.3/ekiga-3.3.1.news

* Fri Jun 17 2011 Milan Crha <mcrha@redhat.com> - 3.3.0-10
- Rebuild against newer evolution-data-server

* Fri May 20 2011 Kalev Lember <kalev@smartlink.ee> - 3.3.0-9
- Rebuilt for libcamel soname bump

* Tue Apr 12 2011 Peter Robinson <pbrobinson@fedoraproject.org> - 3.3.0-8
- rebuild again for new boost

* Tue Mar 15 2011 Peter Robinson <pbrobinson@fedoraproject.org> - 3.3.0-7
- rebuild for new boost

* Tue Feb 08 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.3.0-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Mon Feb 07 2011 Thomas Spura <tomspur@fedoraproject.org> - 3.3.0-5
- rebuild for new boost

* Tue Feb 01 2011 Milan Crha <mcrha@redhat.com> - 3.3.0-4
- Rebuild against newer evolution-data-server

* Wed Jan 12 2011 Milan Crha <mcrha@redhat.com> - 3.3.0-3
- Rebuild against newer evolution-data-server

* Fri Dec 24 2010 Dan Horák <dan[at]danny.cz> - 3.3.0-2
- fix build on non-x86 64-bit architectures (ax_boost_base.m4 is wrong)

* Thu Dec 23 2010 Peter Robinson <pbrobinson@fedoraproject.org> - 3.3.0-1
- Ekiga 3.3.0 devel - Changelog
  http://mail.gnome.org/archives/ekiga-devel-list/2010-December/msg00036.html

* Fri Nov  5 2010 Matthias Clasen <mclasen@redhat.com> - 3.2.7-5
- Rebuild against libnotify 0.7.0

* Mon Jul 26 2010 Caolán McNamara <caolanm@redhat.com> - 3.2.7-4
- add gtk flags to notify plugin to rebuild

* Tue Jul 20 2010 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.7-3
- rebuild against new evolution-data-server

* Thu Jul 15 2010 Matthias Clasen <mclasen@redhat.com> - 3.2.7-2
- rebuild against new evolution-data-server

* Mon May 31 2010 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.7-1
- Ekiga 3.2.7 stable - Changelog
  ftp://ftp.gnome.org/pub/gnome/sources/ekiga/3.2/ekiga-3.2.7.news

* Wed May 26 2010 Peter Robinson <pbrobinson@fedoraproject.org> 3.2.6-4
- Bump build for new evolution

* Tue May  4 2010 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.6-3
- Rebuild for new evolution

* Wed Mar  3 2010 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.6-2
- Add patch to fix DSO linking. Bug 564828

* Tue Sep 22 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.6-1
- Ekiga 3.2.6 stable - Changelog
  ftp://ftp.gnome.org/pub/gnome/sources/ekiga/3.2/ekiga-3.2.6.news

* Fri Aug 21 2009 Tomas Mraz <tmraz@redhat.com> - 3.2.5-4
- rebuilt with new openssl

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.2.5-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Thu Jul  9 2009 Matthias Clasen <mclasen@redhat.com> - 3.2.5-2
- Shrink GConf schemas
 
* Mon Jul  6 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.5-1
- Ekiga 3.2.5 stable - Changelog
  ftp://ftp.gnome.org/pub/gnome/sources/ekiga/3.2/ekiga-3.2.5.news

* Wed May 20 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.4-1
- Ekiga 3.2.4 stable - Changelog
  http://mail.gnome.org/archives/ekiga-devel-list/2009-May/msg00062.html
  http://mail.gnome.org/archives/ekiga-devel-list/2009-May/msg00064.html

* Tue May 19 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.1-1
- Ekiga 3.2.1 stable - Changelog
  http://mail.gnome.org/archives/ekiga-devel-list/2009-May/msg00054.html

* Mon Apr 27 2009 Matthias Clasen <mclasen@redhat.com> - 3.2.0-3
- Rebuild against newer GConf/intltool

* Mon Apr 20 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.0-2
- Add a couple of upstream patches from 3.2.1

* Tue Mar 17 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.2.0-1
- Ekiga 3.2.0 stable

* Fri Mar  6 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.2-4
- Remove CELT until the bitstream is stable and can hence intercommunicate between versions

* Tue Mar  3 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.2-3
- Remove autoconf bits

* Tue Mar  3 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.2-2
- Disable xcap for the moment so ekiga builds

* Tue Mar  3 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.2-1
- Upgrade to the 3.1.2 beta release, enable celt codec, reinstate 
  proper desktop file now its fixed

* Tue Feb 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 3.1.0-11
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Fri Jan 16 2009 Tomas Mraz <tmraz@redhat.com> - 3.1.0-10
- rebuild with new openssl
- add libtoolize call to replace libtool with current version

* Thu Jan 15 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-9
- Add other buildreq for Makefile regen

* Thu Jan 15 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-8
- Regen Makefile.in using autoreconf due to patch

* Wed Jan 14 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-7
- Another fix

* Tue Jan 13 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-6
- And SDL too

* Tue Jan 13 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-5
- Add expat-devel, why not everything else wants it

* Tue Jan 13 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-4
- Disable gstreamer support until there's a new gst-plugins-base

* Tue Jan 13 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-3
- Proper fix from upstream for desktop file

* Wed Jan  7 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-2
- Fix issues with the desktop file

* Mon Jan  5 2009 Peter Robinson <pbrobinson@fedoraproject.org> - 3.1.0-1
- Upgrade to the 3.1.0 devel release, enable gstreamer and xcap, remove libgnome

* Mon Nov 13 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 3.0.1-4
- Fix spec file error

* Mon Nov 13 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 3.0.1-3
- Patch to fix libnotify's breakage of its api

* Mon Oct 20 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 3.0.1-2
- Fix dependency issue

* Mon Oct 20 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 3.0.1-1
- Update to 3.0.1

* Thu Oct 9 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 3.0.0-5
- Remove gnomemeeting obsolete, package review updates

* Thu Oct 9 2008 Matthias Clasen  <mclasen@redhat.com> - 3.0.0-4
- Save some space

* Thu Oct 2 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 3.0.0-3
- require dbus

* Tue Sep 23 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 3.0.0-2
- add libnotify-devel as a build dep

* Tue Sep 23 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 3.0.0-1
- Ekiga 3 final release

* Sun Sep 14 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 2.9.90-3
- more rawhide build fixes

* Sun Sep 14 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 2.9.90-2
- rawhide build fixes

* Thu Sep 11 2008 Peter Robinson <pbrobinson@fedoraproject.org> - 2.9.90-1
- First beta of ekiga 3

* Mon May 12 2008 Paul W. Frields <stickster@gmail.com> - 2.0.12-2
- Rebuild against new opal (#441202)

* Thu Mar 13 2008 Daniel Veillard <veillard@redhat.com> - 2.0.12-1.fc9
- Upgrade to ekiga-2.0.12

* Thu Feb 28 2008 Daniel Veillard <veillard@redhat.com> - 2.0.11-4
- rebuild after applying some fo the cleanups of #160727

* Wed Feb 20 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 2.0.11-3
- Autorebuild for GCC 4.3

* Thu Dec 13 2007 Matěj Cepl <mcepl@redhat.com> 2.0.11-2
- compile with the D-Bus support
- Making rpmlint silent.

* Tue Sep 18 2007 Daniel Veillard <veillard@redhat.com> - 2.0.11-1
- Upgrade to ekiga-2.0.11

* Sun Apr 15 2007 Daniel Veillard <veillard@redhat.com> - 2.0.9-1
- Upgrade to ekiga-2.0.9

* Mon Mar 12 2007 Daniel Veillard <veillard@redhat.com> - 2.0.7-1
- Upgrade to ekiga-2.0.7

* Mon Feb 19 2007 Jeremy Katz <katzj@redhat.com> - 2.0.5-2
- rebuild 

* Wed Feb 14 2007 Daniel Veillard <veillard@redhat.com> - 2.0.5-1
- Upgrade to ekiga-2.0.5

* Mon Jan 22 2007 Daniel Veillard <veillard@redhat.com> - 2.0.4-1
- Upgrade to ekiga-2.0.4

* Thu Nov  2 2006 Daniel Veillard <veillard@redhat.com> - 2.0.3-3
- Resolves: rhbz#201535
- fixes build-requires for opal-devel and pwlib-devel

* Sat Oct 28 2006 Matthias Clasen <mclasen@redhat.com> - 2.0.3-2
- Rebuild against evolution-data-server 1.9

* Sat Oct 21 2006 Matthias Clasen <mclasen@redhat.com> - 2.0.3-1
- Update to 2.0.3

* Sat Sep 30 2006 Matthias Clasen <mclasen@redhat.com> - 2.0.2-7
- Make the status icon work in transparent panels

* Thu Aug 31 2006 Matthias Clasen <mclasen@redhat.com> - 2.0.2-6
- Fix translator credits (197871)

* Mon Aug  7 2006 Matthew Barnes <mbarnes@redhat.com> - 2.0.2-5
- Rebuild against evolution-data-server-1.7.91

* Sat Aug  5 2006 Caolán McNamara <caolanm@redhat.com> - 2.0.2-4
- rebuild against new e-d-s

* Tue Aug  1 2006 Daniel Veillard <veillard@redhat.com> - 2.0.2-3
- rebuilt for #200960

* Wed Jul 12 2006 Jesse Keating <jkeating@redhat.com> - 2.0.2-1.1
- rebuild

* Wed May 31 2006 Daniel Veillard <veillard@redhat.com> - 2.0.2-1
- new release of ekiga 2.0.2
- activating Zeroconf support though avahi

* Mon May 22 2006 Jesse Keating <jkeating@redhat.com> - 2.0.1-3
- Fix BuildRequires and Requires(post), Requires(postun)

* Wed Mar 15 2006 Daniel Veillard <veillard@redhat.com> - 2.0.1-2
- run 'ekiga-config-tool --install-schemas' in %%post, c.f. #178929

* Tue Mar 14 2006 Daniel Veillard <veillard@redhat.com> - 2.0.1-1
- last minute bug rerelease 2.0.1

* Mon Mar 13 2006 Daniel Veillard <veillard@redhat.com> - 2.0.0-1
- final release of 2.0.0

* Mon Feb 20 2006 Karsten Hopp <karsten@redhat.de> 1.99.1-2
- Buildrequires: gnome-doc-utils

* Mon Feb 13 2006 Daniel Veillard <veillard@redhat.com> - 1.99.1-1
- new beta release issued

* Fri Feb 10 2006 Jesse Keating <jkeating@redhat.com> - 1.99.0-3.2
- bump again for double-long bug on ppc(64)

* Tue Feb 07 2006 Jesse Keating <jkeating@redhat.com> - 1.99.0-3.1
- rebuilt for new gcc4.1 snapshot and glibc changes

* Sat Jan 28 2006 Daniel Veillard <veillard@redhat.com> - 1.99.0-3
- Rebuilt following a libedataserver revision

* Fri Jan 27 2006 Matthias Clasen <mclasen@redhat.com> - 1.99.0-2
- Use the upstream .desktop file

* Tue Jan 24 2006 Daniel Veillard <veillard@redhat.com> - 1.99.0-1
- initial version based on the 1.99.0 beta and gnomemeeting spec file.
