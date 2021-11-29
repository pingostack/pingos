#!/bin/sh

exe="nginx"

if [ -n "$PINGOS_HLS_KEY_URL" ]; then
    sed "s!{hls_key_url}!$PINGOS_HLS_KEY_URL!g;" \
    /usr/local/pingos/conf/nginx-hls.conf \
    > /usr/local/pingos/conf/nginx-hls.conf.tmp

    rm -f /usr/local/pingos/conf/nginx-hls.conf
    mv /usr/local/pingos/conf/nginx-hls.conf.tmp /usr/local/pingos/conf/nginx-hls.conf
fi

conf="/usr/local/pingos/conf/nginx.conf"
if [ -n "$PINGOS_SERVICE" ]; then
    conf="/usr/local/pingos/conf/nginx-${PINGOS_SERVICE}.conf"
fi

$exe -c $conf
