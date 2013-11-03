ORCOS 
=====
MainPage: http://orcos.github.io/ORCOS/

ORCOS is a highly configurable Realtime Operating System written in C++ for the following platforms:
- PPC405
- SPARC LEON3
- QEMU
- ARMv4T, ARMv7 featuring BeagleBoard(xM)

Its key features are its C++ architecture, its highly customizable SCL language based configurabilty and the low system call overhead. 
Workerthreads can be used to lower interrupt latencies dramatically. Of course key RT-concepts as Mutexes using PIP are implemented.

Features:
- Deterministic Realtime Scheduling
	- Fixed Priority Scheduling, FCFS, RM, EDF, EDF + TBS
- Linear and Sequential Fit Memory Management
- Patched LWIP stack for dual IPv4/IPv6 Stack Operation
	- IPv4, IPv6, TCP, UDP, DHCP (untested), ARP, NDP support
- EHCI USB Host Controller (Platform independent)
	- USB HUB Support
	- SCSI USB Mass Storage Device Class Support
	- SMSC USB Ethernet Support (BeagleBoardxM Ethernet Port)
- Full Virtual Memory Support for PPC405 and ARMv7
- Filesystem Support:
	- DOS MBR Partitions supported
	- EFI Partitions in development
	- FAT16 Read / FAT32 Read/Write 

Platform Support:	
	
- BeagleBoard(xM) Support:
	- Timer + Clock (32768 Hz resolution)
	- GPIO (Auto Muxed) (6x)
	- UART (2x)
	- I2C
	- USB HC
	- Ethernet: SMSC USB Ethernet Support
	- MMC/SD 
	- TWL4030 Power Control

- RAPTOR:
	- Ethernet
	- Timer  + RT-Clock
	- Serial

For further information have a look at the source directory or the auto-generated
doxygen documentation.	
	
