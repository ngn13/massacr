#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/log.h"
#include "../inc/util.h"

uint16_t common_ports[] = {
    21,   // ftp
    990,  // ftps
    23,   // telnet
    22,   // ssh
    25,   // smtp
    465,  // smtps
    80,   // http
    443,  // https
    110,  // pop3
    995,  // pop3s
    119,  // nntp
    563,  // nntps
    143,  // imap
    993,  // imaps
    6667, // irc
    6697, // ircs
    137,  // netbios stuff
    138,  // more netbios stuff
    139,  // even more netbios stuff
    53,   // dns
    69,   // tftp
    902,  // vmware
    873,  // rsync
    445,  // smb
    3389, // rdp
    0,    // marks the end of the list
};

bool get_subnet(subnet_t *res, char *sub) {
  size_t ipnet_len = strlen(sub);
  char   ipnet_copy[ipnet_len], *save;

  memcpy(ipnet_copy, sub, ipnet_len + 1);

  char *ip = strtok_r(ipnet_copy, "/", &save);
  if (NULL == ip)
    return false;

  char *net = strtok_r(NULL, "/", &save);
  if (NULL == net || (res->net = atoi(net)) < 0 || res->net > 255)
    return false;

  char *ip0 = strtok_r(ip, ".", &save);
  if (NULL == ip0 || (res->ip[0] = atoi(ip0)) < 0 || res->ip[0] > 255)
    return false;

  char *ip1 = strtok_r(NULL, ".", &save);
  if (NULL == ip1 || (res->ip[1] = atoi(ip1)) < 0 || res->ip[1] > 255)
    return false;

  char *ip2 = strtok_r(NULL, ".", &save);
  if (NULL == ip2 || (res->ip[2] = atoi(ip2)) < 0 || res->ip[2] > 255)
    return false;

  char *ip3 = strtok_r(NULL, ".", &save);
  if (NULL == ip3 || (res->ip[3] = atoi(ip3)) < 0 || res->ip[3] > 255)
    return false;

  return true;
}

bool subnet_contains(uint32_t ipv4, subnet_t *subnets, size_t subnet_size) {
  subnet_t cur = {.net = 0};
  cur.ip[0]    = (ipv4 >> 24) & 255;
  cur.ip[1]    = (ipv4 >> 16) & 255;
  cur.ip[2]    = (ipv4 >> 8) & 255;
  cur.ip[3]    = ipv4 & 255;

  for (size_t i = 0; i < subnet_size; i++) {
    // subnet is smaller then 8, we should make sure
    // that first octet is smaller/equal to the first octet
    if (subnets[i].net < 8)
      return subnets[i].ip[0] <= cur.ip[0];

    // subnet is equal to/larger than 8, so we should
    // make sure that the first 8 bits match
    if (subnets[i].ip[0] != cur.ip[0])
      return false;

    // do the same for all the other octets
    if (subnets[i].net < 16)
      return subnets[i].ip[1] <= cur.ip[1];

    if (subnets[i].ip[1] != cur.ip[1])
      return false;

    if (subnets[i].net < 24)
      return subnets[i].ip[2] <= cur.ip[2];

    if (subnets[i].ip[2] != cur.ip[2])
      return false;

    if (subnets[i].net < 32)
      return subnets[i].ip[3] <= cur.ip[3];

    if (subnets[i].ip[3] != cur.ip[3])
      return false;

    return true;
  }

  return false;
}

uint16_t *parse_ports(char *ports) {
  if (eq(ports, "common") || eq(ports, "c") || eq(ports, "top"))
    return common_ports;

  if (!contains(ports, '-') && !contains(ports, ',')) {
    int port = atoi(ports);
    if (port <= 0) {
      error("Bad port number");
      return NULL;
    }

    uint16_t *portlist = malloc(sizeof(uint16_t) * 2);
    portlist[0]        = port;
    portlist[1]        = 0;
    return portlist;
  }

  if (contains(ports, '-')) {
    char *rstart = strtok(ports, "-");
    if (rstart == NULL) {
      error("Bad port range");
      return NULL;
    }

    char *rstop = strtok(NULL, "-");
    if (rstop == NULL) {
      error("Bad port range");
      return NULL;
    }

    int r1 = atoi(rstart);
    int r2 = atoi(rstop);

    if (r1 <= 0 || r2 <= 0 || r1 > UINT16_MAX || r2 > UINT16_MAX) {
      error("Bad port range numbers");
      return NULL;
    }

    if (r2 <= r1) {
      error("Max port number should be bigger than min port number");
      return NULL;
    }

    uint16_t *portlist = malloc(sizeof(int));
    size_t    indx     = 0;

    for (; r1 < r2; r1++) {
      portlist[indx++] = r1;
      portlist         = realloc(portlist, sizeof(int) * (indx + 1));
    }

    portlist[indx] = 0;
    return portlist;
  }

  if (contains(ports, ',')) {
    char *cur = strtok(ports, ",");
    if (cur == NULL) {
      error("Bad port list");
      return NULL;
    }

    uint16_t *portlist = malloc(sizeof(int));
    int       index    = 0;

    do {
      int curi = atoi(cur);
      if (curi <= 0 || curi > UINT16_MAX) {
        error("Bad port number: %s", cur);
        return NULL;
      }

      portlist[index++] = curi;
      portlist          = realloc(portlist, sizeof(int) * (index + 1));
      cur               = strtok(NULL, ",");
    } while (cur != NULL);

    portlist[index] = 0;
    return portlist;
  }

  error("Bad port option");
  return NULL;
}

void clean_ports(uint16_t *ports) {
  if (ports == common_ports)
    return;
  free(ports);
}

void uint32_to_ipstr(char *ipstr, uint32_t ip) {
  snprintf(ipstr, INET_ADDRSTRLEN, "%d.%d.%d.%d", (ip >> 24) & 255, (ip >> 16) & 255, (ip >> 8) & 255, (ip & 255));
}
