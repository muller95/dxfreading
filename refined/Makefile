CC = gcc
CCFLAGS = -Wall -O2 -fomit-frame-pointer
CCFLAGS_DEBUG = -Wall -g -p

SRCS = test.c dxf_file.c entities_structs.c
OBJS = $(SRCS:.c=.o)

test: $(OBJS)
	$(CC) $(CCFLAGS_DEBUG) -o $@ $^

%.o: %.c
	$(CC) $(CCFLAGS_DEBUG) -c -o $@ $<

clean:
	rm -rf $(OBJS) test gmon.out
