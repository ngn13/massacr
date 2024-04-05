#pragma once
#include <stdbool.h>
#include <stdint.h>

#define contains(str, sub) (strchr(str, sub) != NULL)
#define startswith(str, sub) (strncmp(str, sub, strlen(sub)) == 0)
#define neq(s1, s2) (strcmp(s1, s2) != 0)
#define eq(s1, s2) (strcmp(s1, s2) == 0)

typedef struct Range {
  uint8_t ip[4];
  uint8_t net;
} range_t;

int *parse_ports(char *, int *);
void clean_ports(int *);

bool get_range(range_t *, char *);
bool skip_range(range_t *, uint8_t *);
void urljoin(char *, char *, char *);
extern int common_ports[];
extern char *bad_ranges[];
extern int bad_range_size;
