#pragma once
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libnet.h>

#include "../inc/log.h"
#include "../inc/db.h"

struct ThreadArgs {
  uint16_t port;
  bool should_run;
};

extern pthread_mutex_t lock;
bool receive(struct ThreadArgs*);
bool syn(char*, int, int);
