#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "global.h"
#include "utils/log.h"
#include "utils/io_utils.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "base/event_loop.h"
#include "protocol.h"
#include "base/initialize.h"
#include "utils/str_utils.h"



void set_sockopt(int listen_fd){
    int opt = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        log_error_with_errno("setsockopt SO_RESUADDR");
        exit(-1);
    }
    if(setsockopt(listen_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1){
        log_error_with_errno("setsockopt TCP_NODELAY");
        exit(-1);
    }
}


int bind_localhost_port(){
    int listenfd;
    struct sockaddr_in serv_addr;

    char send_buff[SOCKET_BUFSIZE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(send_buff, 0, sizeof(send_buff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(LISTENING_PORT);
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
        log_fatal_with_errno("bind error");
        exit(-1);
    }
    log_info("Listening at \033[32m127.0.0.1:%d...", LISTENING_PORT);
    printf("\033[0m");
    return listenfd;

}




/**
 * format:
 * {
 *      "type": int,
 *      "token": "xxx.xxx.xxx"
 * }
 * **/
static  int get_fd_type_and_token(int connfd, int *type, char * token_buffer){
    int opt = 1;
    char json_msg[SOCKET_BUFSIZE];
    int result = get_json_string_from_socket(json_msg, connfd);
    if(result == -1){
        log_error("connection closed");
        return -1;
    }
    parse_connection_json(json_msg, type, token_buffer, 2048);


    if(*type == T_HEARTBEAT) {
        /**heartbeat flag disable NAGLE algorithm**/
        if(setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1){
            log_error_with_errno("setsockopt TCP_NODELAY");
            exit(-1);
        }
        return T_HEARTBEAT;
    }

    if (*type == T_NOTIFY) {
        /**notify flag disable NAGLE algorithm**/
        if(setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == -1){
            log_error_with_errno("setsockopt TCP_NODELAY");
            exit(-1);
        }
        return T_NOTIFY;
    }
    if (*type == T_FILE) {
        return T_FILE;
    }
    return 0;
}



/**
 * @param listen_fd the flag to listen
 * @return connected flag
 * @description:
 * 1. get peer ip and port
 * 2. check the redis, if the ip is logged in
 * 3. if yes, put the flag type and port in that redis hash for worker thread to use
 * **/
int connect_to_client(int listen_fd, int* type){
    int connfd;
    char token[2048];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    pthread_mutex_lock(&ACCEPT_MUTEX);
    connfd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    pthread_mutex_unlock(&ACCEPT_MUTEX);
    if(connfd == -1){
        log_error_with_errno("accept error");
        exit(-1);
    }

    int result = get_fd_type_and_token(connfd, type,token);
    if(result == -1){
        close(connfd);
        return -1;
    }
    log_trace("token: %s fd: %d, type: %d", token, connfd, *type);
    redisReply* reply;
    pthread_mutex_lock(&REDIS_MUTEX);
    reply = redisCommand(REDIS_CONTEXT, "HEXISTS %s", token);
    if (reply->type == REDIS_REPLY_NIL){
        close(connfd);
        return 0;
    }

    reply = redisCommand(REDIS_CONTEXT, "HSET %s %d %d", token, *type, connfd);
    log_trace("HSET '%s' %d %d", token, *type, connfd);
    if (reply == NULL) {
        log_fatal("redis command failed");
        //exit(-1);
    }
    freeReplyObject(reply);


    if(*type == T_HEARTBEAT){
        reply = redisCommand(REDIS_CONTEXT, "SADD online_users_fd %d", connfd);
        if (reply == NULL) {
            log_fatal("redis command failed");
            //exit(-1);
        }
        freeReplyObject(reply);
    }


    reply = redisCommand(REDIS_CONTEXT, "EXPIRE %s %d", token, TOKEN_EXPIRE_TIME);
    freeReplyObject(reply);

    pthread_mutex_unlock(&REDIS_MUTEX);

    pthread_mutex_lock(&CLOSE_MUTEX[connfd]);
    CLOSE_FLAGS[connfd].type = *type;
    CLOSE_FLAGS[connfd].tid = -1;
    strncpy(CLOSE_FLAGS[connfd].token, token, sizeof(CLOSE_FLAGS[connfd].token));
    pthread_mutex_unlock(&CLOSE_MUTEX[connfd]);
    log_info("new connection from \033[32m%s:%d\033[0m", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    return connfd;
}





/**
 *
 * **/
static void close_socket(int fd){
    if(fd == -1)
        return;
    CLOSE_FLAGS[fd].closing = 1;
    CLOSE_FLAGS[fd].flag = 1;
    pthread_mutex_lock(&REDIS_MUTEX);
    redisReply * reply = redisCommand(REDIS_CONTEXT, "HDEL %s %d", CLOSE_FLAGS[fd].token, CLOSE_FLAGS[fd].type);
    pthread_mutex_unlock(&REDIS_MUTEX);
    freeReplyObject(reply);
    pthread_kill(CLOSE_FLAGS[fd].tid, SIGUSR1);

    log_info("client: %d connection closing",CLOSE_FLAGS[fd].type);
}



void close_client_connection(int heartbeat_fd){
    pthread_mutex_lock(&CLOSE_MUTEX[heartbeat_fd]);
    if (CLOSE_FLAGS[heartbeat_fd].closing == 1){
        pthread_mutex_unlock(&CLOSE_MUTEX[heartbeat_fd]);
        return;
    }
    CLOSE_FLAGS[heartbeat_fd].closing = 1;
    pthread_mutex_unlock(&CLOSE_MUTEX[heartbeat_fd]);
    int notify_fd, file_fd;
    const char* token =  CLOSE_FLAGS[heartbeat_fd].token;
    redisReply * reply;
    pthread_mutex_lock(&REDIS_MUTEX);
    reply = redisCommand(REDIS_CONTEXT,"HGET %s %d", token, T_NOTIFY);
    if(reply->type == REDIS_REPLY_NIL)
        log_error("redis command: HGET %s %d returns nil! The target client is not online", token, T_NOTIFY);
    if(reply->type == REDIS_REPLY_INTEGER)
        notify_fd = (int)reply->integer;
    else
        notify_fd = -1;
    freeReplyObject(reply);

    reply = redisCommand(REDIS_CONTEXT,"HGET %s %d", token, T_FILE);
    if(reply->type == REDIS_REPLY_NIL)
        log_error("redis command: HGET %s %d returns nil! The target client is not online", token, T_NOTIFY);
    if(reply->type == REDIS_REPLY_INTEGER)
        file_fd = (int)reply->integer;
    else
        file_fd = -1;
    freeReplyObject(reply);
    if(notify_fd != -1 && file_fd != -1){
        pthread_mutex_lock(&CLOSE_MUTEX[notify_fd]);
        if (CLOSE_FLAGS[notify_fd].tid == -1){
            CLOSE_FLAGS[notify_fd].closing = 1;
            close(notify_fd);
            event_loop_del(EVENT_LOOP, notify_fd);
        }else{
            CLOSE_FLAGS[notify_fd].closing = 1;
            close_socket(notify_fd);
        }
        pthread_mutex_unlock(&CLOSE_MUTEX[notify_fd]);


        pthread_mutex_lock(&CLOSE_MUTEX[file_fd]);
        if (CLOSE_FLAGS[file_fd].tid == -1){
            CLOSE_FLAGS[file_fd].closing = 1;
            close(file_fd);
            event_loop_del(EVENT_LOOP, file_fd);
        }else{
            CLOSE_FLAGS[file_fd].closing = 1;
            close_socket(file_fd);
        }
        pthread_mutex_unlock(&CLOSE_MUTEX[file_fd]);
    }

    if (file_fd != -1 && notify_fd != -1){
        reply = redisCommand(REDIS_CONTEXT, "HDEL %s %d", CLOSE_FLAGS[file_fd].token, CLOSE_FLAGS[file_fd].type);
        freeReplyObject(reply);
        reply = redisCommand(REDIS_CONTEXT, "HDEL %s %d", CLOSE_FLAGS[notify_fd].token, CLOSE_FLAGS[notify_fd].type);
        freeReplyObject(reply);
    }

    reply = redisCommand(REDIS_CONTEXT, "HDEL %s %d", token, heartbeat_fd);
    freeReplyObject(reply);
    reply = redisCommand(REDIS_CONTEXT, "SREM online_users_fd %d", heartbeat_fd);
    freeReplyObject(reply);
    reply = redisCommand(REDIS_CONTEXT, "DEL %s", token);
    freeReplyObject(reply);
    pthread_mutex_unlock(&REDIS_MUTEX);
}



void close_on_SIGUSR1(int sig){
    int i;
    for(i = 0; i < EVENT_MAX; i++){
        if(CLOSE_FLAGS[i].tid == pthread_self()){
            if(SIG_CAUGHT_FLAG[i] != 1)
                pthread_mutex_lock(&ACCEPT_MUTEX);
            close(i);
            SIG_HANDLED_FLAG[i] = 1;
        }
    }
}
