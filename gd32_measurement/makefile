CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -DPLATFORM_HOST -I src -I test/unity
LDFLAGS = -lm

# Объектные файлы для основной программы
OBJ_MEASURE = build/config.o build/calc.o build/meas_sim.o build/main.o

# Объектные файлы для тестов
OBJ_TEST_CONFIG = build/config.o build/test_main.o
OBJ_TEST_CALC   = build/calc.o build/config.o build/test_calc.o build/unity.o

# Исполняемые файлы
TARGET_MEASURE = build/measure.exe
TARGET_CONFIG  = build/test_config.exe
TARGET_CALC    = build/test_calc.exe

all: $(TARGET_MEASURE) $(TARGET_CONFIG) $(TARGET_CALC)

# Основная программа
$(TARGET_MEASURE): $(OBJ_MEASURE) | build
	$(CC) $^ $(LDFLAGS) -o $@

# Тест конфигурации
$(TARGET_CONFIG): $(OBJ_TEST_CONFIG) | build
	$(CC) $^ -o $@

# Тест расчётов
$(TARGET_CALC): $(OBJ_TEST_CALC) | build
	$(CC) $^ $(LDFLAGS) -o $@

# Правила для компиляции объектных файлов из src/
build/config.o: src/config.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/calc.o: src/calc.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/meas_sim.o: src/meas_sim.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/main.o: src/main.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Правила для тестовых объектных файлов
build/test_main.o: test/test_main.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/test_calc.o: test/test_calc.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/unity.o: test/unity/unity.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Создание папки build
build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: all clean