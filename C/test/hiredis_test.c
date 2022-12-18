//
// Created by xiaoheng on 22-12-8.
//
#include <unistd.h>
#include "hiredis/hiredis.h"
#include "global.h"
#include "utils/log.h"
#include "base/initialize.h"

int main(){
    REDIS_CONTEXT = connect_to_redis();
    char* token =  "Bearer_eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1c2VyIjp7InVzZXJJRCI6IjEyOTg3Nzc0In19.PTc3KubSTAKv-XO9rT955yZfkSU7e8bJWZ772T7Umi8";
    if(REDIS_CONTEXT == NULL){
        log_fatal("connect to redis failed");
        exit(-1);
    }
    redisReply * reply = redisCommand(REDIS_CONTEXT,"HSET %s %d %d", token,2, 14);
    if(reply == NULL){
        log_fatal("redis command failed");
        exit(-1);
    }
    if(reply->type == REDIS_REPLY_INTEGER){
        log_info("redis command return: %d", reply->integer);
    }else{
        log_info("redis command return is not integer");
    }
    freeReplyObject(reply);

    reply = redisCommand(REDIS_CONTEXT,"HGET %s %d",token, 2);
    if (reply == NULL){
        log_fatal("redis command failed");
        exit(-1);
    }

    log_debug("reply type %d", reply->type);

    if(reply->type == REDIS_REPLY_NIL){
        log_info("redis command return nil");
    }
    if(reply->type == REDIS_REPLY_ERROR){
        log_info("redis command return error");
    }
    if(reply->type == REDIS_REPLY_STATUS){
        log_info("redis command return status");
    }
    if(reply->type == REDIS_REPLY_INTEGER){
        log_info("redis command return: %d", reply->integer);
    }
    if(reply->type == REDIS_REPLY_STRING){
        log_info("redis command return string %s", reply->str);
    }

    freeReplyObject(reply);
    return 0;
}