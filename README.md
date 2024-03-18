# avo-chat-server

![](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=&logoColor=white)
![Java](https://img.shields.io/badge/java-%23ED8B00.svg?style=for-the-badge&logo=java&logoColor=white)
![Spring](https://img.shields.io/badge/spring-%236DB33F.svg?style=for-the-badge&logo=spring&logoColor=white)
![Redis](https://img.shields.io/badge/redis-%23DD0031.svg?style=for-the-badge&logo=redis&logoColor=white)
![MySQL](https://img.shields.io/badge/mysql-%2300f.svg?style=for-the-badge&logo=mysql&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)<br>

## :pushpin:简介

**本项目是作者大二上学期的一门课程作业**<br>
客户端Repo：https://github.com/xiaoheng86/avo-chat-client
<br>
AVO是一款支持私聊和群聊的即时聊天软件，服务端采用C和Java配合编写。<br><br>
C主要负责消息通知功能的编写，协议职责有如下三点：

* 心跳检测，定期发送检查客户端是否在线

* 通知客户端主动向Java服务端拉取新消息


  
  <br>
  Java服务端采用SpringBoot框架，接口设计遵循RESTful风格。<br>
  ApiFox接口文档：https://www.apifox.cn/apidoc/shared-04c58c23-00fc-4de2-81fd-131f625ed5bc
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
* 输入命令`sudo docker-compose up -d`

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
  <br>C程序会自动fork并启动springboot server
  <br>注：参数和配置文件详细信息请参考下节。

## :pushpin:架构设计

### 通信方式

![message flow](https://github.com/xiaoheng86/avo-chat-server/blob/main/readme/messageflow.jpg)<br>
&emsp;服务端C语言server使用原生socket，和客户端的C语言client-protocol通信，使用JSON格式交换信息。<br>

&emsp;当SpringBoot服务器收到一条新消息时，会向管道发送一条"notify_json"，包含了消息的类型是P2P还是P2G, pullerID和targetID，分别代表该谁拉消息和拉谁发的消息。server-protocol收到notify_json以后，会向puller客户端的C语言网络通信处理进程转发json，客户端的C语言程序再通过管道向python GTK客户端发消息，客户端再对Springboot server的/message/P2P或者/message/P2G接口发送GET请求，拉取最新消息。
<br>

### C server设计

&emsp;使用Linux epoll机制+非阻塞IO，统一事件源，事件驱动编程。<br>
主线程作为loop监听epoll事件，向线程池分发任务。<br>
在实现通信的同时，需要负责daemonize和启动springboot server，将管道fd通过命令行参数传递给springboot，springboot通过文件IO读写/proc/pid/fd/xx，对管道IO。

#### 信号处理

&emsp;开启一个单独线程，并使用`pthread_sigmask`将所有发往进程的信号重定向到该线程，通过self-piped trick将信号延迟到主循环loop里处理，主要目的是避免信号打断系统调用<br>

#### 定时任务

&emsp;使用定时器容器，本项目实现了一个简单的timer_list，将心跳检测统一到timer tick时处理。<br>




## :pushpin:开源资源

**hiredis**:&emsp;[Minimalistic C client for Redis](https://github.com/redis/hiredis)<br>
**cJSON**:&emsp;[Ultralightweight JSON parser in ANSI C](https://github.com/DaveGamble/cJSON)<br>
**C Thread Pool**:&emsp;[A minimal but powerful thread pool in ANSI C](https://github.com/Pithikos/C-Thread-Pool)<br>
**log.c**:&emsp;[A simple logging library implemented in C99](https://github.com/rxi/log.c)&nbsp;我在此日志库基础上添加了with_errno的功能。
