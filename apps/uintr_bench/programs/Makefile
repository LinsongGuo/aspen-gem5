SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(OBJS) array.o

%.o: %.c
# $(CC) -g -O3 -muintr -c $< -o $@
	$(CC) -g -muintr -c $< -o $@
	
%.o: %.cc
	$(CXX) -g -O3 -muintr -c $< -o $@

clean:
	rm $(OBJS) array.o

