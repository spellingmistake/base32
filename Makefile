TARGETS := libbaseXX.so base_test

CC := gcc
CFLAGS := -fPIC -O0 -ggdb3
LDLIBS :=

all: $(TARGETS)

libbaseXX.so: CFLAGS += -shared
base_test: LDLIBS := -lbaseXX
base_test: LDFLAGS := -L.

libbaseXX.so: base32.o baseXX.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	@rm -f $(TARGETS) *.o

.PHONY: all clean
