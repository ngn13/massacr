#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../inc/util.h"
#include "../inc/log.h"

int common_ports[] = {
  21, 990,        // ftp/ftps
  23, 22,         // telnet/ssh
  25, 465,        // smtp/smtps
  80, 443,        // http/https
  110, 995,       // pop3/pop3s
  119, 563,       // nntp/nntps
  143, 993,       // imap/imaps
  6667, 6697,     // irc
  137, 138, 139,  // netbios stuff
  53,             // dns
  69,             // tftp
  902,            // vmware
  873,            // rsync
  445,            // smb
  3389,           // rdp
};

// https://www.iana.org/assignments/iana-ipv4-special-registry/iana-ipv4-special-registry.xhtml
char* bad_ranges[] = {
  "0.0.0.0/8",
  "10.0.0.0/8",
  "100.64.0.0/10",
  "127.0.0.0/8",
  "169.254.0.0/16",
  "172.16.0.0/12",
  "192.0.0.0/24",
  "192.0.0.0/29",
  "192.0.0.8/32",
  "192.0.0.9/32",
  "192.0.0.170/32",
  "192.0.0.171/32",
  "192.0.2.0/24",
  "192.31.196.0/24",
  "192.52.193.0/24",
  "192.88.99.0/24",
  "192.168.0.0/16",
  "192.175.48.0/24",
  "198.18.0.0/15",
  "198.51.100.0/24",
  "203.0.113.0/24",
  "240.0.0.0/4",
  "255.255.255.255/32"
};
int bad_range_size = sizeof(bad_ranges)/sizeof(char*);

bool get_range(struct Range* res, char* ipnet){
  char* cpy = strdup(ipnet);
  char* ip = strtok(cpy, "/");
  if (NULL == ip)
    return false;
  
  char* net = strtok(NULL, "/");
  if (NULL == net || (res->net = atoi(net)) < 0 || res->net > 255)
    return false;

  char* ip0 = strtok(ip, ".");
  if (NULL == ip0 || (res->ip[0] = atoi(ip0)) < 0 || res->ip[0] > 255)
    return false;

  char* ip1 = strtok(NULL, ".");
  if (NULL == ip1 || (res->ip[1] = atoi(ip1)) < 0 || res->ip[1] > 255)
    return false;

  char* ip2 = strtok(NULL, ".");
  if (NULL == ip2 || (res->ip[2] = atoi(ip2)) < 0 || res->ip[2] > 255)
    return false;

  char* ip3 = strtok(NULL, ".");
  if (NULL == ip3 || (res->ip[3] = atoi(ip3)) < 0 || res->ip[3] > 255)
    return false;

  return true;
}

int* parse_ports(char* ports, int* size) {
  if(eq(ports, "common") || eq(ports, "c") || eq(ports, "top")){
    *size = sizeof(common_ports)/sizeof(int);
    return common_ports;
  }

  if(!contains(ports, '-') && !contains(ports, ',')){
    int port = atoi(ports);
    if(port <= 0){
      error("Bad port number");
      return NULL;
    }

    int* portlist = malloc(sizeof(int));
    portlist[0] = port;
    *size = 1;
    return portlist;
  }

  if(contains(ports, '-')) {
    char* rstart = strtok(ports, "-");
    if(rstart == NULL){
      error("Bad port range");
      return NULL;
    }

    char* rstop = strtok(NULL, "-");
    if(rstop == NULL){
      error("Bad port range");
      return NULL;
    }

    int r1 = atoi(rstart);
    int r2 = atoi(rstop);

    if(r1 <= 0 || r2 <= 0){
      error("Bad port range numbers");
      return NULL;
    }

    if(r2 <= r1) {
      error("Max port number should be bigger than min port number");
      return NULL;
    }

    int* portlist = malloc(sizeof(int));
    int portsize = 0;

    for(;r1 < r2; r1++){
      portlist[portsize] = r1;
      portsize++;
      portlist = realloc(portlist, sizeof(int)*(portsize+1));
    }
    
    *size = portsize;
    return portlist;
  }

  if(contains(ports, ',')) {
    char* cur = strtok(ports, ",");
    if(cur == NULL){
      error("Bad port list");
      return NULL;
    }

    int* portlist = malloc(sizeof(int));
    int portsize = 0;

    do{
      int curi = atoi(cur);
      if(curi <= 0) {
        error("Bad port number");
        return NULL;
      }

      portlist[portsize] = atoi(cur);
      portsize++;
      portlist = realloc(portlist, sizeof(int)*(portsize+1));
      cur = strtok(NULL, ",");
    }while(cur != NULL);

    *size = portsize;
    return portlist;
  }

  error("Bad port option");
  return NULL;
}

void clean_ports(int *ports) {
  if(ports == common_ports)
    return;

  free(ports);
}

bool skip_range(struct Range* ranges, uint8_t* cur){
  for(int i = 0; i < bad_range_size; i++){ 
    if(ranges[i].net == 8 && 
       ranges[i].ip[0] == cur[0]) return true;
    
    else if(ranges[i].net < 8 && 
            ranges[i].ip[0] <= cur[0]) return true;
    
    else if(ranges[i].net == 16 && 
            ranges[i].ip[0] == cur[0] && 
            ranges[i].ip[1] == cur[1]) return true;

    else if(ranges[i].net < 16 && 
            ranges[i].ip[0] == cur[0] && 
            ranges[i].ip[1] <= cur[1]) return true;

    else if(ranges[i].net == 24 && 
            ranges[i].ip[0] == cur[0] && 
            ranges[i].ip[1] == cur[1] &&
            ranges[i].ip[2] == cur[2]) return true;

    else if(ranges[i].net < 24 && 
            ranges[i].ip[0] == cur[0] && 
            ranges[i].ip[1] == cur[1] &&
            ranges[i].ip[2] <= cur[2]) return true;

    else if(ranges[i].net == 32 && 
            ranges[i].ip[0] == cur[0] && 
            ranges[i].ip[1] == cur[1] &&
            ranges[i].ip[2] == cur[2] &&
            ranges[i].ip[3] == cur[3]) return true;

    else if(ranges[i].net < 32 && 
            ranges[i].ip[0] == cur[0] && 
            ranges[i].ip[1] == cur[1] &&
            ranges[i].ip[2] == cur[2] &&
            ranges[i].ip[3] <= cur[3]) return true;
  }

  return false;
}
