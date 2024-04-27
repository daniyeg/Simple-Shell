CC = gcc
CFLAGS = -Wall -g

SRCS = builtin.c execute.c input.c main.c
OBJS = $(SRCS:.c=.o)
MAIN = shell

all: $(MAIN)
	@echo Build completed successfully.

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

run: $(MAIN)
	./$(MAIN)

clean:
	$(RM) *.o $(MAIN)
