//
// Created by xiaoheng on 22-12-7.
//

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "utils/str_utils.h"
#include "cJSON/cJSON.h"
#include "utils/log.h"
#include "protocol.h"

bool str_starts_with(const char *str, const char *prefix) {
    if (str == NULL || prefix == NULL) {
        return false;
    }
    while (*prefix != '\0') {
        if (*str++ != *prefix++) {
            return false;
        }
    }
    return true;
}

bool str_ends_with(const char *str, const char *suffix) {
    if (str == NULL || suffix == NULL) {
        return false;
    }
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (str_len < suffix_len) {
        return false;
    }
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}


bool str_contains(const char *str, const char *substr) {
    if (str == NULL || substr == NULL) {
        return false;
    }
    return strstr(str, substr) != NULL;
}

bool str_equals(const char *str1, const char *str2) {
    if (str1 == NULL || str2 == NULL) {
        return false;
    }
    return strcmp(str1, str2) == 0;
}


bool str_equals_ignore_case(const char *str1, const char *str2) {
    if (str1 == NULL || str2 == NULL) {
        return false;
    }
    return strcasecmp(str1, str2) == 0;
}

bool str_is_empty(const char *str) {
    return str == NULL || *str == '\0';
}

bool str_is_blank(const char *str) {
    if (str == NULL) {
        return true;
    }
    while (*str != '\0') {
        if (!isspace(*str++)) {
            return false;
        }
    }
    return true;
}

void str_trim(char *str) {
    if (str == NULL) {
        return;
    }
    char *start = str;
    while (*start != '\0' && isspace(*start)) {
        start++;
    }
    char *end = start;
    while (*end != '\0') {
        end++;
    }
    end--;
    while (end > start && isspace(*end)) {
        end--;
    }
    end++;
    *end = '\0';
    if (start != str) {
        while ((*str++ = *start++) != '\0');
    }
}

void str_trim_left(char *str) {
    if (str == NULL) {
        return;
    }
    char *start = str;
    while (*start != '\0' && isspace(*start)) {
        start++;
    }
    if (start != str) {
        while ((*str++ = *start++) != '\0');
    }
}

void str_trim_right(char *str) {
    if (str == NULL) {
        return;
    }
    char *end = str;
    while (*end != '\0') {
        end++;
    }
    end--;
    while (end > str && isspace(*end)) {
        end--;
    }
    end++;
    *end = '\0';
}

void str_replace(char *str, char old_char, char new_char) {
    if (str == NULL) {
        return;
    }
    while (*str != '\0') {
        if (*str == old_char) {
            *str = new_char;
        }
        str++;
    }
}

int str_split(const char *str, char delimiter, char **tokens, int max_token_count) {
    if (str == NULL || tokens == NULL || max_token_count == 0) {
        return -1;
    }
    size_t token_count = 0;
    while (*str != '\0') {
        if (token_count >= max_token_count) {
            break;
        }
        tokens[token_count++] = (char *) str;
        while (*str != '\0' && *str != delimiter) {
            str++;
        }
        if (*str == '\0') {
            break;
        }
        *(char *) str++ = '\0';
    }
    return (int) token_count;
}


void parse_heartbeat_responce(const char* msg, char* Token_buffer, int token_buffer_size){
    pthread_mutex_lock(&CJSON_MUTEX);
    cJSON* heartbeat_cJSON = NULL;
    heartbeat_cJSON =cJSON_Parse(msg);
    if (heartbeat_cJSON == NULL)
    {
        log_error("heartbeat_cJSON parse error");
    }
    cJSON* token = cJSON_GetObjectItem(heartbeat_cJSON, "token");
    strncpy(Token_buffer, token->valuestring, token_buffer_size);
    pthread_mutex_unlock(&CJSON_MUTEX);
}



void parse_notify_msg(const char* msg, struct notify_item * notify_msg_buffer){
    pthread_mutex_lock(&CJSON_MUTEX);
    cJSON* notify_cJSON = NULL;
    notify_cJSON =cJSON_Parse(msg);
    if (notify_cJSON == NULL)
    {
        log_error("notify_cJSON parse error");
    }
    cJSON* notify_type = cJSON_GetObjectItem(notify_cJSON, "type");
    notify_msg_buffer->type = notify_type->valueint;

    cJSON* notify_from = cJSON_GetObjectItem(notify_cJSON, "puller");
    strncpy(notify_msg_buffer->puller, notify_from->valuestring, sizeof(notify_msg_buffer->puller));

    cJSON* notify_to = cJSON_GetObjectItem(notify_cJSON, "pull_target");
    strncpy(notify_msg_buffer->pull_target, notify_to->valuestring, sizeof(notify_msg_buffer->pull_target));
    pthread_mutex_unlock(&CJSON_MUTEX);
}


void create_notify_json(char* puller, char* pull_target,char* json_buffer, int json_buffer_size){
    pthread_mutex_lock(&CJSON_MUTEX);
    cJSON* notify_cJSON = NULL;
    notify_cJSON =cJSON_CreateObject();
    if (notify_cJSON == NULL)
    {
        log_error("notify_cJSON create error");
    }
    cJSON_AddNumberToObject(notify_cJSON, "type", T_P2P);
    cJSON_AddNumberToObject(notify_cJSON, "puller", atoi(puller));
    cJSON_AddNumberToObject(notify_cJSON, "pull_target", atoi(pull_target));
    char* json = cJSON_Print(notify_cJSON);
    strncpy(json_buffer, json, json_buffer_size);
    free(json);
    cJSON_Delete(notify_cJSON);
    pthread_mutex_unlock(&CJSON_MUTEX);
}


void parse_connection_json(const char* json_msg,int* type, char* token_buffer, int token_buffer_size){
    pthread_mutex_lock(&CJSON_MUTEX);
    cJSON* connection_cJSON = NULL;
    connection_cJSON =cJSON_Parse(json_msg);
    if (connection_cJSON == NULL)
    {
        log_error("connection_cJSON parse error");
    }
    cJSON* connection_type = cJSON_GetObjectItem(connection_cJSON, "type");
    *type = connection_type->valueint;

    cJSON* connection_token = cJSON_GetObjectItem(connection_cJSON, "token");
    strncpy(token_buffer, connection_token->valuestring, token_buffer_size);
    pthread_mutex_unlock(&CJSON_MUTEX);
}

void parse_close_connection_json(const char* json_msg, int* heartbeat_fd){
    pthread_mutex_lock(&CJSON_MUTEX);
    cJSON* close_connection_cJSON = NULL;
    close_connection_cJSON =cJSON_Parse(json_msg);
    if (close_connection_cJSON == NULL)
    {
        log_error("close_connection_cJSON parse error");
    }
    cJSON* close_connection_heartbeat_fd = cJSON_GetObjectItem(close_connection_cJSON, "heartbeat_fd");
    *heartbeat_fd = close_connection_heartbeat_fd->valueint;
    pthread_mutex_unlock(&CJSON_MUTEX);
}


void create_heartbeat_request_json(char* json_buffer){
    pthread_mutex_lock(&CJSON_MUTEX);
    cJSON* heartbeat_cJSON = NULL;
    heartbeat_cJSON =cJSON_CreateObject();
    if (heartbeat_cJSON == NULL)
    {
        log_error("heartbeat_cJSON create error");
    }
    cJSON_AddNumberToObject(heartbeat_cJSON, "heartbeat", 1);
    char* json = cJSON_Print(heartbeat_cJSON);
    strncpy(json_buffer, json, SOCKET_BUFSIZE);
    free(json);
    pthread_mutex_unlock(&CJSON_MUTEX);
    cJSON_Delete(heartbeat_cJSON);
}





void print_logo(){
    printf("\033[34m   _________   ____________            _________ .__            __\n");
    printf("\033[34m  /  _  \\   \\ /   /\\_____  \\           \\_   ___ \\|  |__ _____ _/  |_\n");
    printf("\033[34m /  /_\\  \\   Y   /  /   |   \\   ______ /    \\  \\/|  |  \\\\__  \\\\   __\\\n");
    printf("\033[34m/    |    \\     /  /    |    \\ /_____/ \\     \\___|   Y  \\/ __ \\|  |\n");
    printf("\033[34m\\____|__  /\\___/   \\_______  /          \\______  /___|  (____  /__|\n");
    printf("\033[34m        \\/                 \\/                  \\/     \\/     \\/\n");
    printf("\033[0m");
}
