#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dns_module.h"

#define PORT 5354 // Порт для DNS сервера

void handle_dns_request(int sockfd, struct sockaddr_in* client_addr, socklen_t client_len, uint8_t* buffer, ssize_t len);
void start_dns_server();

#endif // DNS_SERVER_H
