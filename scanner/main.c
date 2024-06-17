/*

 *  massacr/scanner | mass internet/ipv4 scanner
 *  ============================================
 *  written by ngn (https://ngn.tf) (2024)

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "inc/db.h"
#include "inc/log.h"
#include "inc/net.h"
#include "inc/op.h"
#include "inc/util.h"

recv_args_t args;

// https://www.iana.org/assignments/iana-ipv4-special-registry/iana-ipv4-special-registry.xhtml
char *bad_nets[] = {
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
    "255.255.255.255/32",
    NULL,
};

void handle_int() {
  info("Got interrupt, cleaning up");

  pthread_mutex_lock(&net_lock);
  args.should_run = false;
  pthread_mutex_unlock(&net_lock);

  net_free();
  db_free();
  exit(1);
}

int main(int argc, char **argv) {
  int ret = EXIT_FAILURE;

  // parse options and log them out
  for (int i = 1; i < argc; i++) {
    if (!parse_opt(argv[i])) {
      return EXIT_FAILURE;
    }
  }

  info("massacr scanner (%s)", VERSION);
  info("Running with following options:");
  print_opts();

  // check if running as root
  if (0 != getuid()) {
    error("Cannot create raw sockets without root");
    return ret;
  }

  // check and parse params
  int int_limit = get_int("limit");
  if (int_limit <= 0) {
    error("Invalid limit: %d", int_limit);
    return ret;
  }

  float limit = 1.0 / int_limit;
  limit       = (limit) * 1000000;

  int recvport = get_int("recvport");
  if (recvport <= 0 || recvport > UINT16_MAX) {
    error("Invalid receiver port number: %d", recvport);
    return ret;
  }

  int threads = get_int("threads");
  if (threads <= 0) {
    error("Invalid database thread count: %d", threads);
    return ret;
  }

  char              *startpoint = get_str("startpoint");
  struct sockaddr_in startaddr;

  if (inet_pton(AF_INET, startpoint, &(startaddr.sin_addr)) != 1) {
    error("Invalid starting point for scanning: %s", strerror(errno));
    return ret;
  }

  uint16_t *ports = parse_ports(get_str("ports"));
  if (ports == NULL)
    return ret;

  // convert non public IP ranges to ipv4_t struct
  subnet_t *invalid      = NULL;
  size_t    invalid_size = 0;

  for (int i = 0; bad_nets[i] != NULL; i++)
    invalid_size++;

  subnet_t _invalid[invalid_size];
  invalid = _invalid;

  for (int i = 0; bad_nets[i] != NULL; i++) {
    if (get_subnet(&invalid[i], bad_nets[i]))
      continue;
    error("Invalid IPv4 subnet: %s", bad_nets[i]);
    goto fail;
  }

  // init stuff
  info("Initializing database connection");
  if (!db_init(get_str("mongo")))
    goto fail;

  info("Initializing networking");
  if (!net_init())
    goto fail;

  // start net receiver thread
  pthread_t recv;
  args.should_run = true;
  args.threads    = threads;
  args.port       = recvport;

  pthread_create(&recv, NULL, (void *)net_receive, &args);
  pthread_detach(recv);

  // setup signal handler
  signal(SIGINT, handle_int);

  // start looping over all the IPs
  uint32_t current = ntohl(startaddr.sin_addr.s_addr);

  do {
    // if the ip is in an invalid subnet, continue
    if (subnet_contains(current, invalid, invalid_size))
      continue;

    // send syn packets
    for (int i = 0; ports[i] != 0; i++)
      net_syn(current, ports[i], recvport);

    // sleep according to the limit
    usleep(limit);
  } while ((current++) != UINT32_MAX);

  // wait for receiver thread
  info("Scan completed!");
  info("Now waiting for receiver timeout (%ds)", get_int("timeout"));
  sleep(get_int("timeout"));

  // set the return value
  ret = EXIT_SUCCESS;
  info("Scan completed");

fail:
  // stop the receiver thread
  pthread_mutex_lock(&net_lock);
  args.should_run = false;
  pthread_mutex_unlock(&net_lock);

  // free the resources and return
  net_free();
  db_free();

  clean_ports(ports);
  return ret;
}
