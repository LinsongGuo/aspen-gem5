# # Include standard template for this suite
# -include ../../../Makefile.defaults

# nomakefiles:
# 	@echo "*** Whoops!  I didn't find a Makefile.defaults file."
# 	@echo "*** Your distribution is probably corrupt."
# 	@echo

CFLAGS += -g # -O3 -mno-avx -mno-sse

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

all: $(OBJS) 

%.o: %.c 
	$(CC) $(CFLAGS) -muintr -I../.. -c $< -o $@

clean:
	rm -rf $(OBJS)