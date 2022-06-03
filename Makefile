CC = gcc

CFLAGS += -std=gnu11
CFLAGS += -Wall -Werror

parser: parser.c
	${CC} ${CFLAGS} -o parser parser.c
