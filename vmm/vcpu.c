#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>
#include "vcpu.h"

int vcpu_set_registers(VCPU *vcpu) {
    struct kvm_sregs sregs;
    struct kvm_regs regs;

    // Get current special registers
    if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0) {
        perror("KVM_GET_SREGS");
        return -1;
    }

    // Set code segment (CS) base to 0
    // ensures physical address = CS.base + RIP = 0 + 0x7C00 = 0x7C00
    sregs.cs.base = 0;
    sregs.cs.selector = 0;

    if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &sregs) < 0) {
        perror("KVM_SET_SREGS");
        return -1;
    }

    // Set standard registers (RIP and RFLAGS)
    memset(&regs, 0, sizeof(regs));
    regs.rip = 0x7C00;  // standard BIOS bootloader address
    regs.rflags = 0x2;  // x86 architecture requires bit 1 to always be set

    if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &regs) < 0) {
        perror("KVM_SET_REGS");
        return -1;
    }

    return 0;
}

int vcpu_init(VM *vm, VCPU *vcpu) {
    vcpu->vm = vm;
    vcpu->vcpu_fd = -1;

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
    
    if (vcpu->vcpu_fd >= 0) {
        close(vcpu->vcpu_fd);
    }
}

int vcpu_load_binary(VCPU *vcpu, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("Failed to open binary");
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    
    // Load guest binary at 0x7C00
    // Cast vm->mem to byte pointer, add 0x7C00 and read max 512 bytes
    size_t read_bytes = fread((uint8_t *)vcpu->vm->mem + 0x7C00, 1, size, f);
    printf("Loaded %zu bytes into guest memory at 0x7C00\n", read_bytes);
    fclose(f);
    return 0;
}

void vcpu_dump_regs(VCPU *vcpu) {
    struct kvm_regs regs;
    struct kvm_sregs sregs;

    if (ioctl(vcpu->vcpu_fd, KVM_GET_REGS, &regs) < 0) { perror("KVM_GET_REGS"); return; }
    if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &sregs) < 0) { perror("KVM_GET_SREGS"); return; }

    printf("RIP: %016llx RFLAGS: %016llx\n", regs.rip, regs.rflags);
    printf("CS: base=%016llx selector=%04x\n", sregs.cs.base, sregs.cs.selector);
    printf("CR0: %016llx\n", sregs.cr0);
}

int vcpu_run(VCPU *vcpu) {
    printf("Starting vCPU run loop...\n");

    while (1) {
        // Hand control to the guest. This blocks until a VM exit occurs.
        if (ioctl(vcpu->vcpu_fd, KVM_RUN, 0) < 0) {
            perror("KVM_RUN");
            return -1;
        }

        // KVM_RUN returned. The guest paused. Check the shared run struct to find out why
        switch (vcpu->run->exit_reason) {
            case KVM_EXIT_HLT:
                // The guest executed the 'hlt' instruction
                printf("KVM_EXIT_HLT: Guest executed HLT and halted\n");
                vcpu_dump_regs(vcpu);
                return 0;  // we treat this as a succesful clean exit for now

            case KVM_EXIT_IO:
                // Guest tried to read/write to a hardware port (like a serial port)
                if (vcpu->run->io.direction == KVM_EXIT_IO_OUT) {
                    printf("KVM_EXIT_IO: Guest wrote to port 0x%x\n", vcpu->run->io.port);
                } else {
                    printf("KVM_EXIT_IO: Guest read from port 0x%x\n", vcpu->run->io.port);
                }
                // exit VMM for now (we will fix this later)
                return 0;

            case KVM_EXIT_FAIL_ENTRY:
                // Hardware refused to enter the guest, register state might be invalid
                fprintf(stderr, "KVM_EXIT_FAIL_ENTRY: Hardware entry failure reason 0x%llx\n",
                        (unsigned long long)vcpu->run->fail_entry.hardware_entry_failure_reason);
                return -1;
            
            case KVM_EXIT_INTERNAL_ERROR:
                // KVM itself hit a bug or unsupported state
                fprintf(stderr, "KVM_EXIT_INTERNAL_ERROR: suberror 0x%x\n",
                        vcpu->run->internal.suberror);
                return -1;
            
            default:
                // We hit a exit we haven't written a handler for yet
                fprintf(stderr, "Unhandled exit reason: %d\n", vcpu->run->exit_reason);
                return -1;
        }
    }
}