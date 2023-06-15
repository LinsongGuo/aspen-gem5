SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(OBJS)

%.o: %.c
	gcc -g -O2 -muintr -c $< -o $@
	# gcc -std=gnu11 -g -Wall -D_GNU_SOURCE -I../../../inc -I../../../bindings/cc -DNDEBUG -O2 -muintr -c $< -o $@

clean:
	rm $(OBJS)

