//
// Created by xiaoheng on 22-12-7.
//

#ifndef SERVER_PROTOCOL_PROTOCOL_H
#define SERVER_PROTOCOL_PROTOCOL_H


#define T_P2P 1
#define T_P2G 2

struct notify_item {
    int type;
    char puller[16];
    char pull_target[16];
};



#define T_LISTEN 6
#define T_HEARTBEAT 1
#define T_NOTIFY 2
#define T_FILE 3
#define T_SIGNAL 4
#define T_PIPE 5
void heartbeat_callback(void* data);
void notify_clients(void* data);
void close_connection_actively(void*data);

#endif //SERVER_PROTOCOL_PROTOCOL_H
