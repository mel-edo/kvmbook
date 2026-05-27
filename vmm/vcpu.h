#ifndef VCPU_H
#define VCPU_H

#include <linux/kvm.h>
#include "vm.h"

typedef struct {
    int vcpu_fd;
    struct kvm_run *run;  // Shared memory window between host and KVM
    VM *vm;  // Back pointer to the parent VM container
} VCPU;

int vcpu_init(VM *vm, VCPU *vcpu);
void vcpu_cleanup(VCPU *vcpu);
int vcpu_set_registers(VCPU *vcpu);
int vcpu_run(VCPU *vcpu);
int vcpu_load_binary(VCPU *vcpu, const char *path);

#endif