#include "base/event_loop.h"
#include "utils/log.h"
#include "utils/io_utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "global.h"
#define EVENT_MAX 1024

int init_event_loop(struct event_loop *loop)
{
    loop->epoll_fd = epoll_create(1024);
    if (loop->epoll_fd == -1) {
        log_error("epoll_create");
        return -1;
    }
    loop->events = malloc(sizeof(struct epoll_event) * EVENT_MAX );
    if (loop->events == NULL) {
        log_error("malloc");
        return -1;
    }
    loop->event_count = 0;
    loop->event_capacity = EVENT_MAX;
    return 0;
}

/**
 * add flag to event loop
 * @param loop event loop
 * @param fd the flag to watch
 * @param events the events to watch
 * @param type the type of flag (0:IO    1:signal    2:timer)
 * @return 0 on success, -1 on error
 * **/
int event_loop_add_fd(struct event_loop *loop, int fd, unsigned int events, int type)
{
    char type_name[32];
    if (loop->event_count == loop->event_capacity) {
        log_error("event loop is full");
        return -1;
    }
    set_nonblocking(fd);
    struct epoll_event event;
    event.data.u64 = fd;
    event.data.u64 = (event.data.u64 << 32) | type;
    event.events = events;
    if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1) {
        log_error("epoll_ctl");
        return -1;
    }
    loop->event_count++;
    return 0;
}

int event_loop_del(struct event_loop *loop, int fd)
{
    if (epoll_ctl(loop->epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        log_error("epoll_ctl");
        return -1;
    }
    loop->event_count--;
    return 0;
}

int event_loop_wait(struct event_loop *loop, int timeout)
{
    int count = epoll_wait(loop->epoll_fd, loop->events, EVENT_MAX, timeout);
    if (count == -1) {
        log_error("epoll_wait");
        return -1;
    }
    loop->event_count = count;
    return count;
}

void event_loop_destroy(struct event_loop *loop)
{
    close(loop->epoll_fd);
    free(loop->events);
}

