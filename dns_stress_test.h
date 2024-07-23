#ifndef DNS_STRESS_TEST_H
#define DNS_STRESS_TEST_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "dns_module.h"

typedef struct {
    int sockfd;
    struct sockaddr_in server_addr;
    dns_packet_list_t* packet_list;
    int qps;
    int duration;
    int threads;
} thread_data_t;

void start_stress_test(const char* server_ip, int port, int qps, int duration, int threads);
void* stress_test_thread(void* arg);

#endif // DNS_STRESS_TEST_H

