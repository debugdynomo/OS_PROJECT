# ──────────────────────────────────────────────────────────────
#  Makefile — Multithreaded File Management System
#  Owner: Member 1 (VISHNU TEJA VASAM)
#
#  Usage:
#    make          — build the project
#    make run      — build and run
#    make clean    — remove build artifacts
# ──────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -Wall -Wextra -pthread -g
LDFLAGS = -lpthread -lz

SRC_DIR = src
SRCS    = $(SRC_DIR)/main.c       \
          $(SRC_DIR)/sync.c       \
          $(SRC_DIR)/signals.c    \
          $(SRC_DIR)/logger.c     \
          $(SRC_DIR)/read_write.c \
          $(SRC_DIR)/delete_rename.c \
          $(SRC_DIR)/copy_metadata.c \
          $(SRC_DIR)/compression.c

TARGET  = file_manager

# ── Build ──────────────────────────────────────────────────────

all: dirs $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "  ✓ Build successful: ./$(TARGET)"
	@echo ""

# ── Create required directories ───────────────────────────────

dirs:
	@mkdir -p logs test_files

# ── Run ────────────────────────────────────────────────────────

run: all
	./$(TARGET)

# ── Clean ──────────────────────────────────────────────────────

clean:
	rm -f $(TARGET)
	rm -f logs/*.log
	@echo "  ✓ Cleaned."

.PHONY: all dirs run clean
CC = gcc
CFLAGS = -Wall -Wextra -pthread

SRC = src/file_ops.c src/sync.c src/logger_stub.c src/test_file_ops.c

all:
	$(CC) $(CFLAGS) $(SRC) -o test_file_ops

clean:
	rm -f test_file_ops
