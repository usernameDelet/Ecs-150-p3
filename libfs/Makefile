# Target library
CC := gcc
CFLAGS := -Wall -Werror -Wextra
LIB := libfs.a
OBJS := fs.o disk.o
all: $(lib)

$(LIB): $(OBJS)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(LIB) $(OBJS)
