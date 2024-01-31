#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define contains(str, sub) (strchr(str, sub) != NULL)
#define startswith(str, sub) (strncmp(str, sub, strlen(sub)) == 0)
#define neq(s1, s2) (strcmp(s1, s2)!=0)
#define eq(s1, s2) (strcmp(s1, s2)==0)

struct Range {
  uint8_t ip[4];
  uint8_t net;
};

int* parse_ports(char*, int*);
void clean_ports(int*);

bool get_range(struct Range*, char*);
bool skip_range(struct Range*, uint8_t*);
extern int common_ports[];
extern char* bad_ranges[];
extern int bad_range_size;
