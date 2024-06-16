#pragma once
#include <netinet/tcp.h>
#include <stdbool.h>
#include <stdint.h>

struct tcphdr_ {
  __extension__ union {
    struct {
      uint16_t th_sport; /* source port */
      uint16_t th_dport; /* destination port */
      tcp_seq  th_seq;   /* sequence number */
      tcp_seq  th_ack;   /* acknowledgement number */
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint8_t th_x2 : 4;  /* (unused) */
      uint8_t th_off : 4; /* data offset */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
      uint8_t th_off : 4; /* data offset */
      uint8_t th_x2 : 4;  /* (unused) */
#endif
      uint8_t th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
      uint16_t th_win; /* window */
      uint16_t th_sum; /* checksum */
      uint16_t th_urp; /* urgent pointer */
    };
    struct {
      uint16_t source;
      uint16_t dest;
      uint32_t seq;
      uint32_t ack_seq;
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint16_t res1 : 4;
      uint16_t doff : 4;
      uint16_t fin : 1;
      uint16_t syn : 1;
      uint16_t rst : 1;
      uint16_t psh : 1;
      uint16_t ack : 1;
      uint16_t urg : 1;
      uint16_t res2 : 2;
#elif __BYTE_ORDER == __BIG_ENDIAN
      uint16_t doff : 4;
      uint16_t res1 : 4;
      uint16_t res2 : 2;
      uint16_t urg : 1;
      uint16_t ack : 1;
      uint16_t psh : 1;
      uint16_t rst : 1;
      uint16_t syn : 1;
      uint16_t fin : 1;
#else
#error "Adjust your <bits/endian.h> defines"
#endif
      uint16_t window;
      uint16_t check;
      uint16_t urg_ptr;
    };
  };
};

typedef struct net_recv_args {
  uint16_t port;
  bool     should_run;
  int      threads;
} recv_args_t;

extern pthread_mutex_t net_lock;

bool net_init();
void net_free();
bool net_syn(uint32_t, int, int);
void net_receive(void *);
