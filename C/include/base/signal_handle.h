#ifndef SERVER_PROTOCOL_SIGNAL_HANDLE_H
#define SERVER_PROTOCOL_SIGNAL_HANDLE_H
#define SIGSET_SIZE 20
#include "thpool.h"
typedef void (*sighandler_t)(int);
extern int sigset[SIGSET_SIZE];
extern sighandler_t sighandler_set[SIGSET_SIZE];
void signal_handle_sigint(int signal);
void signal_handle_sigterm(int signal);
void signal_handle_sigpipe(int signal);
void signal_handle_sigalrm(int signal);
void signal_handle_sigchld(int signal);
void install_signal_handler(int signal_num);
void init_sighandling_thread();
void start_signal_handle_thread();
#endif //SERVER_PROTOCOL_SIGNAL_HANDLE_H
