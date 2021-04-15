df_path=$1
service=$2

install_path=$df_path"/usr/local/"$service

cp -rf ./deps/* $install_path"/sbin"

cp ./resource/conf-template/nginx.conf $install_path"/conf/nginx.conf"

cp ./resource/stat.xsl $install_path"/html/stat.xsl"

cp ./resource/crossdomain.xml $install_path"/html/crossdomain.xml"

cp -r ./h5player $install_path"/html/"
