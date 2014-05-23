CC=gcc
CFLAGS=-Wall -std=c99 -I../includes
OFLAGS=-L/workspace_c/tlqc/lib -lbuddy -llibexpat-1 
all: pickaxe

pickaxe: pickaxe.o transition_system.o ctl.o ctl.leg.o core.o
	$(CC) $(OFLAGS) -o pickaxe.exe pickaxe.o transition_system.o ctl.o ctl.leg.o core.o
	
pickaxe.o: pickaxe.c
	$(CC) $(CFLAGS) -c pickaxe.c 

transition_system.o: transition_system.c
	$(CC) $(CFLAGS) -c transition_system.c 

core.o: core.c
	$(CC) $(CFLAGS) -c core.c 
	
ctl.o: ctl.c
	$(CC) $(CFLAGS) -c ctl.c 

ctl.leg.o: ctl.leg.c
	$(CC) $(CFLAGS) -c ctl.leg.c 
	
ctlleg: ctl.leg.c
	$(CC) $(OFLAGS) -o ctlleg ctl.leg.c ctl.c
	
ctl.leg.c: leg
	../bin/leg -o ctl.leg.c ctl.leg
	
leg:
	
clean:
	del *o tlqc.exe