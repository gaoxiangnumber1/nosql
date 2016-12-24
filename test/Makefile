INCLUDE = ../src
CC = gcc
CFLAGS =	-std=c99 -Wall -Wconversion -Werror -Wextra -Winline \
					-Wno-unused-parameter -Wpointer-arith -Wunused-function \
					-Wunused-value -Wunused-variable -Wwrite-strings \
					-I$(INCLUDE)
LDFLAGS = -lpthread

.SUFFIXES: .c .o

SOURCE =	$(INCLUDE)/memory.c \
					$(INCLUDE)/simple_dynamic_string.c simple_dynamic_string_test.c \
					$(INCLUDE)/double_linked_list.c double_linked_list_test.c \
					$(INCLUDE)/dictionary.c dictionary_test.c
OBJECT = $(SOURCE:.c=.o)
SDS_TEST = simple_dynamic_string_test
SDS_OBJ =	simple_dynamic_string_test.o $(INCLUDE)/simple_dynamic_string.o \
					$(INCLUDE)/memory.o
LIST_TEST = double_linked_list
LIST_OBJ = double_linked_list_test.o $(INCLUDE)/double_linked_list.o $(INCLUDE)/memory.o
DICT_TEST = dictionary_test
DICT_OBJ = dictionary_test.o $(INCLUDE)/dictionary.o $(INCLUDE)/memory.o
TEST = $(SDS_TEST) $(LIST_TEST) $(DICT_TEST)

all: $(OBJECT) $(TEST)

$(SDS_TEST): $(SDS_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(LIST_TEST): $(LIST_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(DICT_TEST): $(DICT_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(TEST) *~

.PHONY: all clean
