#ifndef DNS_MODULE_H
#define DNS_MODULE_H

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define BUFFER_SIZE 512
#define DOMAIN_NAME_SIZE 256

// DNS Record Types
#define DNS_TYPE_A 1
#define DNS_TYPE_NS 2
#define DNS_TYPE_CNAME 5
#define DNS_TYPE_SOA 6
#define DNS_TYPE_PTR 12
#define DNS_TYPE_MX 15
#define DNS_TYPE_TXT 16
#define DNS_TYPE_AAAA 28
#define DNS_TYPE_SRV 33
#define DNS_TYPE_NAPTR 35
#define DNS_TYPE_CAA 257

// DNS Record Classes
#define DNS_CLASS_IN 1
#define DNS_CLASS_CS 2
#define DNS_CLASS_CH 3
#define DNS_CLASS_HS 4

// DNS Flags
#define DNS_FLAG_QR 0x8000 // Query/Response flag
#define DNS_FLAG_RD 0x0100 // Recursion Desired flag
#define DNS_FLAG_RA 0x0080 // Recursion Available flag

typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} dns_header_t;

typedef struct {
    uint8_t* data;
    size_t length;
} dns_packet_t;

typedef struct {
    dns_packet_t* packets;
    size_t count;
} dns_packet_list_t;

dns_packet_t create_dns_request(const char* domain, uint16_t qtype);
dns_packet_t create_dns_response(uint16_t id, uint16_t flags, uint16_t qdcount, uint16_t ancount, uint16_t nscount, uint16_t arcount, const uint8_t* query, size_t query_len);
dns_packet_list_t read_dns_requests_from_file();

#endif // DNS_MODULE_H
