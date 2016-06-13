#
# Spec file for package loganalyzer for openSUSE Linux and Fedora Linux
#
# Check for Linux distribution version numbers here:
# https://en.opensuse.org/openSUSE:Build_Service_cross_distribution_howto
#


Name:           loganalyzer
BuildRequires:  gcc gcc-c++ fdupes

# This is for all Fedora
%if 0%{?fedora}

BuildRequires:  qt5-qtbase
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtbase-gui
#BuildRequires:  qt5-qtwebkit-devel
BuildRequires:  qt5-qttools qt5-qttools-devel
BuildRequires:  qt5-qtsvg-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  desktop-file-utils
Requires:       qt5-qtsvg

%else
# This is for all SUSE

BuildRequires:  libqt5-qtbase-devel libqt5-qtdeclarative-devel libQt5Svg-devel
BuildRequires:  update-desktop-files 
Requires:       libQt5Svg5 libQt5Declarative5

%endif

License:        GPL-2.0
Group:          System/GUI/Productivity
Summary:        Tool that helps you analyzing your log files by reducing the content with regular expression patterns you define
Url:            https://github.com/pbek/loganalyzer/
Version:        VERSION-STRING
Release:        1
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Source0:        %{name}-%{version}.tar.xz


%description
LogAnalyzer is a tool that helps you analyzing your log files by reducing 
the content with regular expression patterns you define.

There is a an extension for eZ Publish (legacy) to download log files 
directly from the server.

Author
======
Patrizio Bekerle <patrizio@bekerle.com>


%prep
%setup -q 
mkdir build
pushd build
qmake-qt5 ..
popd

%build
echo centos_version 0%{?centos_version}
echo rhel_version   0%{?rhel_version}
echo fedora 0%{?fedora}
echo suse_version   0%{?suse_version}

pushd build
CFLAGS=$RPM_OPT_FLAGS CCFLAGS=$CFLAGS

%if 0%{?fedora}

make

%else

%make_jobs

%endif

popd

%install
pushd build
install -D -m 0755 LogAnalyzer $RPM_BUILD_ROOT/%{_prefix}/bin/LogAnalyzer
popd

# manually install desktop file for Fedora
%if 0%{?fedora}
install -D -m 0644 LogAnalyzer.desktop $RPM_BUILD_ROOT/%{_datadir}/applications/LogAnalyzer.desktop
%endif

install -D -m 0644 LogAnalyzer.png $RPM_BUILD_ROOT/%{_datadir}/pixmaps/LogAnalyzer.png
install -D -m 0644 LogAnalyzer.png $RPM_BUILD_ROOT/%{_datadir}/icons/hicolor/512x512/apps/LogAnalyzer.png
install -D -m 0644 LogAnalyzer.svg $RPM_BUILD_ROOT/%{_datadir}/icons/hicolor/scalable/apps/LogAnalyzer.svg

%if 0%{?suse_version}
%suse_update_desktop_file -c  LogAnalyzer LogAnalyzer LogAnalyzer LogAnalyzer LogAnalyzer "Utility;"
%endif

%fdupes $RPM_BUILD_ROOT/%{_prefix}

%clean  
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc LICENSE.txt README.md CHANGELOG.md
%{_bindir}/LogAnalyzer
%{_datadir}/pixmaps/LogAnalyzer.png

%{_datadir}/icons/hicolor/512x512/apps/LogAnalyzer.png
%{_datadir}/icons/hicolor/scalable/apps/LogAnalyzer.svg
%{_datadir}/applications/LogAnalyzer.desktop

%dir %{_datadir}/icons/hicolor/512x512/apps
%dir %{_datadir}/icons/hicolor/512x512
%dir %{_datadir}/icons/hicolor
%dir %{_datadir}/icons/scalable/apps
%dir %{_datadir}/icons/scalable
%dir %{_datadir}/LogAnalyzer

%changelog



