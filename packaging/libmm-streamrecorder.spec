Name:       libmm-streamrecorder
Summary:    Media Stream Recorder library
Version:    0.0.4
Release:    0
Group:      Multimedia/Other
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(mm-common)
BuildRequires:  pkgconfig(mm-log)
BuildRequires:  pkgconfig(gstreamer-base-1.0)
BuildRequires:  pkgconfig(gstreamer-video-1.0)
BuildRequires:  pkgconfig(gstreamer-app-1.0)
BuildRequires:  pkgconfig(iniparser)

%description
This library is for making video/audio files with gstreamer


%package devel
Summary:    Common recorder development library
Group:      libdevel
Version:    %{version}
Requires:   %{name} = %{version}-%{release}

%description devel
Media Stream Recorder development library


%prep
%setup -q


%build
#export CFLAGS+=" -DGST_EXT_TIME_ANALYSIS"
export CFLAGS+=" -Wall -Wextra -Wno-array-bounds -Wno-empty-body -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow -Wwrite-strings -Wswitch-default -Wno-unused-but-set-parameter -Wno-unused-but-set-variable"
./autogen.sh
%configure --disable-static
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
%make_install

%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest libmm-streamrecorder.manifest
%defattr(-,root,root,-)
%{_bindir}/*
%{_libdir}/*.so.*
%{_datadir}/license/%{name}


%files devel
%defattr(-,root,root,-)
%{_includedir}/mmf/mm_streamrecorder.h
%{_libdir}/pkgconfig/mm-streamrecorder.pc
%{_libdir}/*.so
