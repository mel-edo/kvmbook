#ifndef VM_H
#define VM_H

#include <stdint.h>

#define MEM_SIZE (128 * 1024 * 1024)  // 128MB

typedef struct {
    int kvm_fd;
    int vm_fd;
    void *mem;  // host virtual address of guest RAM
} VM;

int vm_init(VM *vm);
void vm_cleanup(VM *vm);

#endif