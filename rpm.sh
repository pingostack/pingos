#/bin/sh

function help()
{
    echo "Sample version:
    1.00.1
    1: major version
    00:version
    1: Bug "
    echo "usage: ./rpm.sh 1.00.1 service-name"
}

if [ $# -lt 2 ]
then
    help
    exit 1
fi

ver=$1
name=$2

echo $ver | grep '^[0-9]\+\.[0-9]\+\.[0-9]\+$'
if [ $? != "0" ];then
    help
    exit 1
fi

mkdir -p ./source/$ver
echo "copying source codes"

if [ ! -d "./nginx" ]; then
    exit 1
fi
cp -rf ./nginx ./source/$ver


if [ ! -d "./modules" ]; then
    exit 1
fi
cp -rf ./modules ./source/$ver


if [ ! -d "./h5player" ]; then
    exit 1
fi
cp -rf ./h5player ./source/$ver
cp -rf ./deps ./source/$ver
cp -rf ./resource ./source/$ver

cp -rf ./serv_conf ./source/$ver
cp -rf ./service ./source/$ver
cp -rf ./service.init ./source/$ver
cp -rf ./service.systemd ./source/$ver
cp -rf ./config.sh ./source/$ver

cd ./source
tar -zcvf ./$ver.tar.gz $ver --exclude .svn --exclude .git --exclude *.o --exclude *.oclint --exclude *.log --exclude *.pdf --exclude *.doc

cd ..
mkdir -p ./rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

cp ./source/$ver.tar.gz ./rpmbuild/SOURCES

rpmdir=$PWD/rpmbuild
echo "%_topdir  $rpmdir" > ~/.rpmmacros

echo "

Name:           $name
Version:        $ver
Release:        1%{?dist}
Source:         $ver.tar.gz

Summary:        $name from pingos
License:        GPL
URL:            http://www.pingos.io/
Packager:       pingos <pingos@pingos.io>
Group:          Application/Server
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{realease}-root
 
%description
The GNU wget program downloads files from the Internet using the command-line.
 
%prep
%setup -q -n $ver
cd $RPM_BUILD_DIR

%build
cd ./nginx

./configure --prefix=/usr/local/$name --with-http_ssl_module --add-module=../modules/nginx-rtmp-module --add-module=../modules/nginx-client-module --add-module=../modules/nginx-multiport-module --add-module=../modules/nginx-toolkit-module
make

%install
cd ./nginx
make DESTDIR=%{buildroot} install

cd ..
./serv_conf %{buildroot} $name

./config.sh %{buildroot} $name

%clean
cd ./nginx
make clean

%files
/
%defattr(-,root,root,0755)

" > ./rpmbuild/SPECS/rpmbuild.spec

echo "rpm will be build"
rpmbuild -bb ./rpmbuild/SPECS/rpmbuild.spec || exit 1
