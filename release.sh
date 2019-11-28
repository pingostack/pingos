#!/usr/bin/env bash
MAKE=1
CLEAN=0
INSTALL=0
REBUILD=0
INSTALL_PATH="/usr/local/"
SERVICENAME="pingos"
OPWD=`pwd`
UNINSTALL=0

while getopts 'mcirup:s:' OPT; do
    case $OPT in
        m)
            MAKE=1
            ;;
        c)
            CLEAN=1
            ;;
        i)
            INSTALL=1
            ;;
        u)
            UNINSTALL=1
            ;;
        r)
            REBUILD=1
            ;;
        p)
            INSTALL_PATH=$OPTARG
            ;;
        s)
            SERVICENAME=$OPTARG
            ;;
        ?)
            echo "Usage: `basename $0` [-m make] [-i install] [-c clean] [-p install path] [-s service name] [-r rebuild] [-u uninstall]"
            exit 1
    esac
done

SERVER_ROOT=$INSTALL_PATH$SERVICENAME
WWW_ROOT=$SERVER_ROOT"/html"

if [ $UNINSTALL == 1 ]
then
    while :
    do
        echo "remove $SERVER_ROOT [Y/N] "
        read YN
            case $YN in
            y)
                break
                ;;
            Y)
                break
                ;;
            yes)
                break
                ;;
            YES)
                break
                ;;
            n)
                exit 1
                ;;
            N)
                exit 1
                ;;
            no)
                exit 1
                ;;
            NO)
                exit 1
                ;;
            ?)
                echo "invalid input $YN"
        esac
    done

    echo "removing $SERVER_ROOT ..."
    if [ -d $SERVER_ROOT ]
    then
        rm -rf $SERVER_ROOT
    fi
    exit 1
fi

echo "server root: "$SERVER_ROOT
echo "www root: "$WWW_ROOT

scanOS(){
    OS=`uname -s`
    if [ ${OS} == "Darwin" ]
    then
        PM="brew"
    elif [ ${OS} == "Linux" ]
    then
        if grep -Eqi "CentOS" /etc/issue || grep -Eq "CentOS" /etc/*-release; then
            DISTRO='CentOS'
            PM='yum'
        elif grep -Eqi "Red Hat Enterprise Linux Server" /etc/issue || grep -Eq "Red Hat Enterprise Linux Server" /etc/*-release; then
            DISTRO='RHEL'
            PM='yum'
        elif grep -Eqi "Aliyun" /etc/issue || grep -Eq "Aliyun" /etc/*-release; then
            DISTRO='Aliyun'
            PM='yum'
        elif grep -Eqi "Fedora" /etc/issue || grep -Eq "Fedora" /etc/*-release; then
            DISTRO='Fedora'
            PM='yum'
        elif grep -Eqi "Debian" /etc/issue || grep -Eq "Debian" /etc/*-release; then
            DISTRO='Debian'
            PM='apt-get'
        elif grep -Eqi "Ubuntu" /etc/issue || grep -Eq "Ubuntu" /etc/*-release; then
            DISTRO='Ubuntu'
            PM='apt-get'
        elif grep -Eqi "Raspbian" /etc/issue || grep -Eq "Raspbian" /etc/*-release; then
            DISTRO='Raspbian'
            PM='apt-get'
        else
            DISTRO='unknow'
        fi
    else
        echo ${OS}
    fi

    echo ${PM}
}

modules=(
    "--with-http_ssl_module"
    "--add-module=../modules/nginx-rtmp-module"
    "--add-module=../modules/nginx-client-module"
    "--add-module=../modules/nginx-multiport-module"
    "--add-module=../modules/nginx-toolkit-module"
)

options=( "--prefix=$SERVER_ROOT" )

if [ $CLEAN == 1 ]
then
    if [ -d nginx ]
    then
        cd nginx
        echo "cleanup ..."
        make clean
        cd ..
    fi
    exit 1
fi

if [ $REBUILD == 1 ]
then
    if [ -d nginx ]
    then
        cd nginx
        if [ -e Makefile ]
        then
            make clean
        fi
        cd ..
    fi
fi

CMD=`scanOS`

if [ ! -d "./nginx/objs" ]
then
    # depend
    if [ "$CMD" == "yum" ]
    then
        yum install -y git gcc gcc-c++ openssl openssl-devel pcre-devel wget

    elif [ "$CMD" == "apt-get" ]
    then
        apt-get install -y wget
        apt-get install -y build-essential
        apt-get install -y libtool
        apt-get update
        apt-get install -y libpcre3 libpcre3-dev
        apt-get install -y zlib1g-dev
        apt-get install -y openssl

    elif [ "$CMD" == "brew" ]
    then
        brew install -y openssl pcre
        options[${#options[*]}]='--with-ld-opt="-L/usr/local/opt/openssl/lib/ -L/usr/local/opt/pcre/lib/"'
        options[${#options[*]}]='--with-cc-opt="-I/usr/local/opt/openssl/include/ -I/usr/local/opt/pcre/include/"'

    else
        echo "$CMD not supported"
        exit 1
    fi
fi

if [ ! -d "./nginx" ]
then
    wget https://nginx.org/download/nginx-1.17.5.tar.gz
    tar zxvf nginx-1.17.5.tar.gz
    mv nginx-1.17.5 nginx
    rm -f nginx-1.17.5.tar.gz
    cp resource/conf-template/nginx.conf nginx/conf/nginx.conf
fi

cd nginx

if [ ! -f "Makefile" ]
then
    config="./configure"
    for option in ${options[*]}
    do
        config=$config" "$option
    done

    for module in ${modules[*]}
    do
        config=$config" "$module
    done

    echo $config

    bash -c "$config"

fi

if [ -f "Makefile" ]
then
    if [ $MAKE == 1 ] || [ $REBUILD == 1 ] || [ $INSTALL == 1 ]
    then
        make -j4
    fi

    if [ $INSTALL ]
    then
        make install

        cd $OPWD

        if [ ! -f $SERVER_ROOT"/conf/nginx.conf" ]
        then
            cp ./resource/conf-template/nginx.conf $SERVER_ROOT"/conf/nginx.conf"
        fi

        if [ ! -f $WWW_ROOT"/stat.xsl" ]
        then
            cp ./resource/stat.xsl $WWW_ROOT"/stat.xsl"
        fi

        if [ ! -f $WWW_ROOT"/crossdomain.xml" ]
        then
            cp ./resource/crossdomain.xml $WWW_ROOT"/crossdomain.xml"
        fi

        if [ ! -d $WWW_ROOT"/h5player" ]
        then
            cd $WWW_ROOT
            git clone https://github.com/im-pingo/h5player.git
            cd $OPWD
        fi
    fi

fi

cd $OPWD
