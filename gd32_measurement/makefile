CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -DPLATFORM_HOST -I src -I test/unity
LDFLAGS = -lm

SRC = src/config.c src/calc.c
TEST_SRC = test/test_main.c test/test_calc.c test/unity/unity.c
OBJ_DIR = build

TARGET_CONFIG = $(OBJ_DIR)/test_config.exe
TARGET_CALC   = $(OBJ_DIR)/test_calc.exe

all: $(TARGET_CONFIG) $(TARGET_CALC)

$(TARGET_CONFIG): $(OBJ_DIR)/config.o $(OBJ_DIR)/test_main.o | $(OBJ_DIR)
	$(CC) $^ -o $@

$(TARGET_CALC): $(OBJ_DIR)/calc.o $(OBJ_DIR)/config.o $(OBJ_DIR)/test_calc.o $(OBJ_DIR)/unity.o | $(OBJ_DIR)
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_main.o: test/test_main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_calc.o: test/test_calc.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/unity.o: test/unity/unity.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)