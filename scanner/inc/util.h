#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define contains(str, sub) (strchr(str, sub) != NULL)
#define startswith(str, sub) (strncmp(str, sub, strlen(sub)) == 0)
#define neq(s1, s2) (strcmp(s1, s2) != 0)
#define eq(s1, s2) (strcmp(s1, s2) == 0)

typedef struct subnet {
  uint8_t ip[4];
  uint8_t net;
} subnet_t;

uint16_t *parse_ports(char *);
void      clean_ports(uint16_t *);

bool get_subnet(subnet_t *, char *);
bool subnet_contains(uint32_t, subnet_t *, size_t);
void uint32_to_ipstr(char *, uint32_t);
