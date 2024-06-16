#include <libnet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../inc/db.h"
#include "../inc/log.h"
#include "../inc/net.h"
#include "../inc/pool.h"

pthread_mutex_t net_lock;
libnet_t       *ctx = NULL;

bool net_init() {
  if (pthread_mutex_init(&net_lock, NULL) != 0) {
    error("Failed to init receiver thread lock");
    return false;
  }

  if ((ctx = libnet_init(LIBNET_RAW4, NULL, NULL)) == NULL) {
    error("Failed to init libnet");
    return false;
  }

  return true;
}

void net_free() {
  pthread_mutex_destroy(&net_lock);
  if (NULL != ctx)
    libnet_destroy(ctx);
}

void net_receive(void *ap) {
  // load thread args, create the database pool
  recv_args_t *args = (recv_args_t *)ap;
  pool_t      *pool = pool_init(args->threads);

  // create raw socket
  int raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (raw < 0) {
    error("Failed to create raw TCP socket (are you running as root?)");
    return;
  }

  // setup args used for "recvfrom"
  struct timeval     timeout;
  struct sockaddr_in addr;
  socklen_t          addrlen = sizeof(addr);

  // create recv buffer
  size_t bufsize = sizeof(struct tcphdr_) + sizeof(struct iphdr);
  char   buff[bufsize];
  bzero(buff, bufsize);

  // logging
  pthread_mutex_lock(&net_lock);
  info("Started receiver on port %d", args->port);
  pthread_mutex_unlock(&net_lock);

  // start receiver loop
  while (args->should_run) {
    // recvfrom the raw socket
    if (recvfrom(raw, buff, bufsize, 0, (struct sockaddr *)&addr, &addrlen) <= 0)
      continue;

    // make sure this is connection is received over IPv4
    if (addr.sin_family != AF_INET)
      continue;

    // parse the received data, make sure its SYN+ACK
    struct tcphdr_ *tcph = (struct tcphdr_ *)(buff + sizeof(struct iphdr));
    if (tcph->ack != 1 || tcph->syn != 1 || tcph->rst == 1)
      continue;

    // make sure we received from the correct port
    if (htons(tcph->dest) != args->port)
      continue;

    // send the info to the database
    db_args_t *data = malloc(sizeof(db_args_t));
    data->port      = htons(tcph->source);
    data->ipv4      = addr.sin_addr.s_addr;
    pool_add(pool, (void *)db_add, data);
  }

  // logging & stop the pool
  debug("Closing database pool");
  pool_stop(pool);

  // close the socket
  debug("Closing receiver socket");
  close(raw);
}

bool net_syn(uint32_t ip, int port, int recvport) {
  bool          ret = false;
  libnet_ptag_t r =
      libnet_build_tcp(recvport, port, 0x01010101, 0x02020202, TH_SYN, 32767, 0, 10, LIBNET_TCP_H, NULL, 0, ctx, 0);

  if (r < 0) {
    error("Failed to build TCP packet: %s", libnet_geterror(ctx));
    goto end;
  }

  r = libnet_build_ipv4(
      LIBNET_IPV4_H + LIBNET_TCP_H, 0, 242, 0, 64, IPPROTO_TCP, 0, libnet_get_ipaddr4(ctx), ip, NULL, 0, ctx, 0);

  if (r < 0) {
    error("Failed to build IPv4 header: %s", libnet_geterror(ctx));
    goto end;
  }

  if (libnet_write(ctx) < 0) {
    error("Failed to send the packet: %s", libnet_geterror(ctx));
    goto end;
  }

  ret = true;

end:
  libnet_clear_packet(ctx);
  return ret;
}
