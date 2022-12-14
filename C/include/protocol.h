//
// Created by xiaoheng on 22-12-7.
//

#ifndef SERVER_PROTOCOL_PROTOCOL_H
#define SERVER_PROTOCOL_PROTOCOL_H


#define T_P2P 1
#define T_P2G 2

struct notify_item {
    char sender_token[2048];
    int type;
    char from[16];
    char to[16];
};



#define T_LISTEN 0
#define T_HEARTBEAT 1
#define T_NOTIFY 2
#define T_FILE 3
#define T_SIGNAL 4
#define T_PIPE 5
void heartbeat_callback(void* data);
void notify_clients(void* data);
void close_connection_actively(void*data);

#endif //SERVER_PROTOCOL_PROTOCOL_H
