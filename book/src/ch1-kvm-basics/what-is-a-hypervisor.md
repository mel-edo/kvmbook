## What is a VMM

A VMM's job is to create and control a virtual machine. Specifically it's responsible for three things:

**Memory** - It allocates a chunk of RAM from the host and tells KVM that this is the guest's physical memory. The guest thinks it has real RAM but it's just a big buffer in your process.

**CPU** - It creates virtual CPUs, sets their initial register state and runs them. If the guest does something the real hardware can't or shouldn't handle alone (reading from a device, executing a HLT), the vCPU stops and hands control back to the VMM. The VMM handles this and resumes the guest. This back and forth is the core loop of a VMM.

> HLT is an x86 instruction that tells the CPU to stop executing until an interrupt arrives. It's the simplest guest event we can catch when we begin coding up our VMM.

**Devices** - The guest expects a real keyboard, disk etc. The VMM fakes these for example, when the guest tries to write to a serial port, VMM intercepts and prints to stdout. When guest tries to read from a disk, VMM reads from a file and hands the data back and so on.

Some of the ones you may have already used or heard about include QEMU, AWS Firecracker and by the end of this book, you will have one too!

But a VMM can't do any of this alone , it needs a hypervisor underneath it to actually virtualize the CPU.
## What is a hypervisor

A hypervisor is a piece of software which lets you run a computer inside your computer. It manages CPU time, memory and I/O so that each guest thinks it has the machine to itself.

There are two types. Type 1 hypervisors run directly on bare metal, no host OS needed. Some examples are VMware ESXi and Xen. Type 2 hypervisors run on top of your OS like any other process. VirtualBox is the classic example.

KVM is a kernel module that turns the Linux kernel itself into a type 1 hypervisor but from our perspective, we just interact with it like a type 2. We write a normal C program, open /dev/kvm, make ioctl calls and let the kernel do the heavy lifting.

[Next -> KVM ioctl api](kvm-ioctl-api.md)