#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>
#include "vm.h"

int vm_init(VM *vm) {
    // open /dev/kvm
    vm->kvm_fd = open("/dev/kvm", O_RDWR);
    if (vm->kvm_fd < 0) { perror("open /dev/kvm"); return -1; }

    // create the VM
    vm->vm_fd = ioctl(vm->kvm_fd, KVM_CREATE_VM, 0);
    if (vm->vm_fd < 0) { perror("KVM_CREATE_VM"); return -1; }

    // allocate 128MB guest RAM
    // this will live in the process's address space
    vm->mem = mmap(NULL, MEM_SIZE,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1, 0);
    if (vm->mem == MAP_FAILED) { perror("mmap guest mem"); return -1; }

    // tell KVM about this mem
    // slot 0, guest physical addr 0x0
    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0x0,
        .memory_size = MEM_SIZE,
        .userspace_addr = (uint64_t)vm->mem,
    };
    if (ioctl(vm->vm_fd, KVM_SET_USER_MEMORY_REGION, &region) < 0) {
        perror("KVM_SET_USER_MEMORY_REGION"); return -1;
    }

    return 0;
}

void vm_cleanup(VM *vm) {
    if (vm->mem) munmap(vm->mem, MEM_SIZE);
    if (vm->vm_fd) close(vm->vm_fd);
    if (vm->kvm_fd) close(vm->kvm_fd);
}