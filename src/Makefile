#============================================================================#
#                   MERLIN, IEETA/DETI, UNIVERSITY OF AVEIRO                 #
#============================================================================#
BIN    = .
CC     = gcc
CPLP   = -ffast-math -msse2
#-----------------------------------------------------------------------------
CFLAGS = -O3 -Wall $(CPLP)
#-----------------------------------------------------------------------------
LIBS   = -lm
DEPS   = defs.h
PROGS  = $(BIN)/MERLIN 
OBJS     = mem.o misc.o args.o reads.o buffer.o
#-----------------------------------------------------------------------------
all:
	$(MAKE) progs
progs: $(PROGS)
$(BIN)/MERLIN: merlin.c $(DEPS) $(OBJS) 
	$(CC) $(CFLAGS) -o $(BIN)/MERLIN merlin.c $(OBJS) $(LIBS)
mem.o: mem.c mem.h $(DEPS)
	$(CC) -c $(CFLAGS) mem.c
misc.o: misc.c misc.h $(DEPS)
	$(CC) -c $(CFLAGS) misc.c
args.o: args.c args.h $(DEPS)
	$(CC) -c $(CFLAGS) args.c
reads.o: reads.c reads.h $(DEPS)
	$(CC) -c $(CFLAGS) reads.c
buffer.o: buffer.c buffer.h $(DEPS)
	$(CC) -c $(CFLAGS) buffer.c
clean: 
	/bin/rm -f *.o
cleanall:
	/bin/rm -f *.o $(PROGS)
#-----------------------------------------------------------------------------
