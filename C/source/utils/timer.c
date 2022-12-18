#include "utils/timer.h"
#include <stdlib.h>
#include <unistd.h>
#include "base/thpool.h"
#include "global.h"
#include "log.h"

void init_sort_timer_list(struct sort_timer_list **list){
    *list = malloc(sizeof(struct sort_timer_list));
    (*list)->head = NULL;
    (*list)->tail = NULL;
}


//create a timer, need some attribute settings to do before add it to list
/*params:
* expire is the timestamp than the timer expires
* callback is the function that will be called when the timer expires
* callback_parameter is the parameter that will be passed to the callback function
*/
struct timer* create_timer(time_t expire, callback cb, void* cb_para){
    struct timer* new_timer = (struct timer*)malloc(sizeof(struct timer));
    new_timer->expire = expire;
    new_timer->callback = cb;
    new_timer->callback_parameter = cb_para;
    new_timer->prev = NULL;
    new_timer->next = NULL;
    return new_timer;
}


//add target_timer after head_timer
//this helper function will be called by add_timer and adjust_timer
static void add_timer_helper(struct sort_timer_list *list, struct timer *target_timer, struct timer* head_timer){
    struct timer* prev_timer = head_timer;
    struct timer* tmp = head_timer->next;
    //traverse the timers after head_timer,until find a timer whose expire is bigger than target_timer
    while(tmp){
        if(tmp->expire > target_timer->expire){
            prev_timer->next = target_timer;
            target_timer->prev = prev_timer;
            target_timer->next = tmp;
            tmp->prev = target_timer;
            break;
        }
        prev_timer = tmp;
        tmp = tmp->next;
    }
    //if target_timer is the biggest timer, add it to the tail of the list
    if(!tmp){
        prev_timer->next = target_timer;
        target_timer->prev = prev_timer;
        target_timer->next = NULL;
        list->tail = target_timer;
    }
}

//add timer to the given sort_timer_list
void add_timer(struct sort_timer_list* list, struct timer* timer){
    //if list is empty
    if(list->head == NULL){
        list->head = timer;
        list->tail = timer;
        return;
    }
    //if timer is the smallest timer
    if(timer->expire < list->head->expire){
        timer->next = list->head;
        list->head->prev = timer;
        list->head = timer;
        return;
    }
    //if timer is the biggest timer
    if(timer->expire > list->tail->expire){
        list->tail->next = timer;
        timer->prev = list->tail;
        list->tail = timer;
        return;
    }
    add_timer_helper(list, timer, list->head);
}


//adjust timer's position when it's timeout changed, only consider the timer's expire time being extended.
void adjust_timer(struct sort_timer_list *list, struct timer *timer){
    if(!timer){
        return;
    }
    struct timer* tmp = timer->next;
    //if timer is the biggest timer, no need to adjust
    if(!tmp){
        return;
    }
    //if timer's expire time is still smaller than the next timer, no need to adjust
    if(timer->expire < tmp->expire){
        return;
    }
    //if timer is head
    if(timer->prev == NULL){
        list->head = timer->next;
        timer->next->prev = NULL;
        timer->next = NULL;
        add_timer_helper(list, timer, list->head);
    }else{
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        timer->next = NULL;
        add_timer_helper(list, timer, timer->prev);
    }
}

//delete timer from the given sort_timer_list
void del_timer(struct sort_timer_list* list, struct timer* timer){
    if(!timer){
        return;
    }
    //if timer is head
    if(timer->prev == NULL){
        list->head = timer->next;
        if(list->head){
            list->head->prev = NULL;
        }
        //if timer is the only timer in the list
        if(list->tail == timer && list->head == timer){
            list->tail = NULL;
        }
    }else if(timer->next == NULL){
        //if timer is tail
        list->tail = timer->prev;
        timer->prev->next = NULL;
    }else{
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
    }
    free(timer);
}

//handle the expired timers
void tick(struct sort_timer_list *list){
    if(!list->head){
        return;
    }
    time_t cur = time(NULL);
    struct timer* tmp = list->head;
    //handle all expired timer from head
    while(tmp){
        if(cur < tmp->expire){
            break;
        }
        thpool_add_work(THREADING_POLL_P, tmp->callback, tmp->callback_parameter);

        tmp = tmp->next;
    }
}
