#include "dns_stress_test.h"

int main(int argc, char* argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <qps> <duration> <threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* server_ip = argv[1];
    int port = atoi(argv[2]);
    int qps = atoi(argv[3]);
    int duration = atoi(argv[4]);
    int threads = atoi(argv[5]);

    start_stress_test(server_ip, port, qps, duration, threads);

    return 0;
}
