#ifndef _EVENT_LOOP_
#define _EVENT_LOOP_

struct event_loop{
    int epoll_fd;
    struct epoll_event *events;
    int event_count;
    int event_capacity;

};

int init_event_loop(struct event_loop *loop);


/**
 * add flag to event loop
 * @param loop event loop
 * @param fd the flag to watch
 * @param events the events to watch
 * @param type the type of flag (0:IO    1:signal    2:timer)
 * **/
int event_loop_add_fd(struct event_loop *loop, int fd, unsigned int events, int type);

int event_loop_del(struct event_loop *loop, int fd);

int event_loop_wait(struct event_loop *loop, int timeout);

int rearm_oneshot_fd(struct event_loop *loop, int fd, int type);
#endif