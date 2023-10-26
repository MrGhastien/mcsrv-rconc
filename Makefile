CC := gcc
CFLAGS := -Wall -Wextra -I./include
LDFLAGS :=

SRCS := $(wildcard src/*.c)
HDRS := $(wildcard include/*.h)
OBJS := $(patsubst src/%.c,obj/%.o,$(SRCS))

DIRS = obj/

TARGET = srv-handler

.PHONY: all clean

all: CFLAGS += -g -fsanitize=address -fsanitize=undefined
all: LDFLAGS += -g -fsanitize=address -fsanitize=undefined
all: $(TARGET)

release: $(TARGET)

release: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

obj/%.o: src/%.c $(HDRS) | obj/
	$(CC) $(CFLAGS) -c -o $@ $<

$(DIRS):
	mkdir $@

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
