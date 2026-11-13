# Host build for checking the harness on a PC.
#   make            build with -O2
#   make OPT=O0     build with another optimization level
#   make run        build and run
OPT    ?= O2
CC     ?= gcc
CFLAGS  = -std=c11 -$(OPT) -Wall -Wextra -Wpedantic -Wconversion -Wshadow -D_POSIX_C_SOURCE=199309L -DBENCH_OPT=\"$(OPT)\"
SRC     = main.c ../harness/bench_runner.c ../harness/workloads.c

bench_host: $(SRC) ../harness/*.h
	$(CC) $(CFLAGS) -o $@ $(SRC) -lm

run: bench_host
	./bench_host

clean:
	rm -f bench_host

.PHONY: run clean
