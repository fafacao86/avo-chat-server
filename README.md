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
<br>C程序会自动fork并启动springboot server
<br>注：参数和配置文件详细信息请参考下节。

## :pushpin:架构设计
### 通信方式
![message flow](https://github.com/xiaoheng86/avo-chat-server/blob/main/readme/messageflow.jpg)<br>
&emsp;服务端C语言server-protocol使用原生socket，和客户端的C语言client-protocol通信，使用JSON格式交换信息。<br>
buffer设计为6位length+最高65536的消息体，注，消息中不含'\0'<br>
例：000010{"type":1}<br>
<br>
&emsp;服务端C语言server-protocol与springboot server采用Linux无名管道通信，同样使用JSON格式交换信息。<br>
buffer设计为4位length+最高1024位的消息体，注，消息中不含'\0'<br>
例：0010{"type":1}<br>
<br>
&emsp;当SpringBoot服务器收到一条新消息时，会向管道发送一条"notify_json"，包含了消息的类型是P2P还是P2G, pullerID和targetID，分别代表该谁拉消息和拉谁发的消息。server-protocol收到notify_json以后，会向puller客户端的client-protocol转发json，client-protocol同样会向electron客户端转发json。这里electron里的nodejs可以简单地像文件IO一样，注册回调函数，读管道，回调函数里对Springboot server的/message/P2P或者/message/P2G接口发送GET请求，拉取最新消息。
<br>
### server-protocol设计
&emsp;使用Linux epoll机制+非阻塞IO，统一事件源，事件驱动编程。<br>
主线程作为loop监听epoll事件，向线程池分发任务。<br>
在实现通信的同时，需要负责daemonize和启动springboot server，将管道fd通过命令行参数传递给springboot，springboot通过文件IO读写/proc/pid/fd/xx，对管道IO。
#### 信号处理
&emsp;开启一个单独线程，并使用`pthread_sigmask`将所有发往进程的信号重定向到该线程，通过self-piped trick将信号延迟到主循环loop里处理，主要目的是避免信号打断系统调用等等，以及一些undefined现象。<br>

#### 定时任务
&emsp;使用定时器容器，本项目实现了一个简单的timer_list，将超时处理，心跳检测等统一到timer tick时处理。<br>

#### 线程安全
&emsp;根据需求，只有3种类型的消息会在socket连接间传输，分别是：heartbeat, notify, file。鉴于消息种类少，在本项目设计中，让每个客户端保持与服务端3个socket连接，分别处理3种类型的消息，于是我们在服务器中维护一个全局pthread_mutex数组，每个锁对应一个服务端的fd，每次在socket_fd上进行IO操作时需要加上对应的锁，**使得同一时间只有一个线程读写一个socket_fd**，这也将减轻我们编写代码的难度，比如可以用上EPOLL的ONESHOT事件，还有线程安全地关闭连接。<br>
<br>

##### 怎样安全的关闭连接?<br>
&emsp;本项目中只提供1个用于关闭连接的对外接口函数：`void close_client_connection(int heartbeat_fd);`，在这里我们以心跳fd代表一个在线客户端<br>
&emsp;整个项目中只有两处地方可能会调用该函数：一处是心跳检测超时。另一处是客户端主动向springboot发起logout请求,springboot通过管道通知客户端主动关闭了连接。这两个调用点都是在非socket IO线程中。
<br>问题就来了，如何在一个非socket IO线程中安全地关闭另一个线程可能正在读写的socket_fd？<br>


&emsp;我的主要思路是，向目标线程发送SIGUSR1，打断其IO系统调用，项目中提供的如`get_json_from_socket`和`send_json_to_socket`封装了`readn`和`writen`函数，这两个函数又封装了`read`/`write`系统调用，当信号打断IO后，对IO的一层一层封装的上层函数需要像异常返回链一样向调用者返回错误，及时return。<br>

&emsp;在目标线程执行SIGUSR1的handler中close socket，也就是把close fd这个操作交由socket IO线程自己进行，在`close(fd)`之前一定要再加上一个锁，**这个锁避免了关闭的过程还没完全完成,又有accept新连接把刚close掉的fd占用了**。那问题又来了，这个锁该在哪里unlock呢，我们应该在关闭过程完全结束时unlock，那到底什么叫做“关闭过程完全结束”?简单说就是在线程池worker_function返回的时候。<br>
以下通过部分代码阐述notify_fd的关闭逻辑：<br>
在`close_client_connection`里先看closing flag有没有被设置，如果设置了则退出，将if和set为1这两步用互斥锁保护起来，作为原子操作。
``` C
// source/base/socket_base/close_client_connection函数

/**
* CLOSE_FLAGS是一个全局结构体数组
* 结构体中有:
*   int closing;  //closing flag，同一时间，对同一客户端的关闭过程只能有一个在进行。
*   pthread_t tid; //哪个线程正在对fd读写操作，如果fd当前没有线程在对其IO，则tid = -1; 当线程开始对fd读写时，会将tid设置为他的线程ID。
*/
pthread_mutex_lock(&CLOSE_MUTEX[heartbeat_fd]);
if (CLOSE_FLAGS[heartbeat_fd].closing == 1){
pthread_mutex_unlock(&CLOSE_MUTEX[heartbeat_fd]);
return;
}
CLOSE_FLAGS[heartbeat_fd].closing = 1;
pthread_mutex_unlock(&CLOSE_MUTEX[heartbeat_fd]);
//这一步保证了，直到线程池的worker_function返回只有一个closing过程，也就是说我们关闭过程中不用考虑并发。What a relief!



/**
* 省略部分代码，客户端一连接会在redis里维护一个hash，可以借助redis，通过heartbeat_fd获取到同一客户端的notify_fd与file_fd。需要一并关闭。
*/

pthread_mutex_lock(&CLOSE_MUTEX[notify_fd]);  //加锁保护CLOSE_FLAG
if (CLOSE_FLAGS[notify_fd].tid == -1){  //如果没有worker_thread在对fd进行IO，则直接close掉socket fd
    CLOSE_FLAGS[notify_fd].closing = 1;  //设置关闭进行标志
    close(notify_fd);
    event_loop_del(EVENT_LOOP, notify_fd);  //移除epoll监听
}else{
    CLOSE_FLAGS[notify_fd].closing = 1;
    close_socket(notify_fd);  //如果有线程在对fd进行IO，则调用static函数close_socket()，向目标线程发信号，让目标线程自己close socket fd
}
pthread_mutex_unlock(&CLOSE_MUTEX[notify_fd]);

```
<br>
接下来是线程池worker_function<br>

``` C
// source/main


/**
* wrapper函数为void xxx(void*)的形式，可供线程池直接调用
* 其中封装了业务函数不必关心的加锁解锁等其他操作
*/
void notify_clients_wrapper(void* data){
    int socket_fd = (int)data;
    SIG_CAUGHT_FLAG[socket_fd] = 0; 
    
    pthread_mutex_lock(&FD_MUTEX_ARRAY[socket_fd]); //这个锁是为了使同一时刻只有一个线程对一个fd进行IO

    pthread_mutex_lock(&CLOSE_MUTEX[socket_fd]); //这个锁是为了保护CLOSE_FLAG，此处可能与close_client_connection中判断是否有线程在读写socket fd存在竞争。
    if(CLOSE_FLAGS[socket_fd].closing == 1){   //如果在执行notify_clients之前，已经要求关闭连接，则直接返回
        pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);
        pthread_mutex_unlock(&FD_MUTEX_ARRAY[socket_fd]);
        pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
        return;
    }else{
        CLOSE_FLAGS[socket_fd].tid = pthread_self();  //如果在执行notify_clients之前没有要求关闭连接，则将tid设置为当前thread_id，接着正常执行notify_clients即可
    }
    pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);

    notify_clients();  //被wrapp的函数,内部有对socket fd的IO操作
    
    SIG_CAUGHT_FLAG[socket_fd] = 1;  //为了确保不丢失信号，导致，ACCEPT_MUTEX死锁，详解看下文

    pthread_mutex_unlock(&ACCEPT_MUTEX); //解锁在close_on_SIGUSR1中加锁的，用于保证fd在关闭完成之前不会被新连接占用的互斥锁。
                                         //注意： 多次重复unlock不会导致严重问题，只会返回errno而已
    pthread_mutex_unlock(&FD_MUTEX_ARRAY[socket_fd]);

    pthread_mutex_lock(&CLOSE_MUTEX[socket_fd]);
    if(CLOSE_FLAGS[socket_fd].closing == 1){ //如果正在进行关闭过程，且本该在close_on_SIGUSR1信号处理函数中设置的SIG_HANDLED_FLAG未被设置。
        while(SIG_HANDLED_FLAG[socket_fd] != 1){  //为了防止信号在传递到当前线程之前函数已经返回，即如果信号处理函数没触发则等待5秒，信号如果在此时到来会自动打断sleep。
            sleep(5);
        }
        CLOSE_FLAGS[socket_fd].tid = -1;
    }else{
        CLOSE_FLAGS[socket_fd].tid = -1;
    }
    pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);
}




void close_on_SIGUSR1(int sig){
    int i;
    for(i = 0; i < EVENT_MAX; i++){
        if(CLOSE_FLAGS[i].tid == pthread_self()){
            if(SIG_CAUGHT_FLAG[i] != 1) //如果信号在SIG_CAUGHT_FLAG被设置前触发，则加上锁，在之后触发就不加锁。
                pthread_mutex_lock(&ACCEPT_MUTEX);
            close(i);
            SIG_HANDLED_FLAG[i] = 1;
        }
    }
}

```
![](https://github.com/xiaoheng86/avo-chat-server/blob/main/readme/sigcaughtafterunlock.jpg)<br>
如图所示，如果unlock ACCEPT_MUTEX之后信号才到来，会导致ACCEPT_MUTEX死锁。<br>
&emsp;我的解决方案是，设置一个SIG_CAUGHT_FLAG，如果信号在SIG_CAUGHT_FLAG置为1之前信号到来，则在sighandler中对ACCEPT_MUTEX加锁，在之后信号到来则不加锁，因为没加锁，所以不管是否unlock都不会造成死锁这样的严重后果。
