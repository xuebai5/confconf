.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -O2
CFLAGSDEBUG = -std=c99 -Wall -pedantic -ggdb3 -O0 -DDEBUG
PREFIX = /usr/local
MANPREFIX = /usr/local/share/man

all: confconf

debug: dbg_confconf

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f confconf $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/confconf
	cp -f doc/man/confconf.1 $(DESTDIR)$(MANPREFIX)/man1/confconf.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/confconf.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/confconf
	rm -f $(DESTDIR)$(MANPREFIX)/man1/confconf.1

confconf: build/release build/release/opt.o build/release/tok.o build/release/main.o build/release/analyse.o build/release/parse.o build/release/gen.o
	$(CC) $(LDFLAGS) -o confconf build/release/opt.o build/release/tok.o build/release/main.o build/release/analyse.o build/release/parse.o build/release/gen.o $(LDLIBS)

dbg_confconf: build/debug build/debug/opt.o build/debug/tok.o build/debug/main.o build/debug/analyse.o build/debug/parse.o build/debug/gen.o
	$(CC) $(LDFLAGS) -o dbg_confconf build/debug/opt.o build/debug/tok.o build/debug/main.o build/debug/analyse.o build/debug/parse.o build/debug/gen.o $(LDLIBS)

build/release:
	mkdir -p build/release

build/debug:
	mkdir -p build/debug

src/version.h:
	printf "%s\n%s\n\n%s%s%s\n\n%s\n" \
		"#ifndef CONFCONF_VERSION_H" \
		"#define CONFCONF_VERSION_H" \
		"#define VERSION \"confconf-" "`git describe --always --tags`" "\"" \
		"#endif" > src/version.h

build/release/opt.o: src/opt.c src/version.h src/opt.h \
 src/../reqs/simple-opt/simple-opt.h
	$(CC) -c $(CFLAGS) -o build/release/opt.o src/opt.c
build/release/tok.o: src/tok.c src/tok.h
	$(CC) -c $(CFLAGS) -o build/release/tok.o src/tok.c
build/release/main.o: src/main.c src/err.h src/opt.h src/parse.h \
 src/tok.h src/analyse.h src/gen.h
	$(CC) -c $(CFLAGS) -o build/release/main.o src/main.c
build/release/analyse.o: src/analyse.c src/err.h src/analyse.h src/tok.h \
 src/parse.h
	$(CC) -c $(CFLAGS) -o build/release/analyse.o src/analyse.c
build/release/parse.o: src/parse.c src/parse.h src/tok.h src/err.h
	$(CC) -c $(CFLAGS) -o build/release/parse.o src/parse.c
build/release/gen.o: src/gen.c src/gen.h src/parse.h src/tok.h \
 src/analyse.h src/version.h src/gen-consts.h
	$(CC) -c $(CFLAGS) -o build/release/gen.o src/gen.c

build/debug/opt.o: src/opt.c src/version.h src/opt.h \
 src/../reqs/simple-opt/simple-opt.h
	$(CC) -c $(CFLAGSDEBUG) -o build/debug/opt.o src/opt.c
build/debug/tok.o: src/tok.c src/tok.h
	$(CC) -c $(CFLAGSDEBUG) -o build/debug/tok.o src/tok.c
build/debug/main.o: src/main.c src/err.h src/opt.h src/parse.h src/tok.h \
 src/analyse.h src/gen.h
	$(CC) -c $(CFLAGSDEBUG) -o build/debug/main.o src/main.c
build/debug/analyse.o: src/analyse.c src/err.h src/analyse.h src/tok.h \
 src/parse.h
	$(CC) -c $(CFLAGSDEBUG) -o build/debug/analyse.o src/analyse.c
build/debug/parse.o: src/parse.c src/parse.h src/tok.h src/err.h
	$(CC) -c $(CFLAGSDEBUG) -o build/debug/parse.o src/parse.c
build/debug/gen.o: src/gen.c src/gen.h src/parse.h src/tok.h \
 src/analyse.h src/version.h src/gen-consts.h
	$(CC) -c $(CFLAGSDEBUG) -o build/debug/gen.o src/gen.c

clean:
	rm -f src/version.h
	rm -f confconf
	rm -f dbg_confconf
	rm -rf build

.PHONY: all debug install uninstall confconf dbg_confconf clean
