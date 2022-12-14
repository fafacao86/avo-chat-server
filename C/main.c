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
    //init_springboot_server();

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
    HEARTBEAT_TIMER = create_timer(time(NULL) + TIME_SLOT, heartbeat_callback, NULL);
    add_timer(TIMER_LIST, HEARTBEAT_TIMER);
    alarm(TIME_SLOT);

    //bind a listening socket
    int listen_fd = bind_localhost_port();
    listen(listen_fd, 1024);

    //add listening socket to event loop
    event_loop_add_fd(EVENT_LOOP, listen_fd, EPOLLIN | EPOLLET, 0);
    event_loop_add_fd(EVENT_LOOP, SIG_PIPE_FDS[0], EPOLLIN | EPOLLET, 4);
    int stop_server = 0;

    while(!stop_server){
        int active_fd_num = event_loop_wait(EVENT_LOOP, -1);
        if (active_fd_num == -1){
            log_error_with_errno("event loop wait error");
            continue;
        }
        for(int i = 0; i < active_fd_num; i++){
            int fd = EVENT_LOOP->epoll_fd;
            int fd_type = (int)EVENT_LOOP->events[i].data.u32;
            switch (fd_type) {
                case T_LISTEN:
                {
                    //new connection
                    int fd_type_return = 0;
                    int connfd = connect_to_client(fd, &fd_type_return);
                    if (connfd == -1){
                        log_error("connection close unexpectedly");
                        continue;
                    }else if(connfd == 0){
                        log_error("token authentication failed");
                        continue;
                    }
                    /**
                     * no need to watch heartbeat fd, cause we're gonna handle it in the timer callback
                     * if (fd_type_return == 1)
                     *  event_loop_add_fd(EVENT_LOOP, connfd, EPOLLIN | EPOLLET, 1);
                     *
                     * neither does the notify fd, cause we are the sender, no need to wait for message
                     * if (fd_type_return == 2){
                        event_loop_add_fd(EVENT_LOOP, connfd, EPOLLIN | EPOLLET, 2);
                    **/

                    if (fd_type_return == 3) {
                        event_loop_add_fd(EVENT_LOOP, connfd, EPOLLIN | EPOLLET, 3);
                    }else if(fd_type_return == 4){
                        event_loop_add_fd(EVENT_LOOP, connfd, EPOLLIN | EPOLLET, 4);
                    }else if(fd_type_return == 5){
                        event_loop_add_fd(EVENT_LOOP, connfd, EPOLLIN | EPOLLET, 5);
                    }else{
                        log_error("unknown fd type in connection from client");
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
                }
                case T_PIPE:
                {
                    char json_type[2];
                    readn(fd, json_type, 2);
                    if(json_type[0] == '0') {
                        thpool_add_work(THREADING_POLL_P, notify_clients_wrapper, NULL);
                    }else if(json_type[0] == '1'){
                        thpool_add_work(THREADING_POLL_P, close_actively_wrapper, NULL);
                    break;
                }
                default:
                    break;
            }
        }
    }
    return 0;
}}

void clean_up(){
    free(REDIS_CONTEXT);
    remove(LOCK_PATH);
    log_info("clean up finished before exit");
}

void notify_clients_wrapper(void* data){
    pthread_mutex_lock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
    int fd;
    notify_clients(&fd);
    pthread_mutex_unlock(&ACCEPT_MUTEX);
    pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
}

void close_actively_wrapper(void * data){
    pthread_mutex_lock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
    close_connection_actively(data);
    pthread_mutex_unlock(&FD_MUTEX_ARRAY[SIG_PIPE_FDS[0]]);
}

