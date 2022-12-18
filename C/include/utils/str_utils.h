//
// Created by xiaoheng on 22-12-7.
//

#ifndef SERVER_PROTOCOL_STR_UTILS_H
#define SERVER_PROTOCOL_STR_UTILS_H
#include "protocol.h"
void print_logo();
void str_trim(char *str);
bool str_equals(const char *str1, const char *str2);
bool str_contains(const char *str, const char *substr);
bool str_starts_with(const char *str, const char *prefix);
bool str_ends_with(const char *str, const char *suffix);
void parse_heartbeat_responce(const char* msg, char* Token_buffer, int token_buffer_size);
void parse_notify_msg(const char* msg, struct notify_item * notify_msg_buffer);
void create_notify_json(char* puller, char* pull_target, char* json_buffer, int json_buffer_size);
void parse_connection_json(const char* json_msg,int* type, char* token_buffer, int token_buffer_size);
void parse_close_connection_json(const char* json_msg, int* heartbeat_fd);
void create_heartbeat_request_json(char* json_buffer);
#endif //SERVER_PROTOCOL_STR_UTILS_H
