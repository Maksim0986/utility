#include "dns_module.h"

uint16_t get_query_type(const char* type_str) {
    if (strcmp(type_str, "A") == 0) return DNS_TYPE_A;
    if (strcmp(type_str, "NS") == 0) return DNS_TYPE_NS;
    if (strcmp(type_str, "CNAME") == 0) return DNS_TYPE_CNAME;
    if (strcmp(type_str, "SOA") == 0) return DNS_TYPE_SOA;
    if (strcmp(type_str, "PTR") == 0) return DNS_TYPE_PTR;
    if (strcmp(type_str, "MX") == 0) return DNS_TYPE_MX;
    if (strcmp(type_str, "TXT") == 0) return DNS_TYPE_TXT;
    if (strcmp(type_str, "AAAA") == 0) return DNS_TYPE_AAAA;
    if (strcmp(type_str, "SRV") == 0) return DNS_TYPE_SRV;
    if (strcmp(type_str, "NAPTR") == 0) return DNS_TYPE_NAPTR;
    if (strcmp(type_str, "CAA") == 0) return DNS_TYPE_CAA;
    return DNS_TYPE_A; // default to A if type is unknown
}

dns_packet_t create_dns_request(const char* domain, uint16_t qtype) {
    dns_packet_t packet;
    packet.data = malloc(BUFFER_SIZE);
    packet.length = 0;

    dns_header_t* header = (dns_header_t*)packet.data;
    header->id = htons(1);
    header->flags = htons(0x0100); // standard query
    header->qdcount = htons(1);
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;

    packet.length += sizeof(dns_header_t);

    char* qname = (char*)(packet.data + packet.length);
    const char* pos = domain;
    while (*pos) {
        const char* start = pos;
        while (*pos && *pos != '.') {
            pos++;
        }
        int label_len = pos - start;
        *qname++ = label_len;
        memcpy(qname, start, label_len);
        qname += label_len;
        if (*pos == '.') {
            pos++;
        }
    }
    *qname++ = 0;

    packet.length += (qname - (char*)(packet.data + packet.length));

    uint16_t* qtype_ptr = (uint16_t*)qname;
    *qtype_ptr++ = htons(qtype);
    uint16_t* qclass = (uint16_t*)qtype_ptr;
    *qclass = htons(DNS_CLASS_IN);

    packet.length += 4;

    return packet;
}

dns_packet_t create_dns_response(uint16_t id, uint16_t flags, uint16_t qdcount, uint16_t ancount, uint16_t nscount, uint16_t arcount, const uint8_t* query, size_t query_len) {
    dns_packet_t packet;
    packet.data = malloc(BUFFER_SIZE);
    packet.length = 0;

    dns_header_t* header = (dns_header_t*)packet.data;
    header->id = htons(id);
    header->flags = htons(flags);
    header->qdcount = htons(qdcount);
    header->ancount = htons(ancount);
    header->nscount = htons(nscount);
    header->arcount = htons(arcount);

    packet.length += sizeof(dns_header_t);

    memcpy(packet.data + packet.length, query, query_len);
    packet.length += query_len;

    return packet;
}

dns_packet_list_t read_dns_requests_from_file() {
    const char* filename = "dns_requests.txt"; // hardcoded filename
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    printf("File %s successfully opened.\n", filename);

    dns_packet_list_t list;
    list.packets = malloc(10000 * sizeof(dns_packet_t));
    list.count = 0;

    char line[DOMAIN_NAME_SIZE + 10]; // extra space for type and spaces
    while (fgets(line, sizeof(line), file)) {
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        char type_str[10];
        char domain[DOMAIN_NAME_SIZE];
        sscanf(line, "%s %s", type_str, domain);

        uint16_t qtype = get_query_type(type_str);
        list.packets[list.count++] = create_dns_request(domain, qtype);
    }

    fclose(file);
    printf("Loaded %zu DNS requests.\n", list.count);
    return list;
}
