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

#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "inc/api.h"
#include "inc/log.h"
#include "inc/net.h"
#include "inc/op.h"
#include "inc/util.h"

recv_args_t *args = NULL;
int ports_size, recvport, *ports;

void handle_int() {
  info("Got interrupt, give me a second for cleanup");

  if (NULL != args) {
    pthread_mutex_lock(&net_lock);
    args->should_run = false;
    pthread_mutex_unlock(&net_lock);
  }

  exit(1);
}

bool scan(char *ipstr) {
  for (int i = 0; i < ports_size; i++) {
    int cur = ports[i];

    if (!net_syn(ipstr, cur, recvport)) {
      error("Failed to send SYN to %s:%d", ipstr, ports[i]);
      return false;
    }
  }

  return true;
}

int main(int argc, char **argv) {
  // parse options and log them out
  for (int i = 1; i < argc; i++) {
    if (!parse_opt(argv[i])) {
      return EXIT_FAILURE;
    }
  }

  info("massacr scanner %s: https://github.com/ngn13/massacr", VERSION);
  info("Running with following options:");
  print_opts();

  // check if running as root
  if (0 != getuid()) {
    error("Cannot create raw sockets without root");
    return EXIT_FAILURE;
  }

  // init threading and other stuff
  signal(SIGINT, handle_int);
  if (!api_check())
    return EXIT_FAILURE;

  ports = parse_ports(get_str("ports"), &ports_size);
  if (ports == NULL)
    return EXIT_FAILURE;
  if (ports_size > 1)
    info("Scanning for %d ports", ports_size);
  else
    info("Scanning for 1 port", ports_size);
  recvport = get_int("recvport");

  // converts string non public IP ranges to Range struct
  struct Range *ranges = NULL;
  for (int i = 0; i < bad_range_size; i++) {
    if (ranges == NULL)
      ranges = malloc(sizeof(struct Range));
    else
      ranges = realloc(ranges, sizeof(struct Range) * (i + 1));

    if (!get_range(&ranges[i], bad_ranges[i])) {
      error("Invalid range: %s", bad_ranges[i]);
      goto FAIL;
    }
  }

  // start receiver thread
  pthread_t recv;
  args = malloc(sizeof(recv_args_t));
  args->should_run = true;
  args->port = recvport;

  pthread_mutex_init(&net_lock, NULL);
  pthread_create(&recv, NULL, (void *)net_receive, args);
  pthread_detach(recv);

  // calculating limits
  float limit = 1.0 / get_int("limit");
  limit = (limit) * 1000000;

  // loops over all the IPs
  uint8_t cur[4] = {1, 1, 1, 0};
  while (cur[0] <= 255) {
    while (cur[1] <= 255) {
      while (cur[2] <= 255) {
        while (cur[3] <= 255) {
          if (skip_range(ranges, cur))
            goto NEXT;

          char ipstr[16];
          sprintf(ipstr, "%d.%d.%d.%d", cur[0], cur[1], cur[2], cur[3]);

          scan(ipstr);
          usleep(limit);
        NEXT:
          if (cur[3] == 255) {
            cur[3] = 0;
            break;
          }
          cur[3]++;
        }
        if (cur[2] == 255) {
          cur[2] = 0;
          break;
        }
        cur[2]++;
      }
      info("Scan completed for the %d.%d.0.0/16 range", cur[0], cur[1]);
      if (cur[1] == 255) {
        cur[1] = 0;
        break;
      }
      cur[1]++;
    }
    if (cur[0] == 255)
      break;
    cur[0]++;
  }

  info("Scan completed!");
  info("Now waiting for receiver timeout (%ds)", get_int("timeout"));
  sleep(get_int("timeout"));
  pthread_mutex_lock(&net_lock);
  args->should_run = false;
  pthread_mutex_unlock(&net_lock);
  pthread_mutex_destroy(&net_lock);

  free(ranges);
  clean_ports(ports);
  info("Scan completed");
  return EXIT_SUCCESS;

FAIL:
  pthread_mutex_lock(&net_lock);
  args->should_run = false;
  pthread_mutex_unlock(&net_lock);
  pthread_mutex_destroy(&net_lock);
  free(ranges);
  clean_ports(ports);
  return EXIT_FAILURE;
}
