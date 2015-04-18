Helix OS
========

Helix OS is a rewrite of OS-Thing from scratch, with an emphasis on simplicity and correctness.
The end goal is to be a versatile kernel which is easily modifiable by a programmer.

It currently supports:

- Module loading and linking
- Kernel multithreading
- VFS with multiple file systems
- PCI devices 
- IDE disks
- VGA text mode
- FAT12/16 filesystem
- VBE framebuffer with psf font support

Next steps:

- Start working on the userland
- TCP/IP stack
- More filesystem drivers, e.g. EXT2

After that:

- Port development tools
- Be self-hosting
