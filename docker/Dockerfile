FROM ubuntu:20.04 AS build
USER root
WORKDIR /home/
ENV DEBIAN_FRONTEND=noninteractive
RUN \
    apt update && apt-get upgrade -y && \
    apt-get install -y wget build-essential libtool libpcre3 libpcre3-dev zlib1g-dev openssl libssl-dev --fix-missing git gcc
COPY . /home/
RUN \
    cd /home && \
    chmod +x ./release.sh && \
    ./release.sh -ir

RUN cp -rf /home/docker/conf/ /usr/local/pingos/
RUN cp -r /home/docker/cert/ /cert/

FROM ubuntu:20.04 AS prod

ARG RUN_PATH=/usr/local/pingos
ARG BUILD_PATH=/home

WORKDIR ${RUN_PATH}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt-get upgrade -y && \
    apt-get install -y supervisor ffmpeg openssl libpcre3 libpcre3-dev zlib1g-dev libssl-dev

#RUN rm -f /etc/supervisor/supervisord.conf
ENV PATH=${PATH}:${RUN_PATH}/sbin
COPY --from=build ${RUN_PATH}/ ${RUN_PATH}/
COPY --from=build ${BUILD_PATH}/docker/cert/ /cert
COPY --from=build ${BUILD_PATH}/docker/supervisor/supervisord.conf /etc/supervisor/supervisord.conf
COPY --from=build ${BUILD_PATH}/docker/supervisor/pingos.conf /etc/supervisor/conf.d/pingos.conf
COPY --from=build ${BUILD_PATH}/docker/pingos-entrypoint.sh /usr/local/bin/pingos-entrypoint.sh

RUN mkdir -p /data/record && mkdir -p /usr/local/pingos/conf/conf.d/location && mkdir -p /usr/local/pingos/conf/conf.d/server
RUN chmod 777 /usr/local/bin/pingos-entrypoint.sh

CMD [ "/usr/bin/supervisord" ]
