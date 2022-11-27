TARGETS = base_test

CFLAGS := -fPIC -O2 -Wall -Wextra

CC ?= gcc
LIB_SRCS := \
	baseXX.c \
	base32.c \
	base64.c
LIB_OBJS := $(LIB_SRCS:.c=.o)
LIB := libbaseXX.so

all: $(TARGETS)

base_test: LDLIBS += -lbaseXX
base_test: LDFLAGS += -L.
base_test: $(LIB)

$(LIB): LDFLAGS += -shared
$(LIB): $(LIB_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	@rm -f $(TARGETS) $(LIB) *.o

.PHONY: all clean
