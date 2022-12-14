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
    redisReply * reply = redisCommand(REDIS_CONTEXT,"HEXISTS %s", "13.13.13.12");
    if(reply == NULL){
        log_fatal("redis command failed");
        exit(-1);
    }
    if(reply->type == REDIS_REPLY_NIL){
        log_info("redis command: HEXISTS returns nil");
    }else{
        log_info("redis command: HEXISTS returns not nil");
    }
    freeReplyObject(reply);
    return 0;
}