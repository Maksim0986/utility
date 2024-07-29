CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
LDFLAGS = -pthread

# Объектные файлы
OBJS = dns_module.o dns_server.o main_server.o dns_stress_test.o main.o

all: dns_server dns_stress_test

# Правило для создания dns_server
dns_server: dns_module.o dns_server.o main_server.o
	$(CC) $^ -o $@ $(LDFLAGS)

# Правило для создания dns_stress_test
dns_stress_test: dns_module.o dns_stress_test.o main.o
	$(CC) $^ -o $@ $(LDFLAGS)

# Общие правила компиляции
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) dns_server dns_stress_test
