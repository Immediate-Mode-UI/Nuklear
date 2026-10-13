CFLAGS += -std=c89 -Wall -Wextra -pedantic
LDLIBS += -lm

bin/repro: main.c ../../nuklear.h
	mkdir -p bin
	$(CC) $(CPPFLAGS) $(CFLAGS) main.c $(LDFLAGS) $(LDLIBS) -o $@

.PHONY: test
test: bin/repro
	./bin/repro
