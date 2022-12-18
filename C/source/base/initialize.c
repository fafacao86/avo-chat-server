#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include "utils/log.h"
#include "initialize.h"
#include "global.h"
#include "base/signal_handle.h"
#include "hiredis/hiredis.h"


void daemonize()
{
    // 使当前进程成为守护进程
    pid_t pid = fork();
    if (pid < 0) {
        log_fatal_with_errno("fork error");
        exit(-1);
    }
    if (pid > 0) {
        exit(0);
    }
    if (setsid() < 0) {
        log_fatal_with_errno("setsid error");
        exit(-1);
    }
    pid = fork();
    if (pid < 0) {
        log_fatal_with_errno("fork");
        exit(-1);
    }
    if (pid > 0) {
        exit(0);
    }
    umask(0);
    chdir("/");
    // 关闭标准输入输出
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    // 将标准输入输出重定向到 /dev/null和 LOG文件
    open("/dev/null", O_RDONLY);
    open(LOG_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
    open(LOG_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
}

static void log_Lock_func(bool lock_or_not, void* udata){
    if(lock_or_not){
        pthread_mutex_lock(&LOG_MUTEX);
    }else{
        pthread_mutex_unlock(&LOG_MUTEX);
    }
}

void set_lock_for_logger(){
    log_set_lock(log_Lock_func, NULL);
}

void assert_single_instance(){
    // 确保只有一个实例在运行
    int fd = open(LOCK_PATH, O_WRONLY | O_CREAT | O_EXCL, 0644);
    log_info_with_errno("open lock file");
    if (fd < 0) {
        log_fatal("Another instance is running, exit.");
        exit(0);
    }
    char buf[16];
    sprintf(buf, "%d", getpid());
    write(fd, buf, strlen(buf));
}


void init_springboot_server(){
    // 初始化springboot服务器

    /** Generates two pipe file descriptors **/
    pipe(PIPE_FDS);

    /** run spring boot server **/
    if (!fork()) {
        /**close parent process's log file, open new log file for springboot**/
        char pid_str[64];
        char pipe_fd[16];
        close(0);
        close(1);
        close(2);
        open("/dev/null", O_RDONLY);
        open(JAVA_LOG_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
        open(JAVA_LOG_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
        pid_t pid = getpid();
        CHILD_PID = pid;
        sprintf(pid_str, "%d", pid);
        sprintf(pipe_fd, "%d", PIPE_FDS[1]);
        close(PIPE_FDS[0]);
        execlp("java", "java", "-jar", SPRING_BOOT_JAR_PATH, pid_str, pipe_fd, NULL);

        perror("execlp");
    }else{
        return;
    }
}

//must be called after `init_springboot_server()`, because the subprocess will inherit signal mask

void init_signal_handle()
{

    pipe(SIG_PIPE_FDS);
    sigset[0] = SIGINT;
    sigset[1] = SIGPIPE;
    sigset[2] = SIGALRM;
    sigset[3] = SIGCHLD;
    sigset[4] = SIGTERM;

    sighandler_set[0] = signal_handle_sigint;
    sighandler_set[1] = signal_handle_sigpipe;
    sighandler_set[2] = signal_handle_sigalrm;
    sighandler_set[3] = signal_handle_sigchld;
    sighandler_set[4] = signal_handle_sigterm;

    install_signal_handler(5);
}


/**connect to redis server
 * return a pointer to redisContext
 * redisContent needs to be freed manually after use by caller
 * **/
redisContext* connect_to_redis(){
    redisContext *c;
    redisReply *reply;
    char *hostname = "127.0.0.1";
    int port = 6389;
    struct timeval timeout = { 2, 500000 }; // 2.5 seconds

    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            log_error("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            log_error("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    log_info("Successfully connected to redis server");
    return c;
}



/**
 * take command line arguments to override config file settings
 * -p change the listening port
 * -d run server as daemon
 * **/
void parse_command_line_args(int argc, char** argv){
    int opt;
    if (argc == 1) {
        log_info("No command line arguments specified, using default config.");
        return;
    }
    while((opt = getopt(argc, argv, ":if:lrx")) != -1)
    {
        switch(opt)
        {
            case 'd':
                IS_DAEMON = 1;
                break;
            case 'p':
                LISTENING_PORT = atoi(optarg);
                break;
            case ':':
                log_error("option needs a value\n");
                break;
            case '?':
                log_error("unknown option: %c\n", optopt);
                break;
            default:
                break;
        }
    }
}

void init_recursive_mutex(pthread_mutex_t* mutex_p){
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex_p, &attr);
}


void init_threading_pool(int thread_num){
    THREADING_POLL_P = thpool_init(thread_num);
    log_info("initialized threading pool, worker threads num: %d", thread_num);
}

void load_configuration(){
    if (access(CONFIG_PATH, F_OK) == 0) {

    } else {
        log_warn("Config file not found, using default config.");
    }
}

void init_close_flags(){
    for(int i=0;i<EVENT_MAX;i++){
        CLOSE_FLAGS[i].flag = 0;
        CLOSE_FLAGS[i].tid = 0;
        CLOSE_FLAGS[i].type = 0;
        CLOSE_FLAGS[i].closing = 0;
        CLOSE_FLAGS[i].token[0] = '\0';
        pthread_mutex_init(&CLOSE_MUTEX[i],NULL);
        pthread_mutex_init(&FD_MUTEX_ARRAY[i],NULL);
    }
}

void init_locks(){
    for (int i = 0; i <EVENT_MAX; ++i) {
        pthread_mutex_init(&FD_MUTEX_ARRAY[i], NULL);
        init_recursive_mutex(&CLOSE_MUTEX[i]);
    }
}
