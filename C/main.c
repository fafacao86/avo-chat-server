#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "base/initialize.h"
#include "utils/log.h"
#include "global.h"
#include "str_utils.h"
#include "base/signal_handle.h"
#include "hiredis/hiredis.h"
#include "base/event_loop.h"
#include "base/socket_base.h"
#include "utils/io_utils.h"
#include "utils/str_utils.h"
#include "utils/timer.h"
#include "protocol.h"

void clean_up();
void notify_clients_wrapper(void* data);
void close_actively_wrapper(void * data);
void heartbeat_callback_wrapper(void * data);


int main(int argc, char *argv[])
{
    //register clean up callback when exit
    atexit(clean_up);

    //print the ascii art logo
    print_logo();


    //make sure there is only one instance of this server
    assert_single_instance();

    //init logger, set a mutex for logger for thread safety, if you log in signal handler, don't set a mutex.
    //set_lock_for_logger();

    load_configuration();

    //parse command line args to override some default config
    parse_command_line_args(argc, argv);

    // daemonize the process
    if (IS_DAEMON)
        daemonize();

    //fork java springboot server
    init_springboot_server();


    init_close_flags();

    init_locks();

    // connect to redis
    REDIS_CONTEXT = connect_to_redis();

    //init signal handle
    init_signal_handle();

    //init separate signal handling thread
    init_sighandling_thread();

    //init threading pool
    init_threading_pool(THREAD_POOL_SIZE);

    start_signal_handle_thread();

    //init global epoll event loop
    EVENT_LOOP = (struct event_loop *)malloc(sizeof(struct event_loop));
    init_event_loop(EVENT_LOOP);


    //init the global timer list, setup heartbeat timer
    init_sort_timer_list(&TIMER_LIST);
    HEARTBEAT_TIMER = create_timer(time(NULL) + TIME_SLOT, heartbeat_callback_wrapper, NULL);
    add_timer(TIMER_LIST, HEARTBEAT_TIMER);
    alarm(TIME_SLOT);

    //bind a listening socket
    int listen_fd = bind_localhost_port();
    //add listening socket to event loop
    event_loop_add_fd(EVENT_LOOP, listen_fd, EPOLLIN, T_LISTEN);
    event_loop_add_fd(EVENT_LOOP, SIG_PIPE_FDS[0], EPOLLIN, T_SIGNAL);
    event_loop_add_fd(EVENT_LOOP, PIPE_FDS[0], EPOLLIN | EPOLLONESHOT, T_PIPE);
    log_trace("signal fd: %d, listen_fd: %d", SIG_PIPE_FDS[0], listen_fd);

    set_sockopt(listen_fd);

    listen(listen_fd, 1024);
    int stop_server = 0;

    while(!stop_server){
        int active_fd_num = event_loop_wait(EVENT_LOOP, -1);
        if (active_fd_num == -1){
            log_error_with_errno("event loop wait error");
            continue;
        }
        for(int i = 0; i < active_fd_num; i++){
            int fd;
            uint64_t u64 = EVENT_LOOP->events[i].data.u64;
            uint64_t tmp = (u64 & 0xFFFFFFFF00000000) >> 32;
            fd = (int)tmp;
            int fd_type = (int)(u64 & 0x00000000FFFFFFFF);

            log_trace("active fd: %d, fd_type: %d", fd, fd_type);
            switch (fd_type) {
                case T_LISTEN:
                {
                    //new connection
                    log_trace("new connection on listening port");
                    int fd_type_return = 0;
                    int connfd = connect_to_client(fd, &fd_type_return);
                    if (connfd == -1){
                        log_error("connection close unexpectedly");
                        continue;
                    }else if(connfd == 0){
                        log_error("token authentication failed");
                        continue;
                    }
                    if(fd_type_return == T_HEARTBEAT)
                        log_trace("heartbeat fd: %d", connfd);
                    /**
                     * no need to watch heartbeat fd, cause we're gonna handle it in the timer callback
                     * if (fd_type_return == 1)
                     *  event_loop_add_fd(EVENT_LOOP, connfd, EPOLLIN | EPOLLET, 1);
                     *
                     * neither does the notify fd, cause we are the sender, no need to wait for message
                     * if (fd_type_return == 2){
                        event_loop_add_fd(EVENT_LOOP, connfd, EPOLLIN | EPOLLET, 2);
                    **/

                    if (fd_type_return == T_FILE) {
                        event_loop_add_fd(EVENT_LOOP, connfd, EPOLLIN, T_FILE);
                    }
                    break;
                }
                case T_FILE:
                {
                    // TODO file upload
                    break;
                }
                case T_SIGNAL:
                {
                    pthread_mutex_lock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
                    // signal handling fd
                    char signal_buffer[16];
                    get_a_signal_from_pipe(signal_buffer);
                    if(str_contains(signal_buffer, "SIGALRM")){
                        tick(TIMER_LIST);
                        HEARTBEAT_TIMER->expire = time(NULL) + TIME_SLOT;
                        adjust_timer(TIMER_LIST, HEARTBEAT_TIMER);
                        alarm(TIME_SLOT);
                    }
                    pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
                    break;
                }
                case T_PIPE:
                {
                    log_trace("message coming from pipe");
                    char json_type[2];
                    readn(fd, json_type, 1);
                    log_trace("message type: %c", json_type[0]);
                    json_type[1] = '\0';
                    if(json_type[0] == '0') {
                        thpool_add_work(THREADING_POLL_P, notify_clients_wrapper, (void*)fd);
                    }else if(json_type[0] == '1'){
                        thpool_add_work(THREADING_POLL_P, close_actively_wrapper, (void*)fd);
                    break;
                }
                default:
                    break;
            }
        }
    }
}
    return 0;}

void clean_up(){
    free(REDIS_CONTEXT);
    remove(LOCK_PATH);
    log_info("clean up finished before exit");
}

void notify_clients_wrapper(void* data){
    int socket_fd = (int)data;
    SIG_CAUGHT_FLAG[socket_fd] = 0;
    pthread_mutex_lock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
    pthread_mutex_lock(&FD_MUTEX_ARRAY[socket_fd]);

    pthread_mutex_lock(&CLOSE_MUTEX[socket_fd]);
    if(CLOSE_FLAGS[socket_fd].closing == 1){
        pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);
        pthread_mutex_unlock(&FD_MUTEX_ARRAY[socket_fd]);
        pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
        return;
    }else{
        CLOSE_FLAGS[socket_fd].tid = pthread_self();
    }
    pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);

    notify_clients();
    SIG_CAUGHT_FLAG[socket_fd] = 1;

    pthread_mutex_unlock(&ACCEPT_MUTEX);
    pthread_mutex_unlock(&FD_MUTEX_ARRAY[socket_fd]);
    pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);

    pthread_mutex_lock(&CLOSE_MUTEX[socket_fd]);
    if(CLOSE_FLAGS[socket_fd].closing == 1){
        while(SIG_HANDLED_FLAG[socket_fd] != 1){
            sleep(5);
        }
    }else{
        CLOSE_FLAGS[socket_fd].tid = -1;
    }
    pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);
    rearm_oneshot_fd(EVENT_LOOP, PIPE_FDS[0], T_PIPE);
}

void heartbeat_callback_wrapper(void * data){
    int socket_fd = (int)data;
    SIG_CAUGHT_FLAG[socket_fd] = 0;
    pthread_mutex_lock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
    pthread_mutex_lock(&FD_MUTEX_ARRAY[socket_fd]);

    pthread_mutex_lock(&CLOSE_MUTEX[socket_fd]);
    if(CLOSE_FLAGS[socket_fd].closing == 1){
        pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);
        pthread_mutex_unlock(&FD_MUTEX_ARRAY[socket_fd]);
        pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
        return;
    }else{
        CLOSE_FLAGS[socket_fd].tid = pthread_self();
    }
    pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);

    heartbeat_callback(data);
    SIG_CAUGHT_FLAG[socket_fd] = 1;

    pthread_mutex_unlock(&ACCEPT_MUTEX);
    pthread_mutex_unlock(&FD_MUTEX_ARRAY[socket_fd]);
    pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);

    pthread_mutex_lock(&CLOSE_MUTEX[socket_fd]);
    if(CLOSE_FLAGS[socket_fd].closing == 1){
        while(SIG_HANDLED_FLAG[socket_fd] != 1){
            sleep(5);
        }
    }else{
        CLOSE_FLAGS[socket_fd].tid = -1;
    }
    pthread_mutex_unlock(&CLOSE_MUTEX[socket_fd]);
}

void close_actively_wrapper(void * data){
    pthread_mutex_lock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
    close_connection_actively(data);
    pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
}

