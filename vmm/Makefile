CC = gcc
CFLAGS = -Wall -Wextra -g

vmm: main.o vm.o vcpu.o
	$(CC) $(CFLAGS) -o vmm main.o vm.o vcpu.o

main.o: main.c vm.h
	$(CC) $(CFLAGS) -c main.c

vm.o: vm.c vm.h
	$(CC) $(CFLAGS) -c vm.c

vcpu.o: vcpu.c vcpu.h vm.h
	$(CC) $(CFLAGS) -c vcpu.c

clean:
	rm -f *.o vmm