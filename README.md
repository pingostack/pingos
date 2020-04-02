# PingOS

[![logo](doc/img/logo-banner-blue-400x200.jpeg)](https://pingos.io) 

[![Build Status](https://travis-ci.com/pingostack/pingos.svg?branch=master)](https://travis-ci.com/pingostack/pingos) [![website](https://img.shields.io/badge/website-https://pingos.io-red.svg)](https://pingos.io)

这是一个集众多流媒体协议为一身的流媒体服务器，目的是为了让开发者和不懂技术的普通人都能很快的构建自己的直播服务系统。

---

# 服务器功能

- HTTP(S)-FLV 播放
- HTTP(S)-TS 播放
- HLS+ 播放
- 多进程
- 动态（静态）回源（ HTTP(S)-FLV 和 rtmp协议 ）
- 动态（静态）转推 （ rtmp协议 ）
- HTTP-FLV回源拉流
- RTMP回源拉流
- 秒开功能
- GOP缓存
- VHOST功能
- application支持通配符
- H265编码
- mp3编码
- 服务端录像
- html5网页播放器集成
- 控制台接口
- 流量计费
- 配置动态加载

# 引导

- [项目文档](https://pingos.io/docs/zh/quick-start)

## 快速安装

- [使用Docker镜像](docker/README.md)

- 直接安装到系统
    ```bash
    # 快速安装
    git clone https://github.com/im-pingo/pingos.git

    cd pingos

    ./release.sh -i

    # 启动服务
    cd /usr/local/pingos/
    ./sbin/nginx
    ```

## 操作说明

### 推流

>  目前仅支持rtmp协议推流
>  推流地址：rtmp://ip/live/流名

### 播放地址

- rtmp 播放：rtmp://ip/live/流名

- http-flv 播放：http://ip/live/流名

- hls 播放：http://ip/hls/流名.m3u8

- hls+ 播放：http://ip/hls2/流名.m3u8

- http-ts 播放：http://ip/ts/流名

### 直播流监控后台

> 访问地址：http://ip/rtmp_stat
> 通过该页面可以查看当前正在发生的推流和播放记录。


### html5播放器

> 访问地址： http://ip/h5player/flv
> 这个播放器是基于flv.js的网页播放器，可以实现无插件播放http-flv直播流。
> 一旦你能够访问这个页面，说明你的直播服务器已经成功搭建起来了。

![h5player](doc/img/flvplayer.png)
<div class="article__content" markdown="1">
