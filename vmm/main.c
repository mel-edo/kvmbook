#include <stdio.h>
#include <linux/kvm.h>
#include "vm.h"
#include "vcpu.h"

int main(void) {
    VM vm;
    VCPU vcpu;

    if (vm_init(&vm) < 0) return 1;

    printf("VM initialized, %d MB guest RAM at host vaddr %p\n",
            MEM_SIZE / (1024 * 1024), vm.mem);

    // Initialize the vCPU
    if (vcpu_init(&vm, &vcpu) < 0) {
        vm_cleanup(&vm);
        return 1;
    }

    printf("vCPU initialized successfully\n");

    vcpu_cleanup(&vcpu);
    vm_cleanup(&vm);
    return 0;
}