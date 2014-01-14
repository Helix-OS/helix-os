#ifndef _helix_pci_h
#define _helix_pci_h
#include <base/stdint.h>

// PCI io ports
enum {
	PCI_CONFIG_ADDRESS 	= 0xcf8,
	PCI_CONFIG_DATA 	= 0xcfc,
};

// PCI class values
enum {
	PCI_CLASS_OLD 		= 0,
	PCI_CLASS_MASS_STORAGE 	= 1,
	PCI_CLASS_NETWORK 	= 2,
	PCI_CLASS_DISPLAY 	= 3,
	PCI_CLASS_MULTIMEDIA 	= 4,
	PCI_CLASS_MEMORY 	= 5,
	PCI_CLASS_BRIDGE 	= 6,
	PCI_CLASS_SIM_COM 	= 7,
	PCI_CLASS_BASE_PERIPH 	= 8,
	PCI_CLASS_INPUT_DEV 	= 9,
	PCI_CLASS_DOCK_STAT 	= 0xa,
	PCI_CLASS_PROCESSOR 	= 0xb,
	PCI_CLASS_SERIAL_BUS 	= 0xc,
	PCI_CLASS_WIRELESS 	= 0xd,
	PCI_CLASS_INTELLI_IO 	= 0xe,
	PCI_CLASS_SATALLITE_COM = 0xf,
	PCI_CLASS_CRYPT 	= 0x10,
	PCI_CLASS_DATAAC_SP 	= 0x11,
	PCI_CLASS_RESERVED 	= 0x12,
	/* Everything else between 0x11 and 0xff is reserved, and shouldn't be used. */
	PCI_CLASS_UNDEFINED 	= 0xff
};

// PCI subclass values for PCI_CLASS_MASS_STORAGE
enum {
	PCI_SCLASS_MASS_SCSI 	= 0,
	PCI_SCLASS_MASS_IDE 	= 1,
	PCI_SCLASS_MASS_FLOPPY 	= 2,
	PCI_SCLASS_MASS_IPI 	= 3,
	PCI_SCLASS_MASS_RAID 	= 4,
	PCI_SCLASS_MASS_ATA 	= 5,
	PCI_SCLASS_MASS_SATA 	= 6,
	PCI_SCLASS_MASS_SAS 	= 7,
	PCI_SCLASS_MASS_OTHER 	= 0x80
};

// Config offsets for header types of 0x0
enum {
	PCI_CONFIG_VENDOR 	= 0,
	PCI_CONFIG_DEVICE 	= 2,
	PCI_CONFIG_COMMAND 	= 4,
	PCI_CONFIG_STATUS 	= 6,
	PCI_CONFIG_REV_ID 	= 8,
	PCI_CONFIG_PROG_IF 	= 9,
	PCI_CONFIG_SUBCLASS 	= 0xa,
	PCI_CONFIG_CLASS 	= 0xb,
	PCI_CONFIG_CACHE_SIZE 	= 0xc,
	PCI_CONFIG_LAT_TIMER 	= 0xd,
	PCI_CONFIG_HEAD_TYPE 	= 0xe,
	PCI_CONFIG_BIST 	= 0xf,
	PCI_CONFIG_BAR0 	= 0x10,
	PCI_CONFIG_BAR1 	= 0x14,
	PCI_CONFIG_BAR2 	= 0x18,
	PCI_CONFIG_BAR3 	= 0x1c,
	PCI_CONFIG_BAR4 	= 0x20,
	PCI_CONFIG_BAR5 	= 0x24,
	PCI_CONFIG_CARDBUS 	= 0x28,
	PCI_CONFIG_SUBSYS_VEND 	= 0x2c,
	PCI_CONFIG_SUBSYS_ID 	= 0x2e,
	PCI_CONFIG_EXPAND_ROM 	= 0x30,
	PCI_CONFIG_CAPABLE  	= 0x34,
	PCI_CONFIG_INT_LINE 	= 0x3c,
	PCI_CONFIG_INT_PIN 	= 0x3d,
	PCI_CONFIG_MIN_GRANT 	= 0x3e,
	PCI_CONFIG_MAX_LAT 	= 0x3f
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

pci_device_t *pci_get_device_list( );
pci_device_t *pci_get_device_by_class( pci_device_t *list, uint8_t class );

uint8_t pci_get_class_id( pci_device_t *device );
uint8_t pci_get_subclass_id( pci_device_t *device );

uint32_t pci_dev_conf_read_dword( pci_device_t *device, char offset );
uint16_t pci_dev_conf_read_word( pci_device_t *device, char offset );
uint8_t  pci_dev_conf_read_byte( pci_device_t *device, char offset );
void 	 pci_dev_conf_write_dword( pci_device_t *device, char offset, uint32_t data );
void 	 pci_dev_conf_write_byte( pci_device_t *device, char offset, uint8_t data );

#endif
