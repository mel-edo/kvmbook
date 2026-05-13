#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>

int main(void) {
    int kvm_fd, vm_fd, vcpu_fd;

    // Step 1: open /dev/kvm
    kvm_fd = open("/dev/kvm", O_RDWR);
    if (kvm_fd < 0) { perror("open /dev/kvm"); return 1; }

    // Step 2: create a VM
    vm_fd = ioctl(kvm_fd, KVM_CREATE_VM, 0);
    if (vm_fd < 0) { perror("KVM_CREATE_VM"); return 1; }

    // Step 3: create a vCPU
    vcpu_fd = ioctl(vm_fd, KVM_CREATE_VCPU, 0);
    if (vcpu_fd < 0) { perror("KVM_CREATE_VCPU"); return 1; }

    // Step 4: allocate one page of memory for the guest
    void *guest_mem = mmap(NULL, 0x1000,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS,
                            -1, 0);
    if (guest_mem == MAP_FAILED) { perror("mmap"); return 1; }

    // write HLT (0xf4) at offset 0 - guest starts here
    ((uint8_t *)guest_mem)[0] = 0xf4;

    // Step 5: tell KVM to map this memory at guest physical address 0
    struct kvm_userspace_memory_region region = {
        .slot            = 0,       // memory slot index, just use 0
        .flags           = 0,
        .guest_phys_addr = 0x0,     // guest sees this memory at physical address 0
        .memory_size     = 0x1000,  // one page (4KB)
        .userspace_addr  = (uint64_t)guest_mem,
    };

    if (ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, &region) < 0) {
        perror("KVM_SET_USER_MEMORY_REGION"); return 1;
    }

    printf("kvm_fd=%d vm_fd=%d vcpu_fd=%d\n", kvm_fd, vm_fd, vcpu_fd);

    close(vcpu_fd);
    close(vm_fd);
    close(vcpu_fd);
    return 0;
}