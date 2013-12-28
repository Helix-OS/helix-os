#ifndef _helix_pci_h
#define _helix_pci_h
#include <base/stdint.h>

enum {
	CONFIG_ADDRESS 	= 0xcf8,
	CONFIG_DATA 	= 0xcfc,
};

typedef struct pci_device {
	uint8_t bus;
	uint8_t device;
	uint8_t function;
	uint16_t vendor_id;
	uint16_t dev_id;
	uint8_t  class_id;

	struct pci_device *next;
} pci_device_t;

#endif
