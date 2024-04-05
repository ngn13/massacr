#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../inc/log.h"
#include "../inc/op.h"

void get_time(char *res) {
  time_t now = time(NULL);
  struct tm tm = *localtime(&now);

  sprintf(res, "%02d/%02d-%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_min,
          tm.tm_hour);
}

void info(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[16];
  get_time(tstr);

  if (get_bool("no-color")) {
    printf("[%s] INFO  # ", tstr);
    vprintf(msg, args);
    printf("\n");
    va_end(args);
    return;
  }

  printf(BOLD BLUE "[%s] INFO  # " RESET, tstr);
  vprintf(msg, args);
  printf(RESET "\n");
  va_end(args);
}

void error(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  char tstr[16];
  get_time(tstr);

  if (get_bool("no-color")) {
    printf("[%s] ERROR # ", tstr);
    vprintf(msg, args);
    printf("\n");
    va_end(args);
    return;
  }

  printf(BOLD RED "[%s] ERROR # " RESET, tstr);
  vprintf(msg, args);
  printf(RESET "\n");

  va_end(args);
}

void debug(const char *msg, ...) {
  va_list args;
  va_start(args, msg);

  if (!get_bool("debug"))
    return;

  char tstr[16];
  get_time(tstr);

  if (get_bool("no-color")) {
    printf("[%s] DEBUG # ", tstr);
    vprintf(msg, args);
    printf("\n");
    va_end(args);
    return;
  }

  printf(BOLD GRAY "[%s] DEBUG # " RESET, tstr);
  vprintf(msg, args);
  printf(RESET "\n");

  va_end(args);
}
