all: boot.bin

boot.bin: boot.asm
	nasm -f bin -o boot.bin boot.asm

clean:
	rm -f boot.bin