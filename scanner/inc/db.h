#pragma once 
#include <stdbool.h>
#include <curl/curl.h>
#include "../inc/log.h"

struct Entry {
  char* ip;
  int port;
};

bool check_db(char*, char*);
bool add_entry(void* entry);
