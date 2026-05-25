#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>
#include "vcpu.h"

int vcpu_init(VM *vm, VCPU *vcpu) {
    vcpu->vm = vm;

    // Ask KVM to spawn a vCPU thread for this VM (0 is the vCPU ID)
    vcpu->vcpu_fd = ioctl(vm->vm_fd, KVM_CREATE_VCPU, 0);
    if (vcpu->vcpu_fd < 0) {
        perror("KVM_CREATE_VCPU");
        return -1;
    }

    // KVM dictates how much memory the run struct needs; we have to ask it
    int mmap_size = ioctl(vm->kvm_fd, KVM_GET_VCPU_MMAP_SIZE, NULL);
    if (mmap_size < 0) {
        perror("KVM_GET_VCPU_MMAP_SIZE");
        return -1;
    }

    // Map the kvm_run struct into our process space
    // This is how KVM tells us why the guest paused (e.g. KVM_EXIT_HLT, KVM_EXIT_IO)
    vcpu->run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu->vcpu_fd, 0);
    if (vcpu->run == MAP_FAILED) {
        perror("mmap kvm_run");
        return -1;
    }

    return 0;
}

void vcpu_cleanup(VCPU *vcpu) {
    // You have to ask KVM for the size again to munmap cleanly
    if (vcpu->run && vcpu->vm) {
        int mmap_size = ioctl(vcpu->vm->kvm_fd, KVM_GET_VCPU_MMAP_SIZE, NULL);
        if (mmap_size > 0) {
            munmap(vcpu->run, mmap_size);
        }
    }
    
    if (vcpu->vcpu_fd) {
        close(vcpu->vcpu_fd);
    }
}