CC = gcc
CFLAGS = -DPLATFORM_HOST -I./src -I./test -Wall -Wextra -std=c99
LDFLAGS = -lm

SRC = src/main.c src/calc.c src/config.c src/uart_out.c src/meas_sim.c
OBJ = $(SRC:.c=.o)
TARGET = test_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)