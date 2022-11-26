TARGETS := base32

all: $(TARGETS)

CC := gcc
CFLAGS := -O0 -ggdb3
LDLIBS :=

clean:
	@rm -f $(TARGETS)

.PHONY: all clean
