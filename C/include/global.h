#ifndef SERVER_PROTOCOL_GLOBAL_H
#define SERVER_PROTOCOL_GLOBAL_H
#include<pthread.h>
#include <setjmp.h>
#include "hiredis/hiredis.h"
#define EVENT_MAX 1024

#define SIG_MSG_DIGIT_NUM 2
#define SIG_MSG_BUFSIZE 32
#define PIPE_MSG_DIGIT_NUM 4
#define PIPE_MSG_BUFSIZE 1024
#define SOCKET_MSG_DIGIT_NUM 6
#define SOCKET_BUFSIZE 65536



struct close_flag{
    int flag;
    pthread_t tid;
    char token[2048];
    int type;
    int closing;
};
extern char* LOG_PATH;
extern char* CONFIG_PATH;
extern char* LOCK_PATH;
extern pthread_mutex_t LOG_MUTEX;
extern pthread_mutex_t CLOSE_MUTEX[EVENT_MAX];
extern char* SPRING_BOOT_JAR_PATH;
extern int PIPE_FDS[2];
extern int SIG_PIPE_FDS[2];
extern int TIME_SLOT;
extern int LISTENING_PORT;
extern int IS_DAEMON;
extern redisContext *REDIS_CONTEXT;
extern struct thpool_* THREADING_POLL_P;
extern int THREAD_POOL_SIZE;
extern struct event_loop * EVENT_LOOP;
extern pthread_mutex_t REDIS_MUTEX;
extern pthread_mutex_t FD_MUTEX_ARRAY[EVENT_MAX];
extern struct close_flag CLOSE_FLAGS[EVENT_MAX];
extern struct sort_timer_list* TIMER_LIST;
extern pthread_mutex_t CJSON_MUTEX;
extern int TOKEN_EXPIRE_TIME;
extern struct timer* HEARTBEAT_TIMER;
extern pthread_mutex_t ACCEPT_MUTEX;
#endif
