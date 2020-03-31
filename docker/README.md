## Pingos ##
详见[主页](http://pingos.io/)

## 文件说明 ##
- `conf/nginx.conf`: 配置文件
- `docker-compose.yml`: docker-compose配置文件
- `Dockerfile`: 构建镜像的构建文件
- `run.sh`: 配合docker/compose容器使用从而无需安装docker-compose

### 使用方法 ###

- 本地运行:
  ```sh
  # 添加可执行权限
  $ chmod +x run.sh
  # 编译镜像, 可选操作(./run.sh up -d时若镜像不存在则自动编译)
  $ ./run.sh build
  # 默认以同目录docker-compose.yml文件作为配置文件启动容器
  # 使用docker/compose容器启动容器
  $ ./run.sh up -d
  # 或
  $ docker-compose up -d
  ```

- 离线部署:
  ```sh
  # 在本机上导出镜像
  $ docker save pingos:latest -o pingos.latest.tar
  # 上传本文件夹到服务器上并导入镜像
  $ docker load -i pingos.latest.tar
  # 默认以同目录docker-compose.yml文件作为配置文件启动容器
  # 使用docker/compose容器启动容器
  $ ./run.sh up -d
  # 或
  $ docker-compose up -d
  ```
