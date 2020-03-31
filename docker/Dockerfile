FROM ubuntu:18.04

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
    rm -rf /var/cache/apt/*

WORKDIR /usr/local/pingos/

CMD ["./sbin/nginx", "-g", "daemon off;"]
