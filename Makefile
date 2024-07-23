# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -pthread

# Исходные файлы
DNS_MODULE_SRC = dns_module.c
DNS_SERVER_SRC = dns_server.c
DNS_STRESS_TEST_SRC = dns_stress_test.c
MAIN_SRC = main.c
MAIN_SERVER_SRC = main_server.c

# Объектные файлы
DNS_MODULE_OBJ = dns_module.o
DNS_SERVER_OBJ = dns_server.o
DNS_STRESS_TEST_OBJ = dns_stress_test.o
MAIN_OBJ = main.o
MAIN_SERVER_OBJ = main_server.o

# Исполняемые файлы
DNS_SERVER_BIN = dns_server
DNS_STRESS_TEST_BIN = dns_stress_test

# Правило по умолчанию
all: $(DNS_SERVER_BIN) $(DNS_STRESS_TEST_BIN)

# Компиляция объектных файлов
$(DNS_MODULE_OBJ): $(DNS_MODULE_SRC)
	$(CC) $(CFLAGS) -c $(DNS_MODULE_SRC)

$(DNS_SERVER_OBJ): $(DNS_SERVER_SRC)
	$(CC) $(CFLAGS) -c $(DNS_SERVER_SRC)

$(DNS_STRESS_TEST_OBJ): $(DNS_STRESS_TEST_SRC)
	$(CC) $(CFLAGS) -c $(DNS_STRESS_TEST_SRC)

$(MAIN_OBJ): $(MAIN_SRC)
	$(CC) $(CFLAGS) -c $(MAIN_SRC)

$(MAIN_SERVER_OBJ): $(MAIN_SERVER_SRC)
	$(CC) $(CFLAGS) -c $(MAIN_SERVER_SRC)

# Сборка исполняемых файлов
$(DNS_SERVER_BIN): $(DNS_MODULE_OBJ) $(DNS_SERVER_OBJ) $(MAIN_SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(DNS_SERVER_BIN) $(DNS_MODULE_OBJ) $(DNS_SERVER_OBJ) $(MAIN_SERVER_OBJ)

$(DNS_STRESS_TEST_BIN): $(DNS_MODULE_OBJ) $(DNS_STRESS_TEST_OBJ) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $(DNS_STRESS_TEST_BIN) $(DNS_MODULE_OBJ) $(DNS_STRESS_TEST_OBJ) $(MAIN_OBJ)

# Очистка
clean:
	rm -f *.o $(DNS_SERVER_BIN) $(DNS_STRESS_TEST_BIN)
