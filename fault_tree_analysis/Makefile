CC := g++
#CFLAGS := -Wall -g
SRCS := $(wildcard *.cc)
OBJS := $(patsubst %.cc,%,$(SRCS))

all: $(OBJS)

$(OBJS):
	$(CC) $@.cc -o $@

clean:
	rm -rf $(OBJS) *.o
	
.PHONY: all clean
