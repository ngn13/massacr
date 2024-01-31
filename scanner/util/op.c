#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "../inc/op.h"
#include "../inc/log.h"
#include "../inc/util.h"

struct Option options[] = {
  {.name="no-color",    .type=TYPE_BOOL,    .value="false",                   .desc="Do not print colored output"},
  {.name="recvport",    .type=TYPE_INT,     .value="13421",                   .desc="Source port for TCP packets"},
  {.name="timeout",     .type=TYPE_INT,     .value="10",                      .desc="Timeout for receiver thread"},
  {.name="ports",       .type=TYPE_STR,     .value="common",                  .desc="Ports to scan for"},
  {.name="limit",       .type=TYPE_INT,     .value="20",                      .desc="Packets per second limit"},
  {.name="debug",       .type=TYPE_BOOL,    .value="false",                   .desc="Enable debug output"},
  {.name="url",         .type=TYPE_STR,     .value="http://localhost:3231",   .desc="Database HTTP(S) URL"},
  {.name="pwd",         .type=TYPE_STR,     .value="default",                 .desc="Database password"},
};

char* extract_value(char* o) {
  char* value = strtok(o, "=");
  if(NULL == value) return NULL;
  
  value = strtok(NULL, "=");
  if(NULL == value) return NULL;

  return value;
}

void print_opts(){
  int max_len = 0;
  for(int i = 0; i < sizeof(options)/sizeof(struct Option); i++){
    if(max_len < strlen(options[i].name))
      max_len = strlen(options[i].name);
  }

  for(int i = 0; i < sizeof(options)/sizeof(struct Option); i++){
    char spacebuf[(max_len-strlen(options[i].name))+2];
    for(int i = 0; i < sizeof(spacebuf); i++)
      spacebuf[i] = ' ';
    spacebuf[sizeof(spacebuf)-1] = '\0';

    if(get_bool("no-color")){
      printf(SPACE"%s%s=> %s\n", 
          options[i].name, spacebuf, options[i].value);
      continue;
    }

    if(options[i].type == TYPE_BOOL)
      printf(SPACE BOLD"%s"RESET"%s=> %s\n"RESET, 
          options[i].name, spacebuf, get_bool(options[i].name) ? GREEN"true" : RED"false");
    else
      printf(SPACE BOLD"%s"RESET"%s=> %s\n"RESET, 
          options[i].name, spacebuf, options[i].value);
  }
}

int get_int(char* name) {
  for(int i = 0; i < sizeof(options)/sizeof(struct Option); i++){
    if(eq(options[i].name, name))
      return atoi(options[i].value);
  }

  return -1;
}

bool get_bool(char* name){
  for(int i = 0; i < sizeof(options)/sizeof(struct Option); i++){
    if(eq(options[i].name, name))
      return eq(options[i].value, "true");
  }
  
  return false;
}

char* get_str(char* name) {
  for(int i = 0; i < sizeof(options)/sizeof(struct Option); i++){
    if(eq(options[i].name, name))
      return options[i].value;
  }

  return NULL;
}

void print_help() {
  info("massacr scanner %s: https://github.com/ngn13/massacr", VERSION);
  info("Listing available options:");
  
  int max_len = 0;
  for(int i = 0; i < sizeof(options)/sizeof(struct Option); i++){
    if(max_len < strlen(options[i].name))
      max_len = strlen(options[i].name);
  }

  for(int i = 0; i < sizeof(options)/sizeof(struct Option); i++){
    char spacebuf[(max_len-strlen(options[i].name))+2];
    for(int i = 0; i < sizeof(spacebuf); i++)
      spacebuf[i] = ' ';
    spacebuf[sizeof(spacebuf)-1] = '\0';

    if(get_bool("no-color")){
      printf(SPACE"--%s%s=> %s\n", 
          options[i].name, spacebuf, options[i].value);
      continue;
    }
      
    printf(SPACE BOLD"--%s"RESET"%s=> %s\n"RESET, 
        options[i].name, spacebuf, options[i].desc);
  }

  info("Example: "BOLD"massacr --no-color --recvport=1234 --limit=200 --ports=80,443"RESET);
  info("For more examples and details checkout the README");

  printf("\n");
  info("This program is free software: you can redistribute it and/or modify");
  printf(SPACE"it under the terms of the GNU General Public License as published by\n");
  printf(SPACE"the Free Software Foundation, either version 3 of the License, or\n");
  printf(SPACE"(at your option) any later version.\n");
  printf("\n");
  printf(SPACE"This program is distributed in the hope that it will be useful\n");
  printf(SPACE"but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
  printf(SPACE"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
  printf(SPACE"GNU General Public License for more details.\n");
  printf("\n");
  printf(SPACE"You should have received a copy of the GNU General Public License\n");
  printf(SPACE"along with this program.  If not, see <https://www.gnu.org/licenses/>.\n");
}

bool parse_opt(char* o) {
  if(eq(o, "--help") || eq(o, "--h")){
    print_help();
    exit(EXIT_SUCCESS);
  }

  for(int i = 0; i < sizeof(options)/sizeof(struct Option); i++){
    char fullop[strlen(options[i].name)+5];
    sprintf(fullop, "--%s", options[i].name);
    if(!startswith(o, fullop))
      continue;

    switch (options[i].type) {
      case TYPE_BOOL:
        if(neq(o, fullop))
          goto UNKNOWN;
        options[i].value = "true";
        return true;
        break;
      case TYPE_STR:
        options[i].value = extract_value(o);
        if(NULL == options[i].value){
          error("Value not specified for the option: %s", options[i].name);
          return false;
        }
        return true;
        break;
      case TYPE_INT:
        options[i].value = extract_value(o);
        if(NULL == options[i].value){
          error("Value not specified for the option: %s", options[i].name);
          return false;
        }

        if(atoi(options[i].value)<=0){
          error("Bad value for option: %s", options[i].name);
          return false;
        }

        return true;
        break;
    }
  }

UNKNOWN:
  error("Unknown option: %s", o);
  return false;
}
