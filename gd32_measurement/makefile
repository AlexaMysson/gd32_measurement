CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -DPLATFORM_HOST -I src

SRC = src/config.c
TEST_SRC = test/test_main.c
OBJ_DIR = build

TARGET = $(OBJ_DIR)/test_config.exe

all: $(TARGET)

$(TARGET): $(OBJ_DIR)/config.o $(OBJ_DIR)/test_main.o | $(OBJ_DIR)
	$(CC) $^ -o $@

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_main.o: test/test_main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)