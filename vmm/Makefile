CC = gcc
CFLAGS = -Wall -Wextra -g

vmm: main.o vm.o
	$(CC) $(CFLAGS) -o vmm main.o vm.o

main.o: main.c vm.h
	$(CC) $(CFLAGS) -c main.c

vm.o: vm.c vm.h
	$(CC) $(CFLAGS) -c vm.c

clean:
	rm -f *.o vmm