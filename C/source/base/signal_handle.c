#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utils/log.h"
#include "stddef.h"
#include "base/signal_handle.h"
#include "utils/io_utils.h"
void (*sighandler_set[SIGSET_SIZE])(int);
int sigset[SIGSET_SIZE];


void install_signal_handler(int signal_num)
{
    for (int i = 0; i < signal_num; ++i) {
        if (signal(sigset[i], sighandler_set[i]) == SIG_ERR) {
            log_fatal_with_errno("signal error");
        }
    }
}

void signal_handle_sigint(int signal){
    if(IS_DAEMON){
        log_debug("signal_handle_sigint, ignore SIGINT");
    }else{
        log_warn("SIGINT caught, exiting...");
        log_info("Closing child process...");
        kill(CHILD_PID, SIGINT);
        exit(0);
    }
}

void signal_handle_sigterm(int signal){
    if(IS_DAEMON){
        log_debug("signal_handle_sigterm, ignore SIGTERM");
    }else{
        log_warn("SIGTERM caught, exiting...");
        log_info("Closing child process...");
        kill(CHILD_PID, SIGINT);
        exit(0);
    }
}

void signal_handle_sigpipe(int signal){
    if(IS_DAEMON){
        log_debug("signal_handle_sigpipe, ignore SIGPIPE");
        }else{
        log_warn("SIGPIPE caught");
    }
}

void signal_handle_sigalrm(int signal){
    log_debug("signal_handle_sigalrm");
    send_a_signal_to_pipe("<SIGALRM>");
}



void signal_handle_sigchld(int signal){
    log_debug("signal_handle_sigchld");
    pid_t   pid;
    int exit_flag;
    int     stat;
    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        log_error("child pid:%d terminated unexpectedly", pid);
        log_debug("shutdown server...");
        exit_flag = 1;
    }
    if (exit_flag) {
        exit(-1);
    }

}

static void signal_handle_thread(void* arg){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGPIPE);
    sigaddset(&set, SIGALRM);

    int s, sig;

    for (;;) {
        s = sigwait(&set, &sig);
        if (s != 0)
            log_error_with_errno("sigwait");
        log_info("signal_handle_thread: got signal %d", sig);
        switch (sig) {
            case SIGINT:
                signal_handle_sigint(sig);
                break;
            case SIGTERM:
                signal_handle_sigterm(sig);
                break;
            case SIGPIPE:
                signal_handle_sigpipe(sig);
                break;
            case SIGALRM:
                signal_handle_sigalrm(sig);
                break;
            case SIGCHLD:
                signal_handle_sigchld(sig);
                break;
            default:
                log_error("unexpected signal %d", sig);
        }
    }

}


 /**
  * route all signals to a single thread
  * to prevent other threads to be interrupted, causing some undefined error
  * Caution: This function must be called before creating threading-pool!
  *          Because the pthread_sigmask will be inherited by child threads
  * **/
void init_sighandling_thread(){
    pipe(SIG_PIPE_FDS);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGPIPE);
    sigaddset(&set, SIGALRM);

    if(pthread_sigmask(SIG_BLOCK, &set, NULL)){
        log_error_with_errno("pthread_sigmask");
    }
}


/**
 * called after threading pool initialization
 * **/
void start_signal_handle_thread(){
    thpool_add_work(THREADING_POLL_P, signal_handle_thread, NULL);
    log_info("Successfully started a separate signal handle thread");
}

