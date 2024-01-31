#pragma once
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "../inc/log.h"
#include "../inc/util.h"
#define VERSION "1.0"

#define TYPE_BOOL 1
#define TYPE_STR 2
#define TYPE_INT 3

struct Option {
  char* value;
  char* name;
  char* desc;
  int type;
};

extern struct Option options[];
char* extract_value(char*);
bool parse_opt(char*);
bool get_bool(char*);
char* get_str(char*);
int get_int(char*);
void print_opts();
