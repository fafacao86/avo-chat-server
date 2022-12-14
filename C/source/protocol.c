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
    redisReply *reply = redisCommand(REDIS_CONTEXT, "HEXISTS %s", CLOSE_FLAGS[fd].token);
    if (reply->type == REDIS_REPLY_NIL){
        log_error("redis command: HEXISTS %s token returns nil! The target client is not online", CLOSE_FLAGS[fd].token);
        pthread_mutex_unlock(&REDIS_MUTEX);
        freeReplyObject(reply);
        return -1;
    }
    if (reply->type == REDIS_REPLY_STRING){
        if (strcmp(reply->str, token) == 0){
            pthread_mutex_unlock(&REDIS_MUTEX);
            freeReplyObject(reply);
            return 0;
        }
    }
    redisCommand(REDIS_CONTEXT, "EXPIRE %s %d", CLOSE_FLAGS[fd].token, TOKEN_EXPIRE_TIME);
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
    log_trace("heartbeat_callback");
    int fd = *(int*)data;
    redisReply *reply = redisCommand(REDIS_CONTEXT, "SMEMBERS %s", "online_users_fd");
    if (reply == NULL) {
        log_fatal("redis command failed");
        exit(-1);
    }
    char json_buffer[SOCKET_BUFSIZE];
    char token[SOCKET_BUFSIZE/2];
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < reply->elements; ++i) {
            int heartbeat_fd = (int) atoi(reply->element[i]->str);
            struct timer* socket_timeout_timer = create_timer(((TIME_SLOT/6) * 5)+ time(NULL), timeout_callback_adapter, &heartbeat_fd);
            add_timer(TIMER_LIST, socket_timeout_timer);
            if(send_json_string_to_socket("{\"type\": \"Heartbeat\"}", heartbeat_fd) == -1){
                log_error("send heartbeat to fd: %d failed", heartbeat_fd);
                return;
            }
            if(get_json_string_from_socket(json_buffer, heartbeat_fd) == -1){
                log_error("get heartbeat from fd: %d failed", heartbeat_fd);
                return;
            }
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
 *          "from": "ID",
 *          "to": "ID",
 *          "sender_token": "xxx.xxx.xxx"
 *      }
 *
 *   *JSON send through socket
 *      {
 *          "type": int,
 *          "from": int,
 *          "to": int,
 *          "sender": int
 *      }
 * **/
void notify_clients(void* data){


    char json_buffer[PIPE_MSG_BUFSIZE];
    int result = get_json_string_from_pipe(json_buffer);
    if (result == -1){
        log_error("connection closed");
        return;
    }

    struct notify_item* notify = (struct notify_item*)malloc(sizeof(struct notify_item));
    parse_notify_msg(json_buffer, notify);

    pthread_mutex_lock(&REDIS_MUTEX);
    char sender_id[16];

    redisReply * reply;
    reply = redisCommand(REDIS_CONTEXT, "HGET %s token", notify->sender_token);
    if(reply->type == REDIS_REPLY_STRING)
    {
        strncpy(sender_id, reply->str, strlen(reply->str));
    }


    if (notify->type == T_P2P) {
        create_notify_json(T_P2P, notify->from, notify->to, "0",json_buffer, SOCKET_BUFSIZE);
        reply = redisCommand(REDIS_CONTEXT, "HGET %s token", notify->to);
    if (reply->type == REDIS_REPLY_NIL){
        log_error("redis command: HGET %s ip returns nil! The target client is not online", notify->to);
        pthread_mutex_unlock(&REDIS_MUTEX);
        freeReplyObject(reply);
        return;
    }
    if (reply->type == REDIS_REPLY_STRING) {
        char target_token[2048];
        strcpy(target_token, reply->str);
        freeReplyObject(reply);
        reply = redisCommand(REDIS_CONTEXT, "HGET %s %d", target_token, T_NOTIFY);
        if (reply->type == REDIS_REPLY_NIL) {
            log_error("redis command: HGET %s %d returns nil! The target client is not online", target_token, T_NOTIFY);
            pthread_mutex_unlock(&REDIS_MUTEX);
            freeReplyObject(reply);
            return;
        }
        if (reply->type == REDIS_REPLY_INTEGER) {
            *((int*)data) = reply->integer;
            int target_fd = (int) reply->integer;
            send_json_string_to_socket(json_buffer, target_fd);
            log_info("notify client %s to pull new messages", target_token);
        }
    }


    }else if (notify->type == T_P2G){
        redisReply* reply_array = redisCommand(REDIS_CONTEXT, "SMEMBERS %s", notify->from);
        if (reply_array->type == REDIS_REPLY_ARRAY){
            for (int i = 0; i < reply_array->elements; ++i){
                char target_token[2048];
                char to[16];
                strncpy(to, reply_array->element[i]->str, 16);
                reply = redisCommand(REDIS_CONTEXT, "HGET %s token", to);
                if(reply->type == REDIS_REPLY_STRING){
                    strcpy(target_token, reply->str);
                    freeReplyObject(reply);
                    reply = redisCommand(REDIS_CONTEXT, "HGET %s %d", target_token, T_NOTIFY);
                    if (reply->type == REDIS_REPLY_INTEGER){
                        int target_fd = (int) reply->integer;
                        create_notify_json(T_P2G, notify->from, to, sender_id,json_buffer, SOCKET_BUFSIZE);
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
