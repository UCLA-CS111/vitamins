CC = gcc
CFLAGS = -std=c17 -O0 -ggdb3 -pthread
LDFLAGS = -pthread

EXECUTABLES = web_server_plain web_server_processes web_server_threads

.PHONY: all clean

all: $(EXECUTABLES)

web_server_%: web_server_%.o libhttp.o
	$(CC) $(LDFLAGS) -o $@ $^

web_server_plain.o: CFLAGS += -DWEB_SERVER_PLAIN
web_server_processes.o: CFLAGS += -DWEB_SERVER_PROCESSES
web_server_threads.o: CFLAGS += -DWEB_SERVER_THREADS

web_server_%.o: web_server.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXECUTABLES)
