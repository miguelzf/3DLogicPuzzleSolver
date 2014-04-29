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
EXEC=

3dlogic: clean $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(CFLAGS) $(LIBS)

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
ifeq ($(UNAME), windows32)
	rm -f $(OBJECTS) 3dlogic.exe
else
	rm -f $(OBJECTS) 3dlogic
endif

TESTS= 117 119 121 123 125 127 210 225 227 \
	118 120 122 124 126 130 212 224 226 \
	129 228-fully-walled

HARDS=	128 228 222 223

benchmark:
	time sh -c './$(EXEC) ../tests/129.in > /dev/null; \
				./$(EXEC) ../tests/228-fully-walled.in > /dev/null'

test: 3dlogic runtests

testhard: 3dlogic runhards

runtests: setvars
	@list='$(TESTS)'; for p in $$list; do \
		echo "Test $$p" ; \
		./$(EXEC) ../tests/$$p.in > difftest ; \
		diff ../tests/$$p.out difftest ; \
	done

runhards: setvars
	@list='$(HARDS)'; for p in $$list; do \
		echo "Test $$p" ; \
		./$(EXEC) ../tests/$$p.in > difftest ; \
		diff ../tests/$$p.out difftest ; \
	done

setvars:
ifeq ($(UNAME), windows32)
EXEC=3dlogic.exe
else
EXEC=3dlogic
endif

