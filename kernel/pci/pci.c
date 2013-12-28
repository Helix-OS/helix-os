#include <base/stdio.h>
#include <pci/pci.h>

char *depends[] = { "base", 0 };
char *provides = "pci";

static pci_device_t *pci_list = 0;

uint32_t pci_conf_read_dword( char bus, char slot, char func, char offset ){
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = func;
	uint32_t ret = 0;

	address = (uint32_t)((lbus << 16) | (lslot << 11) | 
			(lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

	outl( CONFIG_ADDRESS, address );

	ret = inl( CONFIG_DATA );

	return ret;
}

uint16_t pci_conf_read_word( char bus, char slot, char func, char offset ){
	uint32_t temp;
	uint16_t ret = 0;

	temp = pci_conf_read_dword( bus, slot, func, offset );

	ret = (uint16_t)(( temp >> ((offset & 2 ) * 8 )) & 0xffff );

	return ret;
}

void pci_enumerate_devices( ){
	int bus, device, function;
	uint16_t vendor;
	uint16_t dev_id;
	uint8_t class_id;

	pci_device_t *last_dev,
		     *new_dev;

	// TODO; Free old list, and enumerate devices again
	if ( pci_list ) return;

	for ( bus = 0; bus < 256; bus++ ){
		for ( device = 0; device < 32; device++ ){
			for ( function = 0; function < 7; function++ ){
				if (( vendor = pci_conf_read_word( bus, device, function, 0 )) != 0xffff ){
					dev_id = pci_conf_read_word( bus, device, function, 2 );
					class_id = (uint8_t)(pci_conf_read_dword( bus, device, function, 0x0b ) >> 24);

					new_dev = kmalloc( sizeof( pci_device_t ));
					new_dev->bus = bus;
					new_dev->device = device;
					new_dev->function = function;
					new_dev->vendor_id = (uint16_t)vendor;
					new_dev->dev_id = dev_id;
					new_dev->class_id = class_id;
					new_dev->next = 0;

					if ( pci_list ){
						last_dev->next = new_dev;
						last_dev = new_dev;
					} else {
						last_dev = pci_list = new_dev;
					}


				} else if ( function == 0 ){
					break;
				}
			}
		}
	}
}

void pci_dump_devices( ){
	pci_device_t *move = pci_list;

	while( move ){
		kprintf( "[%s] pci %x:%x.%x: vendor = 0x%x, id = 0x%x, class = 0x%x\n", 
			provides, move->bus, move->device, move->function,
			move->vendor_id, move->dev_id, move->class_id );

		move = move->next;
	}
}

int init( ){
	kprintf( "[%s] initializing %s\n", provides, provides );
	pci_enumerate_devices( );
	pci_dump_devices( );
	return 0;
}

void remove( ){
	kprintf( "[%s] removing %s\n", provides, provides );
}
