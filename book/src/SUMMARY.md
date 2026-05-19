# Summary

- [Chapter 1: KVM Basics](ch1-kvm-basics/README.md)
  - [What is a Hypervisor](ch1-kvm-basics/what-is-a-hypervisor.md)
  - [The KVM ioctl API](ch1-kvm-basics/kvm-ioctl-api.md)
  - [VM Exits](ch1-kvm-basics/vm-exits.md)

- [Chapter 2: Memory](ch2-memory/README.md)
  - [x86 Memory Model](ch2-memory/x86-memory-model.md)
  - [Real, Protected, and Long Mode](ch2-memory/real-protected-long-mode.md)
  - [KVM Memory Slots](ch2-memory/kvm-memory-slots.md)

- [Chapter 3: Serial Port](ch3-serial/README.md)
  - [I/O Port Emulation](ch3-serial/io-port-emulation.md)
  - [KVM_EXIT_IO](ch3-serial/kvm-exit-io.md)
  - [Why Serial](ch3-serial/why-serial.md)

- [Chapter 4: Interrupts](ch4-interrupts/README.md)
  - [x86 Interrupt Model](ch4-interrupts/x86-interrupt-model.md)
  - [IDT Structure](ch4-interrupts/idt-structure.md)
  - [APIC Basics](ch4-interrupts/apic-basics.md)
  - [Injecting Interrupts](ch4-interrupts/injecting-interrupts.md)
  - [Triple Faults](ch4-interrupts/triple-faults.md)

- [Chapter 5: virtio-blk](ch5-virtio-blk/README.md)
  - [What is virtio](ch5-virtio-blk/what-is-virtio.md)
  - [The Virtqueue Protocol](ch5-virtio-blk/virtqueue-protocol.md)
  - [Guest and Host Memory Sharing](ch5-virtio-blk/guest-host-memory-sharing.md)
  - [VMM Implementation](ch5-virtio-blk/vmm-implementation.md)
  - [Guest Driver](ch5-virtio-blk/guest-driver.md)

- [Chapter 6: Wrapup](ch6-wrapup/README.md)
  - [Going Production Grade](ch6-wrapup/production-grade.md)
  - [Firecracker and crosvm](ch6-wrapup/firecracker-crosvm.md)
  - [Further Reading](ch6-wrapup/further-reading.md)