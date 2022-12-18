//
// Created by xiaoheng on 22-12-7.
//

#include "protocol.h"
#include <stdlib.h>
#include <unistd.h>
#include "utils/log.h"
#include "utils/io_utils.h"
#include "utils/timer.h"
#include "hiredis/hiredis.h"
#include "event_loop.h"
#include "utils/str_utils.h"
#include "base/socket_base.h"




static void timeout_callback_adapter(void * fd){
    int heartbeat_fd = *(int*)fd;
    close_client_connection(heartbeat_fd);
}

static inline int check_token(const char* token, int fd){
    pthread_mutex_lock(&REDIS_MUTEX);
    redisReply *reply = redisCommand(REDIS_CONTEXT, "HEXISTS '%s'", CLOSE_FLAGS[fd].token);
    if (reply->type == REDIS_REPLY_NIL){
        log_error("redis command: HEXISTS %s token returns nil! The target client is not online", CLOSE_FLAGS[fd].token);
        pthread_mutex_unlock(&REDIS_MUTEX);
        freeReplyObject(reply);
        return -1;
    }else{
        pthread_mutex_unlock(&REDIS_MUTEX);
        freeReplyObject(reply);
        return 0;

    }
    redisCommand(REDIS_CONTEXT, "EXPIRE '%s' %d", CLOSE_FLAGS[fd].token, TOKEN_EXPIRE_TIME);
    pthread_mutex_unlock(&REDIS_MUTEX);
    return 1;
}

/**Called every TIME_SLOT seconds to check if there is a client timeout
 *
 *  * HeartBeat Message Format:
 *  Server->Client
 *      <SYN>
 *
 *  Client->Server
 *      {
 *          "Token": "xxx.xxx.xxx"
 *      }
**/
void heartbeat_callback(void* data) {
    redisReply *reply = redisCommand(REDIS_CONTEXT, "SMEMBERS %s", "online_users_fd");
    if (reply == NULL) {
        log_warn("redis command failed");
    }
    char json_buffer[SOCKET_BUFSIZE];
    char token[SOCKET_BUFSIZE/2];
    log_info("online heartbeat fd count: %d", reply->elements);

    if (reply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < reply->elements; ++i) {
            int heartbeat_fd = (int) atoi(reply->element[i]->str);
            create_heartbeat_request_json(json_buffer);
            struct timer* socket_timeout_timer = create_timer(((TIME_SLOT/6) * 5)+ time(NULL), timeout_callback_adapter, &heartbeat_fd);
            add_timer(TIMER_LIST, socket_timeout_timer);
            log_trace("sending heartbeat to fd: %d", heartbeat_fd);
            if(send_json_string_to_socket(json_buffer, heartbeat_fd) == -1){
                log_error("send heartbeat to fd: %d failed", heartbeat_fd);
                return;
            }
            if(get_json_string_from_socket(json_buffer, heartbeat_fd) == -1){
                log_error("get heartbeat from fd: %d failed, %s", heartbeat_fd, json_buffer);
                return;
            }
            log_trace("got response from fd: %d %s", heartbeat_fd, json_buffer);
            del_timer(TIMER_LIST, socket_timeout_timer);
            parse_heartbeat_responce(json_buffer, token, SOCKET_BUFSIZE/2);
            if (check_token(token, heartbeat_fd) == 0){
                log_info("client %d heartbeat success", heartbeat_fd);
                HEARTBEAT_TIMER->expire = TIME_SLOT + time(NULL);
                adjust_timer(TIMER_LIST, HEARTBEAT_TIMER);
            } else {
                log_info("client %d heartbeat failed", heartbeat_fd);
                close_client_connection(heartbeat_fd);
            }
        }
    }
}



/**
 * Springboot server will send a JSON through pipe when any client send message to server,
 * and this protocol will notify the targeted clients to pull new messages actively from springboot server.
 *
 *   *JSON through pipe format:
 *      {
 *          "type": "P2P or P2G",   //(1->P2P    2->P2G)
 *          "puller": "ID",
 *          "pull_target": "ID",
 *      }
 *
 *   *JSON send through socket
 *      {
 *          "type": int,
 *          "puller": "ID",
 *          "pull_target": "ID"
 *      }
 * **/
void notify_clients(){
    log_trace("notify begins");

    char json_buffer[SOCKET_BUFSIZE];
    int result = get_json_string_from_pipe(json_buffer);
    if (result == -1){
        log_error("connection closed");
        return;
    }
    log_trace("get json from pipe: %s", json_buffer);
    struct notify_item* notify = (struct notify_item*)malloc(sizeof(struct notify_item));
    parse_notify_msg(json_buffer, notify);

    pthread_mutex_lock(&REDIS_MUTEX);

    redisReply * reply;
    log_trace("notify type: %d", notify->type);
    if (notify->type == T_P2P) {
        create_notify_json(notify->puller, notify->pull_target, json_buffer, SOCKET_BUFSIZE);
        reply = redisCommand(REDIS_CONTEXT, "HGET %s token", notify->puller);
    if (reply->type == REDIS_REPLY_NIL){
        log_warn("redis command: HGET %s ip returns nil! The target client is not online", notify->puller);
        pthread_mutex_unlock(&REDIS_MUTEX);
        freeReplyObject(reply);
        return;
    }
    if (reply->type == REDIS_REPLY_STRING) {
        log_trace("token: %s", reply->str);
        char target_token[2048];
        strcpy(target_token, reply->str);
        freeReplyObject(reply);
        reply = redisCommand(REDIS_CONTEXT, "HGET '%s' %d", target_token, T_NOTIFY);
        log_trace("NOTIFY fd: %d", reply->integer);
        if (reply->type == REDIS_REPLY_NIL) {
            log_error("redis command: HGET %s %d returns nil! The target client is not online", target_token, T_NOTIFY);
            pthread_mutex_unlock(&REDIS_MUTEX);
            freeReplyObject(reply);
            return;
        }
        if (reply->type == REDIS_REPLY_INTEGER) {
            int target_fd = (int) reply->integer;
            result = send_json_string_to_socket(json_buffer, target_fd);
            if (result == -1) {
                log_error("send notify to fd: %d failed", target_fd);
                pthread_mutex_unlock(&REDIS_MUTEX);
                freeReplyObject(reply);
                return;
            }
            log_info("notify client %s to pull new messages", target_token);
        }
    }


    }else if (notify->type == T_P2G){
        redisReply* reply_array = redisCommand(REDIS_CONTEXT, "SMEMBERS %s", notify->pull_target);
        if (reply_array->type == REDIS_REPLY_ARRAY){
            for (int i = 0; i < reply_array->elements; ++i){
                char target_token[2048];
                char puller[16];
                strncpy(puller, reply_array->element[i]->str, 16);
                reply = redisCommand(REDIS_CONTEXT, "HGET %s token", puller);
                if(reply->type == REDIS_REPLY_STRING){
                    strcpy(target_token, reply->str);
                    freeReplyObject(reply);
                    reply = redisCommand(REDIS_CONTEXT, "HGET '%s' %d", target_token, T_NOTIFY);
                    if (reply->type == REDIS_REPLY_INTEGER){
                        int target_fd = (int) reply->integer;
                        create_notify_json(puller, notify->pull_target,json_buffer, SOCKET_BUFSIZE);
                        result = send_json_string_to_socket(json_buffer, target_fd);
                        if (result == -1){
                            log_error("connection closed");
                            pthread_mutex_unlock(&REDIS_MUTEX);
                            freeReplyObject(reply);
                            return;
                        }
                    }
                    freeReplyObject(reply);
                    log_info("notify client %s to pull new messages", target_token);
                }
            }
        }
        freeReplyObject(reply_array);
    }
    free(notify);
    pthread_mutex_unlock(&REDIS_MUTEX);
}



/**
 * read a json message from pipe
 * springboot server will help us send the heartbeat fd in the json
 *
 * json format:
 *    {
 *          "heartbeat_fd": int
 *    }
 * **/
void close_connection_actively(void*data){
    int heartbeat_fd;
    char json_buffer[PIPE_MSG_BUFSIZE];
    int result = get_json_string_from_pipe(json_buffer);
    if (result == -1){
        log_error("connection closed");
        return;
    }
    parse_close_connection_json(json_buffer, &heartbeat_fd);
    close_client_connection(heartbeat_fd);
}
