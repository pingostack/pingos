FROM jrottenberg/ffmpeg:4-ubuntu

RUN \
    apt update && \
    apt install -y --fix-missing git gcc libssl-dev && \
    git clone --depth=1 https://github.com/im-pingo/pingos.git && \
    (cd pingos && \
    chmod +x ./release.sh && \
    ./release.sh -i) && \
    rm -rf pingos && \
    apt autoclean && apt autoremove && \
    rm -rf /var/lib/apt/lists/* && \
    rm -rf /var/cache/apt/* && \
    cp /usr/local/bin/ffmpeg /usr/bin

WORKDIR /usr/local/pingos/

ENTRYPOINT [ "./sbin/nginx" ]
CMD ["-g", "daemon off;"]

