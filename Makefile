CC := g++
#CFLAGS := -Wall -g
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%,$(SRCS))

all: $(OBJS) 

$(OBJS):
	$(CC) $@.cpp -o $@

clean:
	rm -rf $(OBJS) *.o
	
.PHONY: all clean
