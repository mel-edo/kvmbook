#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>

void dump_regs(int vcpu_fd) {
    struct kvm_regs regs;
    struct kvm_sregs sregs;

    if (ioctl(vcpu_fd, KVM_GET_REGS, &regs) < 0) { perror("KVM_GET_REGS"); return; }
    if (ioctl(vcpu_fd, KVM_GET_SREGS, &sregs) < 0) { perror("KVM_GET_SREGS"); return; }

    printf("RIP: %016llx RFLAGS: %016llx\n", regs.rip, regs.rflags);
    printf("RAX: %016llx RBX: %016llx RCX: %016llx RDX: %016llx\n",
            regs.rax, regs.rbx, regs.rcx, regs.rdx);
    printf("RSI: %016llx RDI: %016llx RSP: %016llx RBP: %016llx\n",
            regs.rsi, regs.rdi, regs.rsp, regs.rbp);
    printf("CS: base=%016llx selectro=%04x\n", sregs.cs.base, sregs.cs.selector);
    printf("CR0: %016llx CR3: %016llx CR4: %016llx\n",
            sregs.cr0, sregs.cr3, sregs.cr4);
}

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

    // Step 6: get the mmap size KVM needs for the run struct, then map it
    int mmap_size = ioctl(kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (mmap_size < 0) { perror("KVM_GET_VCPU_MMAP_SIZE"); return 1; }

    struct kvm_run *run = mmap(NULL, mmap_size,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            vcpu_fd, 0);  // mmap on vcpu_fd, not /dev/kvm
    if (run == MAP_FAILED) { perror("mmap kvm_run"); return 1; }

    // Step 7: fix up CS so the guest executes at physical address 0x0
    struct kvm_sregs sregs;
    if (ioctl(vcpu_fd, KVM_GET_SREGS, &sregs) < 0) { perror("KVM_GET_SREGS"); return 1; }
    sregs.cs.base = 0;
    sregs.cs.selector = 0;
    if (ioctl(vcpu_fd, KVM_SET_SREGS, &sregs) < 0) { perror("KVM_SET_SREGS"); return 1; }

    // Step 8: set RIP to 0 -> combined with cs.base=0, guest starts at physical 0x0
    struct kvm_regs regs = {
        .rip = 0,
        .rflags = 0x2,  // bit 1 is always required to be set by the x86 spec
    };
    if (ioctl(vcpu_fd, KVM_SET_REGS, &regs) < 0) { perror("KVM_SET_REGS"); return 1; }

    // Step 9: run the vcpu and handle exits
    while (1) {
        if (ioctl(vcpu_fd, KVM_RUN, 0) < 0) { perror("KVM_RUN"); return 1; }

        switch (run->exit_reason) {
            case KVM_EXIT_HLT:
                printf("KVM_EXIT_HLT - guest executed HLT, we're done\n");
                dump_regs(vcpu_fd);
                goto done;

            case KVM_EXIT_IO:
                printf("KVM_EXIT_IO (unexpected this wekk)\n");
                goto done;
            
            case KVM_EXIT_FAIL_ENTRY:
                printf("KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx\n",
                        run->fail_entry.hardware_entry_failure_reason);
                goto done;

            case KVM_EXIT_INTERNAL_ERROR:
            printf("KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x\n",
                    run->internal.suberror);
                goto done;

            default:
                printf("unexpected exit case: %d\n", run->exit_reason);
                goto done;
        }
    }
    done:

    printf("kvm_fd=%d vm_fd=%d vcpu_fd=%d\n", kvm_fd, vm_fd, vcpu_fd);

    close(vcpu_fd);
    close(vm_fd);
    close(vcpu_fd);
    return 0;
}