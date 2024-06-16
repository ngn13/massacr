#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct db_args {
  uint16_t port;
  uint32_t ipv4;
} db_args_t;

bool db_init(char *);
void db_add(void *);
void db_free();
