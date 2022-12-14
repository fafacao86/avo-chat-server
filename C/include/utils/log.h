/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "string.h"
#include <errno.h>
#include "global.h"

#define LOG_VERSION "0.1.0"

#define LOG_USE_COLOR

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define log_error_with_errno(...) log_log_with_errno(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal_with_errno(...) log_log_with_errno(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn_with_errno(...) log_log_with_errno(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_info_with_errno(...) log_log_with_errno(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug_with_errno(...) log_log_with_errno(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_trace_with_errno(...) log_log_with_errno(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)


const char* log_level_string(int level);
void log_set_lock(log_LockFn fn, void *udata);
void log_set_level(int level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void *udata, int level);
int log_add_fp(FILE *fp, int level);

void log_log(int level, const char *file, int line, const char *fmt, ...);
void  log_log_with_errno(int level, const char *file, int line, const char *fmt, ...);

#endif
