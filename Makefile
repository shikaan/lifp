include flags.mk

ifeq ($(PROFILE),1)
	CFLAGS := $(CFLAGS) -DMEMORY_PROFILE
endif

.PHONY: all
all: artifacts/docs.h artifacts/lifp.1 bin/lifp

linenoise.o: CFLAGS = -Wall -W -Os
linenoise.o: vendor/linenoise/linenoise.c
	$(CC) $(CFLAGS) -c $< -o $@

args.o: CFLAGS = -Wall -W -Os
args.o: vendor/args/src/args.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/list.o: lib/arena.o

lifp/tokenize.o: lib/list.o lib/arena.o
lifp/parse.o: lifp/tokenize.o lib/list.o lib/arena.o lifp/node.o
lifp/node.o: lib/arena.o
lifp/value.o: lib/arena.o lifp/node.o
lifp/virtual_machine.o: lifp/value.o
lifp/evaluate.o: \
  lib/arena.o lifp/virtual_machine.o lifp/value.o lifp/specials.o

tests/tokenize.test: lifp/tokenize.o lib/list.o lib/arena.o
tests/parser.test: \
	lifp/parse.o lifp/tokenize.o lib/list.o lifp/node.o lib/arena.o
tests/list.test: lib/list.o lib/arena.o
tests/arena.test: lib/arena.o
tests/evaluate.test: \
	lifp/evaluate.o lifp/node.o lib/list.o lib/arena.o lifp/virtual_machine.o \
	lifp/value.o lifp/fmt.o lifp/specials.o
tests/specials.test: \
	lifp/specials.o lifp/evaluate.o lifp/node.o lib/list.o lib/arena.o \
	lifp/virtual_machine.o lifp/value.o lifp/fmt.o lifp/tokenize.o \
	lifp/parse.o
tests/fmt.test: lifp/fmt.o lifp/node.o lib/arena.o lib/list.o lifp/value.o \
	lifp/virtual_machine.o lifp/specials.o lifp/evaluate.o
tests/virtual_machine.test: lifp/virtual_machine.o lib/list.o \
	lib/arena.o lifp/fmt.o lifp/specials.o lifp/evaluate.o lifp/value.o \
	lifp/node.o


tests/integration.test: \
	lifp/tokenize.o lifp/parse.o lib/arena.o lifp/evaluate.o lib/list.o \
	lifp/node.o lifp/virtual_machine.o lifp/value.o lifp/fmt.o \
	lifp/specials.o

tests/memory.test: \
	lifp/tokenize.o lifp/parse.o lib/arena.o lifp/evaluate.o lib/list.o \
	lifp/node.o lifp/virtual_machine.o lifp/value.o lifp/fmt.o \
	lib/profile.o lifp/specials.o

bin/lifp: CFLAGS := $(CFLAGS) -DVERSION='"$(VERSION)"' -DSHA='"$(SHA)"'
bin/lifp: \
	lifp/tokenize.o lifp/parse.o lib/list.o lifp/evaluate.o lifp/node.o \
	lib/arena.o lifp/virtual_machine.o lib/profile.o lifp/fmt.o \
	lifp/value.o lifp/specials.o linenoise.o args.o

.PHONY: artifacts/docs.h
artifacts/docs.h:
	VERSION="$(VERSION)" SHA="$(SHA)" python3 scripts/docs.py repl

.PHONY: artifacts/lifp.1
artifacts/lifp.1:
	VERSION="$(VERSION)" SHA="$(SHA)" python3 scripts/docs.py man

.PHONY: docs/index.md
docs/index.md:
	VERSION="$(VERSION)" SHA="$(SHA)" python3 scripts/docs.py web

.PHONY: repl
repl: bin/lifp
	@bin/lifp repl

.PHONY: clean
clean:
	rm -rf *.o **/*.o **/*.dSYM main *.dSYM *.plist
	rm -f tests/*.test

.PHONY: lifp-test

lifp-test: \
	tests/tokenize.test tests/parser.test tests/evaluate.test \
	tests/integration.test tests/fmt.test tests/tokenize.test \
	tests/parser.test tests/evaluate.test tests/fmt.test \
	tests/virtual_machine.test tests/specials.test \
	tests/integration.test
	tests/tokenize.test
	tests/parser.test
	tests/evaluate.test
	tests/integration.test
	tests/fmt.test
	tests/tokenize.test
	tests/parser.test
	tests/evaluate.test
	tests/fmt.test
	tests/virtual_machine.test
	tests/specials.test
	tests/integration.test

.PHONY: lib-test
lib-test: tests/arena.test tests/list.test
	tests/arena.test
	tests/list.test

.PHONY: memory-test
memory-test:
	# Memory tests can only be run with the profiler on
	make PROFILE=1 clean tests/memory.test
	tests/memory.test
	make clean

.PHONY: test
test: lifp-test lib-test memory-test

.PHONY: docker-build
docker-build:
	docker build . --tag lifp

.PHONY: docker-run
docker-run:
	docker run -it --rm -v "$(PWD)":/mnt lifp bash
