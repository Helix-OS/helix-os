#include <base/stdio.h>
#include <pci/pci.h>

char *depends[] = { "base", "hal", 0 };
char *provides = "pci";

static pci_device_t *pci_list = 0;

static uint32_t pci_conf_read_dword( char bus, char slot, char func, char offset ){
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = func;
	uint32_t ret = 0;

	address = (uint32_t)((lbus << 16) | (lslot << 11) | 
			(lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

	outl( PCI_CONFIG_ADDRESS, address );

	ret = inl( PCI_CONFIG_DATA );

	return ret;
}

static uint16_t pci_conf_read_word( char bus, char slot, char func, char offset ){
	uint32_t temp;
	uint16_t ret = 0;

	temp = pci_conf_read_dword( bus, slot, func, offset );

	ret = (uint16_t)(( temp >> ((offset & 2 ) * 8 )) & 0xffff );

	return ret;
}

static uint8_t pci_conf_read_byte( char bus, char slot, char func, char offset ){
	uint32_t temp;
	uint8_t ret;

	temp = pci_conf_read_dword( bus, slot, func, offset );
	ret = ( temp >> ( offset % 4 * 8 ));

	return ret;
}

static void pci_conf_write_dword( char bus, char slot, char func, char offset, uint32_t data ){
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = func;

	address = (uint32_t)((lbus << 16) | (lslot << 11) | 
			(lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

	outl( PCI_CONFIG_ADDRESS, address );
	outl( PCI_CONFIG_DATA, data );
}

static void pci_conf_write_byte( char bus, char slot, char func, char offset, uint8_t data ){
	uint32_t temp;
	uint32_t datal = data;

	datal <<= ( offset % 4 * 8 );

	temp = pci_conf_read_dword( bus, slot, func, offset );
	temp &= ~( 0xff << ( offset % 4 * 8 ));
	temp |= datal;

	pci_conf_write_dword( bus, slot, func, offset, temp );
}

uint32_t pci_dev_conf_read_dword( pci_device_t *device, char offset ){
	if ( device )
		return pci_conf_read_dword( device->bus, device->device, device->function, offset );

	return 0;
}

uint16_t pci_dev_conf_read_word( pci_device_t *device, char offset ){
	if ( device )
		return pci_conf_read_word( device->bus, device->device, device->function, offset );

	return 0;
}

uint8_t pci_dev_conf_read_byte( pci_device_t *device, char offset ){
	if ( device )
		return pci_conf_read_byte( device->bus, device->device, device->function, offset );

	return 0;
}

void pci_dev_conf_write_dword( pci_device_t *device, char offset, uint32_t data ){
	if ( device )
		pci_conf_write_dword( device->bus, device->device, device->function, offset, data );
}

void pci_dev_conf_write_byte( pci_device_t *device, char offset, uint8_t data ){
	if ( device )
		pci_conf_write_byte( device->bus, device->device, device->function, offset, data );
}

static void pci_enumerate_devices( ){
	int bus, device, function;
	uint16_t vendor;
	uint16_t dev_id;
	uint8_t class_id;

	pci_device_t *last_dev,
		     *new_dev;

	if ( pci_list ){
		kprintf( "[pci_enumerate_devices] Reenumerating devices\n" );
		pci_device_t *move = pci_list,
			     *temp;

		for ( ; move; move = temp ){
			temp = move->next;
			kfree( move );
		}
	}

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

uint8_t pci_get_class_id( pci_device_t *device ){
	uint8_t ret;
	//ret = (uint8_t)(pci_conf_read_dword( device->bus, device->device, device->function, 0xb ) >> 24 );
	ret = pci_conf_read_byte( device->bus, device->device, device->function, PCI_CONFIG_CLASS );

	return ret;
}

uint8_t pci_get_subclass_id( pci_device_t *device ){
	uint8_t ret;
	//ret = (uint8_t)(pci_conf_read_dword( device->bus, device->device, device->function, 0xa ) >> 24 );
	ret = pci_conf_read_byte( device->bus, device->device, device->function, PCI_CONFIG_SUBCLASS );

	return ret;
}

pci_device_t *pci_get_device_list( ){
	return pci_list;
}

pci_device_t *pci_get_device_by_class( pci_device_t *list, uint8_t class ){
	pci_device_t *ret = 0;

	if ( list ){
		if ( list->class_id == class )
			ret = list;
		else
			ret = pci_get_device_by_class( list->next, class );
	}

	return ret;
}

void pci_dump_devices( ){
	pci_device_t *move = pci_list;
	char *pci_strings[] = {
		"Old",
		"Mass storage controller",
		"Network controller",
		"Display controller",
		"Multimedia controller",
		"Memory controller",
		"Bridge device",
		"Simple communications controller",
		"Base system peripheral",
		"Input device controller",
		"Dock station controller",
		"Processor",
		"Serial bus controller",
		"Wireless controller",
		"Intelligent IO controller",
		"Satallite communications controller",
		"Crypto controller",
		"Data acquisition/Signal processing controller"
	};
	char *temp;

	while( move ){
		temp = "Undefined device";
		if ( move->class_id < PCI_CLASS_RESERVED )
			temp = pci_strings[ move->class_id ];

		kprintf( "[%s] pci %x:%x.%x: vendor = 0x%x, id = 0x%x, class = 0x%x (%s), subclass = 0x%x\n", 
			provides, move->bus, move->device, move->function,
			move->vendor_id, move->dev_id, move->class_id, temp,
			pci_get_subclass_id( move ));

		kprintf( "[%s]     BAR0: 0x%x\n", provides, pci_dev_conf_read_dword( move, PCI_CONFIG_BAR0 ));
		kprintf( "[%s]     BAR1: 0x%x\n", provides, pci_dev_conf_read_dword( move, PCI_CONFIG_BAR1 ));
		kprintf( "[%s]     BAR2: 0x%x\n", provides, pci_dev_conf_read_dword( move, PCI_CONFIG_BAR2 ));
		kprintf( "[%s]     BAR3: 0x%x\n", provides, pci_dev_conf_read_dword( move, PCI_CONFIG_BAR3 ));
		kprintf( "[%s]     BAR4: 0x%x\n", provides, pci_dev_conf_read_dword( move, PCI_CONFIG_BAR4 ));
		kprintf( "[%s]     BAR5: 0x%x\n", provides, pci_dev_conf_read_dword( move, PCI_CONFIG_BAR5 ));

		move = move->next;
	}
}

int init( ){
	kprintf( "[%s] initializing %s\n", provides, provides );
	pci_enumerate_devices( );
	pci_dump_devices( );
	kprintf( "[%s] Initialized\n", provides );
	return 0;
}

void remove( ){
	kprintf( "[%s] removing %s\n", provides, provides );
}
