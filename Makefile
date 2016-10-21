
ifdef DEBUG
CFLAGS  := -g -Wall -O0
else
CFLAGS  := -pg -Wall -O3 -msse3
endif

OBJECTS :=  img.o context.o tpl.o pnm.o  ascmat.o galois.o
INCFLAGS = -I../dist
LIBS    := -lm
TARGETS := arte mktpl

all:  $(TARGETS)

.c.o:
	cc -o $@ -c $(CFLAGS) $(INCFLAGS) $<

clean:
	rm -f *.o $(TARGETS)

arte: arte.o $(OBJECTS)
	cc -o $@ $< $(OBJECTS) $(LDFLAGS) $(LIBS)

mktpl: mktpl.o $(OBJECTS)
	cc -o $@ $< $(OBJECTS) $(LDFLAGS) $(LIBS)

