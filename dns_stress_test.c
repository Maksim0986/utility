#include "dns_stress_test.h"

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    stop = 1;
}

void* stress_test_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int sockfd = data->sockfd;
    struct sockaddr_in server_addr = data->server_addr;
    dns_packet_list_t* packet_list = data->packet_list;
    int qps_per_thread = data->qps / data->threads;
    int interval_microseconds = 1000000 / qps_per_thread;

    size_t packet_index = 0;

    while (!stop) {
        dns_packet_t* packet = &packet_list->packets[packet_index];
        sendto(sockfd, packet->data, packet->length, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        packet_index = (packet_index + 1) % packet_list->count;
        usleep(interval_microseconds);
    }

    return NULL;
}

void start_stress_test(const char* server_ip, int port, int qps, int duration, int threads) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Создание сокета
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Загрузка DNS запросов из файла
    dns_packet_list_t packet_list = read_dns_requests_from_file();

    // Создание потоков для тестирования нагрузки
    pthread_t* thread_ids = malloc(threads * sizeof(pthread_t));
    thread_data_t data = {sockfd, server_addr, &packet_list, qps, duration, threads};

    signal(SIGINT, handle_sigint);

    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_ids[i], NULL, stress_test_thread, &data);
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

    // Освобождение памяти
    for (size_t i = 0; i < packet_list.count; i++) {
        free(packet_list.packets[i].data);
    }
    free(packet_list.packets);
    free(thread_ids);
    close(sockfd);
}
