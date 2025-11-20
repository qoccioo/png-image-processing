TARGET = cw

CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LIBS = -lpng -lm

SRCS = src/main.c src/image_utils.c
OBJS = main.o image_utils.o  

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: src/%.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o 
