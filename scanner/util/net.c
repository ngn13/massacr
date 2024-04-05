#include <libnet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../inc/api.h"
#include "../inc/log.h"
#include "../inc/net.h"
#include "../inc/pool.h"

pthread_mutex_t net_lock;

bool net_receive(void *ap) {
  recv_args_t *args = (recv_args_t *)ap;
  pool_t *pool = pool_init(10);

  int s = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (s < 0) {
    debug("Cannot create raw TCP socket");
    return false;
  }

  struct timeval timeout;
  struct sockaddr_in addr;
  uint addrlen = sizeof(addr);

  char buff[sizeof(struct tcphdr_) + sizeof(struct iphdr)] = {0};

  info("Started receiver on port %d", args->port);
  while (args->should_run) {
    int res =
        recvfrom(s, &buff, sizeof(buff), 0, (struct sockaddr *)&addr, &addrlen);

    if (res <= 0)
      continue;

    struct tcphdr_ *tcph = (struct tcphdr_ *)(buff + sizeof(struct iphdr));
    if (tcph->ack != 1 || tcph->syn != 1 || tcph->rst == 1)
      continue;

    if (htons(tcph->dest) != args->port)
      continue;

    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ipstr, INET_ADDRSTRLEN);

    pthread_t t;
    api_args_t *data = malloc(sizeof(api_args_t));
    data->port = htons(tcph->source);
    data->ip = ipstr;

    pool_add(pool, (void *)api_add, data);
    pthread_mutex_lock(&net_lock);
    debug("Received SYN+ACK from %s:%d", ipstr, htons(tcph->source));
    pthread_mutex_unlock(&net_lock);
  }

  pthread_mutex_lock(&net_lock);
  debug("Closing thread");
  pthread_mutex_unlock(&net_lock);

  pool_stop(pool);
  free(args);
  close(s);
  return true;
}

bool net_syn(char *ipstr, int port, int recvport) {
  char *payload = "massacr scanner IGNORE";
  int payload_s = strlen(payload);

  libnet_t *handle = libnet_init(LIBNET_RAW4, NULL, NULL);

  libnet_ptag_t ret = libnet_build_tcp(
      recvport, port, 0x01010101, 0x02020202, TH_SYN, 32767, 0, 10,
      LIBNET_TCP_H + payload_s, (uint8_t *)payload, payload_s, handle, 0);

  if (ret < 0)
    goto FAIL;

  ret = libnet_build_ipv4(LIBNET_IPV4_H + LIBNET_TCP_H + payload_s, 0, 242, 0,
                          64, IPPROTO_TCP, 0, libnet_get_ipaddr4(handle),
                          libnet_name2addr4(handle, ipstr, LIBNET_RESOLVE),
                          NULL, 0, handle, 0);

  if (ret < 0)
    goto FAIL;

  if (libnet_write(handle) < 0)
    goto FAIL;

  libnet_destroy(handle);
  return true;

FAIL:
  libnet_destroy(handle);
  return false;
}
