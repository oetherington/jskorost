EXAMPLE_SRC = example.c
EXAMPLE_TARGET = example

TEST_SRC = test.c
TEST_TARGET = test

PROFILER_SRC = profiler.c
PROFILER_TARGET = profiler

CC = clang
CFLAGS = -std=c99 -W -Wall -Wextra -pedantic
TEST_FLAGS = -lcmocka

.PHONY: all example test $(EXAMPLE_TARGET) $(TEST_TARGET) $(PROFILER_TARGET) run clean

all: test

example: CFLAGS += -O1 -g3 -fno-omit-frame-pointer
example:
	$(CC) $(CFLAGS) $(EXAMPLE_SRC) -o $(EXAMPLE_TARGET)

test: CFLAGS += -O1 -g3 -fno-omit-frame-pointer
test:
	$(CC) $(CFLAGS) $(TEST_FLAGS) $(TEST_SRC) -o $(TEST_TARGET)

profiler: CFLAGS += -Ofast -g0 -s -fomit-frame-pointer
profiler:
	$(CC) $(CFLAGS) $(PROFILER_FLAGS) $(PROFILER_SRC) -o $(PROFILER_TARGET)

run:
	./${TEST_TARGET}

clean:
	rm -f $(EXAMPLE_TARGET) $(TEST_TARGET) $(PROFILER_TARGET)
