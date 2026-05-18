#include <stdio.h>
#include <linux/kvm.h>
#include "vm.h"

int main(void) {
    VM vm;

    if (vm_init(&vm) < 0) return 1;

    printf("VM initialized, %d MB guest RAM at host vaddr %p\n",
            MEM_SIZE / (1024 * 1024), vm.mem);

    vm_cleanup(&vm);
    return 0;
}