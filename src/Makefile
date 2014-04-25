.PHONY: clean

CC=gcc
CFLAGS= -O3 -g -Wall -Wextra \
		-Wno-char-subscripts -Wno-unused-value -Wno-unused-parameter \
		-Wno-unused-variable -Wno-comment -Wno-unknown-pragmas
LIBS=
SOURCES=$(wildcard *.c)
HEADERS=$(SOURCES:%.c=%.h)
OBJECTS=$(SOURCES:%.c=%.o)

UNAME=$(shell uname)

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

3dlogic: clean $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS) $(LIBS)
	
clean:
ifeq ($(UNAME), windows32)
	rm -f $(OBJECTS) 3dlogic.exe
else
	rm -f $(OBJECTS) 3dlogic
endif

TESTS= 117 119 121 123 125 127 129 210 215 225 227 \
	118 120 122 124 126 130 212 224 226

HARDS=	128 228 222 223 229

test:	gcc
	@list='$(TESTS)'; for p in $$list; do \
		echo "Test $$p" ; \
		./3dlogic testes/$$p.in > difftest  ; \
		diff testes/$$p.out difftest ; \
	done

