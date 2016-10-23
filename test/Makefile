INCLUDE = /home/xiang/Gao/Codes/nosql/src

CC = gcc
CFLAGS = -std=c99 -g -I$(INCLUDE)
LDFLAGS = -lpthread

.SUFFIXES: .c .o

SOURCE = double_linked_list_test.c $(INCLUDE)/double_linked_list.c $(INCLUDE)/memory.c
OBJECT = $(SOURCE:.c=.o)
EXEC = double_linked_list_test

all: double_linked_list_test

$(EXEC): $(OBJECT)
	$(CC) -o $(EXEC) $(OBJECT) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(EXEC) *~

.PHONY: all clean
