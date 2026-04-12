CC = gcc
CFLAGS = -Wall -Wextra -pthread

SRC = src/file_ops.c src/sync.c src/logger_stub.c src/test_file_ops.c

all:
	$(CC) $(CFLAGS) $(SRC) -o test_file_ops

clean:
	rm -f test_file_ops