#include<stddef.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "global.h"
#include "utils/log.h"


/** read n bytes from a file descriptor
 *
 * @param fd the file descriptor
 * @param vptr the buffer to store the data
 * @param n the number of bytes to read
 * @return -1 indicates error, 0 indicates connection closed, >=0 indicates bytes read.
 * Note: this function will block until all n bytes are read,
 *       when peer closed the connection, the process will deal with the TCP closing routine then return number of bytes read.
 *       The reason to close the connection in this reading function other than defer it in signal handler
 *       is to avoid that the peer closed the connection but the process still try to write to the peer. It may cause problems
 *       like SIGPIPE and TCP RESET which could be very tricky.
 */

int readn(int fd, void *vptr, int n)
{

    int  nleft = n;
    int nread;
    char   *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = (int)read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR) {
                return 0;
            }/* and call read() again */
        } else if (nread == 0){
            log_error("client closed the connection actively, fd: %d", fd);
            close(fd);
            break;
        }/* EOF */
        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;         /* return >= 0*/
}


// write n bytes from a file descriptor
int writen(int fd, const void *vptr, int n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0&& errno == EINTR){
                return 0;
            }/* and call write() again */
            else{
                log_error_with_errno("writen error");
                return (-1);    /* error */
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

// read a line from a file descriptor
ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
again:
        if ( (rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;          /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1);     /* EOF, n - 1 bytes were read */
        } else {
            if (errno == EINTR)
                goto again;
            return (-1);        /* error, errno set by read() */
        }
    }
    *ptr = 0;                   /* null terminate like fgets() */
    return (n);
}



int get_json_string_from_socket(char* json_buffer, int socket_fd){
    char message_size[SOCKET_MSG_DIGIT_NUM+1] = {'\0'};
    int read_fd = socket_fd;
    int read_size = readn(read_fd, message_size, SOCKET_MSG_DIGIT_NUM);
    if(read_size == 0){
        log_error("socket closed %d", read_fd);
        return -1;
    }else if(read_size != SOCKET_MSG_DIGIT_NUM){
        log_error("read size error");
        return -1;
    }
    int json_size = atoi(message_size);
    if(json_size > SOCKET_BUFSIZE){
        log_error("json size error");
        return -1;
    }
    read_size = readn(read_fd, json_buffer, json_size);
    if(read_size != json_size){
        log_error("read size error");
        return -1;
    }else if (read_size == 0){
        log_error("socket closed %d", read_fd);
        return -1;
    }
    json_buffer[json_size] = '\0';

    return 0;
}


int send_json_string_to_socket(char* json_buffer, int socket_fd){
    int json_size = (int)strlen(json_buffer);
    if(json_size > SOCKET_BUFSIZE){
        log_error("json string too long error");
        return -1;
    }
    char message_size[SOCKET_MSG_DIGIT_NUM+1] = {'\0'};
    sprintf(message_size, "%06d", json_size);
    int write_fd = socket_fd;
    int write_size = writen(write_fd, message_size, SOCKET_MSG_DIGIT_NUM);
    if(write_size != SOCKET_MSG_DIGIT_NUM){
        log_error("write size error");
        return -1;
    }
    write_size = writen(write_fd, json_buffer, json_size);
    if(write_size != json_size){
        log_error("write size error");
        return -1;
    }
    return 0;
}


// get a json string from pipe
int get_json_string_from_pipe(char* json_buffer){
    char message_size[PIPE_MSG_DIGIT_NUM+1] = {'\0'};
    int read_fd = PIPE_FDS[0];
    int read_size = readn(read_fd, message_size, PIPE_MSG_DIGIT_NUM);
    if (read_size == 0){
        log_error("socket closed %d", read_fd);
        return -1;
    }else if(read_size != PIPE_MSG_DIGIT_NUM){
        log_error("read size error");
        return -1;
    }
    int json_size = atoi(message_size);
    if(json_size > PIPE_MSG_BUFSIZE){
        log_error("json size error");
        return -1;
    }
    read_size = readn(read_fd, json_buffer, json_size);
    if(read_size != json_size){
        log_error("read size error");
        return -1;
    }else if (read_size == 0){
        log_error("socket closed %d", read_fd);
        return -1;
    }
    json_buffer[json_size] = '\0';
    return 0;
}

//send a json string to pipe
int send_json_string_to_pipe(char* json_buffer){
    int json_size = (int)strlen(json_buffer);
    if(json_size > PIPE_MSG_BUFSIZE){
        log_error("json size error");
        return -1;
    }
    char message_size[PIPE_MSG_DIGIT_NUM+1] = {'\0'};
    sprintf(message_size, "%04d", json_size);
    int write_fd = PIPE_FDS[1];
    int write_size = writen(write_fd, message_size, PIPE_MSG_DIGIT_NUM);
    if(write_size != PIPE_MSG_DIGIT_NUM){
        log_error("write size error");
        return -1;
    }
    write_size = writen(write_fd, json_buffer, json_size);
    if(write_size != json_size){
        log_error("write size error");
        return -1;
    }
    return 0;
}


// get a pending signal log from sig_pipe
int get_a_signal_from_pipe(char* signal_buffer){
    char message_size[SIG_MSG_DIGIT_NUM+1] = {'\0'};
    int read_fd = SIG_PIPE_FDS[0];
    int read_size = readn(read_fd, message_size, SIG_MSG_DIGIT_NUM);
    if (read_size == 0){
        log_error("socket closed %d", read_fd);
        return -1;
    }else if(read_size != SIG_MSG_DIGIT_NUM){
        log_error("read size error");
        return -1;
    }

    int signal_size = atoi(message_size);
    if(signal_size > SIG_MSG_BUFSIZE){
        log_error("json size error");
        return -1;
    }else if (signal_size == 0){
        log_error("socket closed %d", read_fd);
        return -1;
    }

    read_size = readn(read_fd, signal_buffer, signal_size);
    if(read_size != signal_size){
        log_error("read size error");
        return -1;
    }
    signal_buffer[signal_size] = '\0';
    return 0;
}

//send a pending signal log to sig_pipe
int send_a_signal_to_pipe(char* signal_buffer){
    int signal_size = (int)strlen(signal_buffer);
    if(signal_size > SIG_MSG_BUFSIZE){
        log_error("json size error");
        return -1;
    }
    char message_size[PIPE_MSG_DIGIT_NUM+1] = {'\0'};
    sprintf(message_size, "%02d", signal_size);
    int write_fd = SIG_PIPE_FDS[1];
    int write_size = writen(write_fd, message_size, SIG_MSG_DIGIT_NUM);

    if(write_size != SIG_MSG_DIGIT_NUM){
        log_error("write size error");
        return -1;
    }
    write_size = writen(write_fd, signal_buffer, signal_size);
    if(write_size != signal_size){
        log_error("write size error");
        return -1;
    }
    return 0;
}


int set_nonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

