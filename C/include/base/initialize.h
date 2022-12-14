#ifndef _INITIALIZE_
#define _INITIALIZE_
#include "hiredis/hiredis.h"
void daemonize();
void parse_command_line_args(int argc, char** argv);
redisContext* connect_to_redis();
void set_lock_for_logger();
void init_springboot_server();
void init_signal_handle();
void init_threading_pool(int thread_num);
void load_configuration();
void assert_single_instance();
void init_recursive_mutex(pthread_mutex_t* mutex_p);
void init_close_flags();
void init_locks();
#endif