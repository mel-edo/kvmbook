# So you want to build a VMM from scratch

Before diving into the deep end, let's establish what a hypervisor actually is — because it's the foundation of everything we're building.

## What is a hypervisor

At its core, a hypervisor is software that lets you run one computer inside another. It sits between the hardware and one or more guest operating systems, managing CPU time, memory, and I/O so each guest thinks it has the machine to itself.

There are two types. Type 1 hypervisors run directly on bare metal — no host OS underneath, just the hypervisor and the hardware. VMware ESXi and Xen work this way. Type 2 hypervisors run on top of a normal OS, like any other process — VirtualBox is the classic example.

KVM is the interesting case. It's a kernel module that turns Linux itself into a type 1 hypervisor, but from your perspective as a programmer you interact with it like type 2 — you write a normal Linux process, open /dev/kvm, and make ioctl calls. The kernel does the heavy lifting underneath.

That process you write is called a VMM — a Virtual Machine Monitor. QEMU is one. Firecracker is another. By the end of this book, yours will be too.