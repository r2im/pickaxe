CC=gcc
CFLAGS=-Wall -std=c99 -I../includes -O2
OFLAGS=-L/workspace_c/tlqc/lib -lbuddy -llibexpat-1 
all: pickaxe

pickaxe: pickaxe.o transition_system.o ltl.o ltl.leg.o core.o
	$(CC) $(OFLAGS) -o pickaxe.exe pickaxe.o transition_system.o ltl.o ltl.leg.o core.o
	
pickaxe.o: pickaxe.c
	$(CC) $(CFLAGS) -c pickaxe.c 

transition_system.o: transition_system.c
	$(CC) $(CFLAGS) -c transition_system.c 

core.o: core.c
	$(CC) $(CFLAGS) -c core.c 
	
ltl.o: ltl.c
	$(CC) $(CFLAGS) -c ltl.c 

ltl.leg.o: ltl.leg.c
	$(CC) $(CFLAGS) -c ltl.leg.c 
	
ltlleg: ltl.leg.c
	$(CC) $(OFLAGS) -o ltlleg ltl.leg.c ltl.c
	
ltl.leg.c: leg
	../bin/leg -o ltl.leg.c ltl.leg
	
leg:
	
clean:
	del *o pickaxe.exe