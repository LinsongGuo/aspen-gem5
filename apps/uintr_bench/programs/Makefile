SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(OBJS) array.o

%.o: %.c
	$(CC) -g -O3 -muintr -mavx2 -c $< -o $@
	
%.o: %.cc
	$(CXX) -g -O3 -muintr -c $< -o $@

clean:
	rm $(OBJS) array.o

