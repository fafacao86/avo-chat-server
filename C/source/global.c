#include <pthread.h>
#include <signal.h>
#include <setjmp.h>
#include "hiredis/hiredis.h"
#include "base/thpool.h"
#define EVENT_MAX 1024
struct close_flag{
    int flag;
    pthread_t tid;
    char token[2048];
    int type;
    int closing;
};

char* CONFIG_PATH = "/etc/avo/avo.conf";
char* LOG_PATH = "/var/log/avo/avo.log";
char* JAVA_LOG_PATH = "/var/log/avo/springboot_server.log";
char* LOCK_PATH = "/var/run/avo.pid";
char* SPRING_BOOT_JAR_PATH = "";
pthread_mutex_t LOG_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t REDIS_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CJSON_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ACCEPT_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CLOSE_MUTEX[EVENT_MAX];
int LISTENING_PORT = 5000;
int PIPE_FDS[2];
int TIME_SLOT = 18;      //seconds
int SIG_PIPE_FDS[2];
int IS_DAEMON=0;
redisContext *REDIS_CONTEXT = NULL;
struct thpool_* THREADING_POLL_P = NULL;
int THREAD_POOL_SIZE = 6;
struct event_loop * EVENT_LOOP = NULL;
pthread_mutex_t FD_MUTEX_ARRAY[EVENT_MAX];
struct close_flag CLOSE_FLAGS[EVENT_MAX];
struct sort_timer_list* TIMER_LIST = NULL;
int TOKEN_EXPIRE_TIME = 259200;
struct timer* HEARTBEAT_TIMER;
