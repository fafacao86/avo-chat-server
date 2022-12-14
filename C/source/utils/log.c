/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "../../include/utils/log.h"

#define MAX_CALLBACKS 32

typedef struct {
  log_LogFn fn;
  void *udata;
  int level;
} Callback;

static struct {
  void *udata;
  log_LockFn lock;
  int level;
  bool quiet;
  Callback callbacks[MAX_CALLBACKS];
} L;


static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif


static void stdout_callback(log_Event *ev) {
  char buf[32];
  buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
#ifdef LOG_USE_COLOR
  fprintf(
    ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
    buf, level_colors[ev->level], level_strings[ev->level],
    ev->file, ev->line);
#else
  fprintf(
    ev->udata, "%s %-5s %s:%d: ",
    buf, level_strings[ev->level], ev->file, ev->line);
#endif
  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}


static void file_callback(log_Event *ev) {
  char buf[64];
  buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
  fprintf(
    ev->udata, "%s %-5s %s:%d: ",
    buf, level_strings[ev->level], ev->file, ev->line);
  vfprintf(ev->udata, ev->fmt, ev->ap);
  fprintf(ev->udata, "\n");
  fflush(ev->udata);
}


static void lock(void)   {
  if (L.lock) { L.lock(true, L.udata); }
}


static void unlock(void) {
  if (L.lock) { L.lock(false, L.udata); }
}


const char* log_level_string(int level) {
  return level_strings[level];
}


void log_set_lock(log_LockFn fn, void *udata) {
  L.lock = fn;
  L.udata = udata;
}


void log_set_level(int level) {
  L.level = level;
}


void log_set_quiet(bool enable) {
  L.quiet = enable;
}


int log_add_callback(log_LogFn fn, void *udata, int level) {
  for (int i = 0; i < MAX_CALLBACKS; i++) {
    if (!L.callbacks[i].fn) {
      L.callbacks[i] = (Callback) { fn, udata, level };
      return 0;
    }
  }
  return -1;
}


int log_add_fp(FILE *fp, int level) {
  return log_add_callback(file_callback, fp, level);
}


static void init_event(log_Event *ev, void *udata) {
  if (!ev->time) {
    time_t t = time(NULL);
    ev->time = localtime(&t);
  }
  ev->udata = udata;
}


void log_log(int level, const char *file, int line, const char *fmt, ...) {
  log_Event ev = {
    .fmt   = fmt,
    .file  = file,
    .line  = line,
    .level = level,
  };

  lock();

  if (!L.quiet && level >= L.level) {
    init_event(&ev, stdout);
    va_start(ev.ap, fmt);
    stdout_callback(&ev);
    va_end(ev.ap);
  }

  for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
    Callback *cb = &L.callbacks[i];
    if (level >= cb->level) {
      init_event(&ev, cb->udata);
      va_start(ev.ap, fmt);
      cb->fn(&ev);
      va_end(ev.ap);
    }
  }

  unlock();
}

void log_log_with_errno(int level, const char *file, int line, const char *fmt, ...){
    char fmt_buf[256];
    snprintf(fmt_buf, sizeof(fmt_buf), "%s: %s", fmt, strerror(errno));

    log_Event ev = {
            .fmt   = fmt_buf,
            .file  = file,
            .line  = line,
            .level = level,
    };

    lock();

    if (!L.quiet && level >= L.level) {
        init_event(&ev, stdout);
        va_start(ev.ap, fmt_buf);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt_buf);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    unlock();
}
//
//
//void log_fatal_with_errno(const char* message){
//    char* buf = (char*)malloc(ERRNO_MESSAGE_BUFSIZE);
//    if (buf == NULL){
//        log_fatal("Failed to allocate memory for error message");
//    }
//    strerror_r(errno, buf, ERRNO_MESSAGE_BUFSIZE);
//    log_fatal("%s: %s", message, buf);
//    free(buf);
//}
//
//void log_error_with_errno(const char* message){
//    char* buf = (char*)malloc(ERRNO_MESSAGE_BUFSIZE);
//    if (buf == NULL){
//        log_error("Failed to allocate memory for error message");
//    }
//    strerror_r(errno, buf, ERRNO_MESSAGE_BUFSIZE);
//    log_trace("%s: %s", message, buf);
//    free(buf);
//}
//
//void log_warn_with_errno(const char* message){
//    char* buf = (char*)malloc(ERRNO_MESSAGE_BUFSIZE);
//    if (buf == NULL){
//        log_warn("Failed to allocate memory for error message");
//    }
//    strerror_r(errno, buf, ERRNO_MESSAGE_BUFSIZE);
//    log_warn("%s: %s", message, buf);
//    free(buf);
//}
//
//void log_info_with_errno(const char* message){
//    char* buf = (char*)malloc(ERRNO_MESSAGE_BUFSIZE);
//    if (buf == NULL){
//        log_info("Failed to allocate memory for error message");
//    }
//    strerror_r(errno, buf, ERRNO_MESSAGE_BUFSIZE);
//    log_info("%s: %s", message, buf);
//    free(buf);
//}
//
//void log_debug_with_errno(const char* message){
//    char* buf = (char*)malloc(ERRNO_MESSAGE_BUFSIZE);
//    if (buf == NULL){
//        log_debug("Failed to allocate memory for error message");
//    }
//    strerror_r(errno, buf, ERRNO_MESSAGE_BUFSIZE);
//    log_debug("%s: %s", message, buf);
//    free(buf);
//}
//
//void log_trace_with_errno(const char* message){
//    char* buf = (char*)malloc(ERRNO_MESSAGE_BUFSIZE);
//    if (buf == NULL){
//        log_trace("Failed to allocate memory for error message");
//    }
//    strerror_r(errno, buf, ERRNO_MESSAGE_BUFSIZE);
//    log_trace("%s: %s", message, buf);
//    free(buf);
//}