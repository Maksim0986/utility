#include "dns_server.h"

void handle_dns_request(int sockfd, struct sockaddr_in* client_addr, socklen_t client_len, uint8_t* buffer, ssize_t len) {
    dns_header_t* header = (dns_header_t*)buffer;

    // Установить флаг QR (1 бит) в заголовке DNS
    header->flags |= htons(DNS_FLAG_QR);

    // Если установлен флаг RD (1 бит), установить флаг RA (1 бит)
    if (header->flags & htons(DNS_FLAG_RD)) {
        header->flags |= htons(DNS_FLAG_RA); // Установить флаг RA
    }

    // Создать DNS ответ
    dns_packet_t response = create_dns_response(
        ntohs(header->id),
        ntohs(header->flags),
        ntohs(header->qdcount),
        0, 0, 0,
        buffer + sizeof(dns_header_t),
        len - sizeof(dns_header_t)
    );

    // Отправить пакет обратно клиенту
    sendto(sockfd, response.data, response.length, 0, (struct sockaddr*)client_addr, client_len);

    free(response.data);
}

void start_dns_server() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    uint8_t buffer[BUFFER_SIZE];
    socklen_t client_len;
    ssize_t len;

    // Создание сокета
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Привязка сокета к адресу сервера
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("DNS server is running on port %d\n", PORT);

    // Основной цикл обработки запросов
    while (1) {
        client_len = sizeof(client_addr);
        len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if (len < 0) {
            perror("Failed to receive data");
            continue;
        }

        // Обработка DNS запроса
        handle_dns_request(sockfd, &client_addr, client_len, buffer, len);
    }

    close(sockfd);
}
