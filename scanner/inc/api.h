#pragma once
#include "../inc/log.h"
#include <stdbool.h>

typedef struct APIArgs {
  char *ip;
  int port;
} api_args_t;

bool api_check();
bool api_add(void *);
