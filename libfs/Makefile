# Target library
CC := gcc
CFLAGS := -Wall -Werror -Wextra
LIB := libfs.a
OBJS := fs.o disk.o
all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test_fs.x: test_fs.o $(LIB)
	$(CC) -o $@ $^ -L. -lfs

clean:
	rm -f $(LIB) $(OBJS)
