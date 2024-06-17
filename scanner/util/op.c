#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../inc/log.h"
#include "../inc/op.h"
#include "../inc/util.h"

option_t options[] = {
    {.name = "no-color",   .type = TYPE_BOOL, .value = "false",                     .desc = "Do not print colored output"},
    {.name = "recvport",   .type = TYPE_INT,  .value = "1337",                      .desc = "Source port for TCP packets"},
    {.name = "timeout",    .type = TYPE_INT,  .value = "600",                       .desc = "Timeout for receiver thread"},
    {.name = "ports",      .type = TYPE_STR,  .value = "common",                    .desc = "Ports to scan for"          },
    {.name = "limit",      .type = TYPE_INT,  .value = "20",                        .desc = "Packets per second limit"   },
    {.name = "startpoint", .type = TYPE_STR,  .value = "0.0.0.0",                   .desc = "Address scanning startpoint"},
    {.name = "mongo",      .type = TYPE_STR,  .value = "mongodb://localhost:27017", .desc = "MongoDB URL"                },
    {.name = "threads",    .type = TYPE_INT,  .value = "10",                        .desc = "Database thread count"      },
    {.name = "debug",      .type = TYPE_BOOL, .value = "false",                     .desc = "Enable debug output"        },
};

char *extract_value(char *o) {
  char *value = strtok(o, "=");
  if (NULL == value)
    return NULL;

  value = strtok(NULL, "=");
  if (NULL == value)
    return NULL;

  return value;
}

void print_opts() {
  int max_len = 0;
  for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
    if (max_len < strlen(options[i].name))
      max_len = strlen(options[i].name);
  }

  for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
    char spacebuf[(max_len - strlen(options[i].name)) + 2];
    for (int i = 0; i < sizeof(spacebuf); i++)
      spacebuf[i] = ' ';
    spacebuf[sizeof(spacebuf) - 1] = '\0';

    if (options[i].type == TYPE_BOOL)
      printf(FG_BOLD "    %s" FG_RESET "%s=> %s\n" FG_RESET,
          options[i].name,
          spacebuf,
          get_bool(options[i].name) ? FG_GREEN "true" : FG_RED "false");
    else
      printf(FG_BOLD "    %s" FG_RESET "%s=> %s\n" FG_RESET, options[i].name, spacebuf, options[i].value);
  }
}

int get_int(char *name) {
  for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
    if (eq(options[i].name, name))
      return atoi(options[i].value);
  }

  return -1;
}

bool get_bool(char *name) {
  for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
    if (eq(options[i].name, name))
      return eq(options[i].value, "true");
  }

  return false;
}

char *get_str(char *name) {
  for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
    if (eq(options[i].name, name))
      return options[i].value;
  }

  return NULL;
}

void print_help() {
  info("Listing available options:");

  int max_len = 0;
  for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
    if (max_len < strlen(options[i].name))
      max_len = strlen(options[i].name);
  }

  for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
    int  spacesz = (max_len - strlen(options[i].name)) + 2;
    char spacebuf[spacesz];

    for (int e = 0; e < spacesz; e++)
      spacebuf[e] = ' ';
    spacebuf[spacesz - 1] = '\0';

    printf(FG_BOLD "    --%s" FG_RESET "%s=> %s\n" FG_RESET, options[i].name, spacebuf, options[i].desc);
  }

  printf("\n");
}

bool parse_opt(char *o) {
  if (eq(o, "--help") || eq(o, "-h")) {
    print_help();
    exit(EXIT_SUCCESS);
  }

  for (int i = 0; i < sizeof(options) / sizeof(option_t); i++) {
    char fullop[strlen(options[i].name) + 5];
    sprintf(fullop, "--%s", options[i].name);
    if (!startswith(o, fullop))
      continue;

    switch (options[i].type) {
    case TYPE_BOOL:
      if (!eq(o, fullop))
        goto unknown;
      options[i].value = "true";
      return true;
      break;
    case TYPE_STR:
      options[i].value = extract_value(o);
      if (NULL == options[i].value) {
        error("Value not specified for the option: %s", options[i].name);
        return false;
      }
      return true;
      break;
    case TYPE_INT:
      options[i].value = extract_value(o);
      if (NULL == options[i].value) {
        error("Value not specified for the option: %s", options[i].name);
        return false;
      }

      if (atoi(options[i].value) <= 0) {
        error("Bad value for option: %s", options[i].name);
        return false;
      }

      return true;
      break;
    }
  }

unknown:
  error("Unknown option: %s", o);
  return false;
}
