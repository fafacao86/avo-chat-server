# avo-chat-server
AVO, a simple P2P and P2G real-time chatting app, server repo written in C and Java.<br>


![](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=&logoColor=white)
![Java](https://img.shields.io/badge/java-%23ED8B00.svg?style=for-the-badge&logo=java&logoColor=white)
![Spring](https://img.shields.io/badge/spring-%236DB33F.svg?style=for-the-badge&logo=spring&logoColor=white)
![Redis](https://img.shields.io/badge/redis-%23DD0031.svg?style=for-the-badge&logo=redis&logoColor=white)
![MySQL](https://img.shields.io/badge/mysql-%2300f.svg?style=for-the-badge&logo=mysql&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)<br>
## :pushpin:简介

AVO是一款轻量级的P2P和P2G即时聊天软件，服务端采用C和Java配合编写。<br><br>
C主要负责通信协议的编写，协议职责有如下三点：
* 心跳检测，定期发送检查客户端是否在线
* 通知客户端主动向Java服务端拉取新消息
* 文件传输
<br>
Java服务端采用SpringBoot框架，Mybatis作为ORM，接口设计遵循RESTful风格。<br>
接口文档：https://www.apifox.cn/apidoc/shared-04c58c23-00fc-4de2-81fd-131f625ed5bc
<br>
进程间通信使用Linux无名管道

## :pushpin:部署与运行
在项目路径/java/src/main/resources/docker-compose.yml为dockercompose文件<br>
### 1. 根据{}中的指示修改docker-compose
```text
version: '2.3'

services:
  mysql:
    restart: always
    image: mysql:5.7.39
    container_name: mysql5.7
    ports:
      - "3306:3306"
    volumes:
      - {mysql的数据从docker容器映射到物理机路径}:/var/lib/mysql
      - {mysql的配置文件从docker容器映射到物理机路径}:/etc/mysql/my.cnf
    command: mysqld --character-set-server=utf8mb4 --collation-server=utf8mb4_unicode_ci
    environment:
      - TZ=Asia/Shanghai
      - MYSQL_ROOT_PASSWORD={mysql root密码}
      - MYSQL_DATABASE={项目的数据库schema}
      - MYSQL_USER={普通用户名}
      - MYSQL_PASSWORD={普通用户密码}

  redis-server:
    restart: always
    image: redis:7
    container_name: redis-server
    command: /bin/bash -c 'redis-server --appendonly yes'
    sysctls:
      - net.core.somaxconn=65535
    ports:
      - "6379:6379"
    volumes:
      - {redis的数据从docker容器映射到物理机路径}:/data
      - {redis的配置文件从docker容器映射到物理机路径}:/etc/redis.conf
```

### 2.运行docker容器
* 终端cd进入docker-compose.yml所在文件夹
* 输入命令`sudo docker-compose up -d`，-d可以使其以daemon的身份运行，脱离终端。

### 3.打包java代码为jar包
* 终端cd进入/java/
* 运行`mvn clean package`，会生成/java/target/目录
* 复制/java/target中的jar包的全路径[例如:AVO-server-0.0.1-SNAPSHOT.jar]
* 修改配置文件中的SPRING_BOOT_JAR_PATH为对应全路径

### 4.编译运行C程序
* 终端cd进入/C/build/
* `cmake .. -DIF_TEST="OFF"`，-DIF_TEST跳过测试代码编译，如果需要运行测试代码，请不加该选项。
* `make`
* `./server_protocol -p 5000 -d`,-p代表监听端口，默认为5000，-d代表以daemon运行
C程序会自动fork并启动springboot server
注：参数和配置文件详细信息请参考下节。

## :pushpin:架构设计
