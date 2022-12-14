#ifndef SERVER_PROTOCOL_TIMER_H
#define SERVER_PROTOCOL_TIMER_H

#include <time.h>
typedef void (*callback)(void *);



struct timer{
    time_t expire;
    void (*callback)(void *);
    void* callback_parameter;
    struct timer* prev;
    struct timer* next;
};


struct sort_timer_list{
    struct timer* head;
    struct timer* tail;
};


void init_sort_timer_list(struct sort_timer_list **list);

//create a timer, need some attribute settings to do before add it to list
/*params:
* expire is the timestamp than the timer expires
* callback is the function that will be called when the timer expires
* callback_parameter is the parameter that will be passed to the callback function
*/
struct timer* create_timer(time_t expire, callback cb, void* cb_para);


//add timer to the given sort_timer_list
/*
 * params:
 * list: the list that the timer will be added to
 * timer: the timer that will be added to the list
 */
void add_timer(struct sort_timer_list* list, struct timer* timer);


//adjust timer's position when it's timeout changed, only consider the timer's expire time being extended.
/*
 * params:
 * list: the list that the timer is in
 * timer: the timer that will be adjusted
 */
void adjust_timer(struct sort_timer_list *list, struct timer *timer);


//delete timer from the given sort_timer_list
/*
 * params:
 * list: the list that the timer is in
 * timer: the timer that will be deleted
 */
void del_timer(struct sort_timer_list* list, struct timer* timer);


//handle the expired timers
/*
 * params:
 * list: the list that the timer is in
 */
void tick(struct sort_timer_list *list);
#endif
