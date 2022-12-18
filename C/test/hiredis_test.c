//
// Created by xiaoheng on 22-12-8.
//
#include "hiredis/hiredis.h"
#include "global.h"
#include "utils/log.h"
#include "base/initialize.h"

int main(){
    REDIS_CONTEXT = connect_to_redis();
    if(REDIS_CONTEXT == NULL){
        log_fatal("connect to redis failed");
        exit(-1);
    }
    redisReply * reply = redisCommand(REDIS_CONTEXT,"HSET \"%s\" %d %d", "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJ1c2VyIjp7InVzZXJJRCI6IjEyOTg3Nzc0In19.PTc3KubSTAKv-XO9rT955yZfkSU7e8bJWZ772T7Umi8", 2, 14);
    if(reply == NULL){
        log_fatal("redis command failed");
        exit(-1);
    }
    if(reply->type == REDIS_REPLY_NIL){
        log_info("redis command: HSET returns nil");
    }else{
        log_info("redis command: HSET returns not nil");
    }
    freeReplyObject(reply);
    return 0;
}