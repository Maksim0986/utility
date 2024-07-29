#include "dns_stress_test.h"
#include "dns_module.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <pthread.h>

#ifndef timersub
#define timersub(a, b, result)                     \
  do {                                             \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;  \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) {                   \
      --(result)->tv_sec;                          \
      (result)->tv_usec += 1000000;                \
    }                                              \
  } while (0)
#endif

#ifndef HAVE_MMSGHDR
struct mmsghdr {
    struct msghdr msg_hdr;  /* Message header */
    unsigned int  msg_len;  /* Number of bytes transmitted */
};
#endif

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    stop = 1;
}

void* stress_test_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    char filename[256];
    snprintf(filename, sizeof(filename), "thread_%d.log", data->thread_id);
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open log file");
        pthread_exit(NULL);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        fclose(file);
        pthread_exit(NULL);
    }
    struct sockaddr_in server_addr = data->server_addr;
    dns_packet_list_t* packet_list = data->packet_list;
    int qps_per_thread = (data->qps + data->threads - 1) / data->threads;
    int packets_per_send = 10;
    int interval_microseconds = 1000000 / (qps_per_thread / packets_per_send);

    size_t packet_index = 0;
    struct timeval now, deadline, timeout, start_time, end_time;
    size_t pkt_sent = 0;
    size_t pkt_recv = 0;
    size_t pkt_nsent = 0;

    gettimeofday(&start_time, NULL);
    gettimeofday(&now, NULL);
    deadline = now;
    deadline.tv_usec += interval_microseconds;

    while (!stop) {
        struct mmsghdr msgs[packets_per_send];
        struct iovec iovecs[packets_per_send];
        for (int i = 0; i < packets_per_send; i++) {
            if (packet_index >= packet_list->count) {
                packet_index = 0;
            }
            dns_packet_t* packet = &packet_list->packets[packet_index++];
            iovecs[i].iov_base = packet->data;
            iovecs[i].iov_len = packet->length;

            msgs[i].msg_hdr.msg_name = &server_addr;
            msgs[i].msg_hdr.msg_namelen = sizeof(server_addr);
            msgs[i].msg_hdr.msg_iov = &iovecs[i];
            msgs[i].msg_hdr.msg_iovlen = 1;
            msgs[i].msg_hdr.msg_control = NULL;
            msgs[i].msg_hdr.msg_controllen = 0;
            msgs[i].msg_hdr.msg_flags = 0;
            msgs[i].msg_len = 0;
        }

        int sent = sendmmsg(sockfd, msgs, packets_per_send, 0);
        if (sent < 0) {
            perror("Failed to send messages");
            pkt_nsent += packets_per_send;
        } else {
            pkt_sent += sent;
            pkt_nsent += packets_per_send - sent;
        }

        gettimeofday(&now, NULL);
        timersub(&deadline, &now, &timeout);
        if (timeout.tv_sec < 0 || (timeout.tv_sec == 0 && timeout.tv_usec <= 0)) {
            timeout.tv_sec = 0;
            timeout.tv_usec = interval_microseconds;
        }
        select(0, NULL, NULL, NULL, &timeout);

        gettimeofday(&now, NULL);
        deadline = now;
        deadline.tv_usec += interval_microseconds;
        if (deadline.tv_usec >= 1000000) {
            deadline.tv_sec += deadline.tv_usec / 1000000;
            deadline.tv_usec %= 1000000;
        }
    }

    gettimeofday(&end_time, NULL);
    timersub(&end_time, &start_time, &data->run_time);

    data->pkt_sent = pkt_sent;
    data->pkt_recv = pkt_recv;
    data->pkt_nsent = pkt_nsent;
    data->estimated_qps = pkt_sent / (data->run_time.tv_sec + data->run_time.tv_usec / 1000000.0);

    fprintf(file, "Thread %d: run_time=%.2f, target_qps=%d, estimated_qps=%.2f, pkt_sent=%zu, pkt_recv=%zu, pkt_nsent=%zu\n",
            data->thread_id, data->run_time.tv_sec + data->run_time.tv_usec / 1000000.0,
            data->qps / data->threads, data->estimated_qps, data->pkt_sent, data->pkt_recv, data->pkt_nsent);

    close(sockfd);
    fclose(file);
    return NULL;
}

void print_stats(thread_data_t* data, int num_threads) {
    size_t total_pkt_sent = 0;
    size_t total_pkt_recv = 0;
    size_t total_pkt_nsent = 0;
    double total_run_time = 0;

    for (int i = 0; i < num_threads; i++) {
        total_pkt_sent += data[i].pkt_sent;
        total_pkt_recv += data[i].pkt_recv;
        total_pkt_nsent += data[i].pkt_nsent;
        total_run_time += data[i].run_time.tv_sec + data[i].run_time.tv_usec / 1000000.0;
        printf("Thread %d: run_time=%.2f, target_qps=%d, estimated_qps=%.2f, pkt_sent=%zu, pkt_recv=%zu, pkt_nsent=%zu\n",
               i, data[i].run_time.tv_sec + data[i].run_time.tv_usec / 1000000.0,
               data[i].qps / data[i].threads, data[i].estimated_qps, data[i].pkt_sent, data[i].pkt_recv, data[i].pkt_nsent);
    }

    double estimated_qps = total_pkt_sent / total_run_time;
    double estimated_rps = total_pkt_recv / total_run_time;

    printf("\nAggregated Stats:\n");
    printf("Total run_time=%.2f, target_qps=%d, estimated_qps=%.2f, estimated_rps=%.2f, total_pkt_sent=%zu, total_pkt_recv=%zu, total_pkt_nsent=%zu\n",
           total_run_time, data->qps, estimated_qps, estimated_rps, total_pkt_sent, total_pkt_recv, total_pkt_nsent);
}

void start_stress_test(const char* server_ip, int port, int qps, int duration, int threads) {
    struct sockaddr_in server_addr;

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Загрузка DNS запросов из файла
    dns_packet_list_t packet_list = read_dns_requests_from_file();

    // Создание потоков для тестирования нагрузки
    pthread_t* thread_ids = malloc(threads * sizeof(pthread_t));
    thread_data_t* thread_data = malloc(threads * sizeof(thread_data_t));

    for (int i = 0; i < threads; i++) {
        thread_data[i].sockfd = -1;
        thread_data[i].server_addr = server_addr;
        thread_data[i].packet_list = &packet_list;
        thread_data[i].qps = qps;
        thread_data[i].duration = duration;
        thread_data[i].threads = threads;
        thread_data[i].pkt_sent = 0;
        thread_data[i].pkt_recv = 0;
        thread_data[i].pkt_nsent = 0;
        thread_data[i].run_time.tv_sec = 0;
        thread_data[i].run_time.tv_usec = 0;
        thread_data[i].thread_id = i;
    }

    signal(SIGINT, handle_sigint);

    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_ids[i], NULL, stress_test_thread, &thread_data[i]);
    }

    if (duration > 0) {
        sleep(duration);
        stop = 1;
    } else {
        while (!stop) {
            pause();
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    // Печать статистики
    print_stats(thread_data, threads);

    // Освобождение памяти
    for (size_t i = 0; i < packet_list.count; i++) {
        free(packet_list.packets[i].data);
    }
    free(packet_list.packets);
    free(thread_ids);
    free(thread_data);
}

    free(packet_list.packets);
    free(thread_ids);
    close(sockfd);
}
