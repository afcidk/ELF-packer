CC = gcc
CFLAGS=-O0 -g -Wall -Werror -no-pie
OBJS = main.o \
	   getter.o \
	   utils.o \
	   unpack.o 

%.c:%.h
	$(CC) $(CFLAGS) -c $@

all: $(OBJS)
	$(CC) $(CFLAGS) -o packer $^

%.o: %.asm
	nasm -f elf64 -o $@ $^

clean:
	$(RM) $(OBJS)
	$(RM) packer
	$(RM) peda*
	$(RM) *.old 

.PHONY: clean
