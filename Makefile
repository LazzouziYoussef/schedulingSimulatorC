CC      = gcc
CFLAGS  = -Wall -I.
SRC     = main.c sched.c metrics.c print.c
OBJ     = $(SRC:.c=.o)
TARGET  = sched

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

re: clean all
