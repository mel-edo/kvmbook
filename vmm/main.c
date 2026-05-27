#include <stdio.h>
#include <linux/kvm.h>
#include "vm.h"
#include "vcpu.h"

int main(void) {
    VM vm;
    VCPU vcpu;

    if (vm_init(&vm) < 0) return 1;
    // Initialize the vCPU
    if (vcpu_init(&vm, &vcpu) < 0) {
        vm_cleanup(&vm);
        return 1;
    }

    printf("vCPU initialized successfully\n");
    
    printf("VM initialized, %d MB guest RAM at host vaddr %p\n",
        MEM_SIZE / (1024 * 1024), vm.mem);
        
    if (vcpu_load_binary(&vcpu, "guest/boot.bin") < 0) {
        vcpu_cleanup(&vcpu);
        vm_cleanup(&vm);
        return 1;
    }

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