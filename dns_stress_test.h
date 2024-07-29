#ifndef DNS_STRESS_TEST_H
#define DNS_STRESS_TEST_H

#include <sys/time.h>
#include <netinet/in.h>
#include "dns_module.h"

typedef struct {
    int sockfd;
    struct sockaddr_in server_addr;
    dns_packet_list_t *packet_list;
    int qps;
    int duration;
    int threads;
    size_t pkt_sent;
    size_t pkt_recv;
    size_t pkt_nsent;
    struct timeval run_time;
    double estimated_qps;
    int thread_id;
} thread_data_t;

void start_stress_test(const char *server_ip, int port, int qps, int duration, int threads);
void print_stats(thread_data_t *data, int num_threads);

#endif // DNS_STRESS_TEST_H
