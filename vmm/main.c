#include <stdio.h>
#include <linux/kvm.h>
#include "vm.h"
#include "vcpu.h"

int main(void) {
    VM vm;
    VCPU vcpu;

    if (vm_init(&vm) < 0) return 1;

    // Load guest binary at 0x7C00
    FILE *f = fopen("guest/boot.bin", "rb");
    if (!f) {
        perror("Failed to open guest/boot.bin");
        vm_cleanup(&vm);
        return 1;
    }

    // Cast vm.mem to byte pointer, add 0x7C00 and read max 512 bytes
    size_t read_bytes = fread((uint8_t *)vm.mem + 0x7C00, 1, 512, f);
    printf("Loaded %zu bytes into guest memory at 0x7C00\n", read_bytes);
    fclose(f);

    printf("VM initialized, %d MB guest RAM at host vaddr %p\n",
            MEM_SIZE / (1024 * 1024), vm.mem);

    // Initialize the vCPU
    if (vcpu_init(&vm, &vcpu) < 0) {
        vm_cleanup(&vm);
        return 1;
    }

    printf("vCPU initialized successfully\n");

    // set initial x86 registers
    if (vcpu_set_registers(&vcpu) < 0) {
        vcpu_cleanup(&vcpu);
        vm_cleanup(&vm);
        return 1;
    }

    printf("vCPU registers configured (RIP=0x7C00)\n");

    if (vcpu_run(&vcpu) < 0) {
        fprintf(stderr, "vCPU run loop failed or hit an error\n");
    }

    vcpu_cleanup(&vcpu);
    vm_cleanup(&vm);
    return 0;
}