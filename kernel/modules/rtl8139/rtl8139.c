#include <base/logger.h>
#include <base/stdio.h>
#include <net/rtl8139/rtl8139.h>

char *depends[] = { "base", "pci", 0 };
char *provides = "rtl8139";
pci_device_t *device;

int init( ){
	pci_device_t *pci_list;

	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );

	kprintf( "[%s] Initializing...\n", provides );

	pci_list = pci_get_device_list( );
	device = pci_get_device_by_class( pci_list, PCI_CLASS_NETWORK );

	if ( device && device->vendor_id == 0x10ec && device->dev_id == 0x8139 ){
		kprintf( "[%s] Have rtl8139 NIC at 0x%x\n", provides, device );

		if ( initialize_rtl8139( device )){
			kprintf( "[%s] Initialized successfully.\n", provides );
		}

	} else {
		kprintf( "[%s] could not find rtl8139 device.\n", provides );
	}

	return 0;
}

bool initialize_rtl8139( pci_device_t *device ){
	bool ret = true;
	uint32_t ioaddr_read;
	uint16_t ioaddr;

	kprintf( "[%s] Header type: 0x%x\n", provides, 
		pci_dev_conf_read_byte( device, PCI_CONFIG_HEAD_TYPE ));

	ioaddr_read = pci_dev_conf_read_dword( device, PCI_CONFIG_BAR0 );
	ioaddr = (uint16_t)ioaddr_read;
	kprintf( "[%s] Base io address: 0x%x\n", provides, ioaddr );

	kprintf( "[%s] Mac0-5: 0x%x\n", provides, inl((uint16_t)ioaddr ));

	kprintf( "[%s] Resetting device...", provides );
	outb( ioaddr + RTL8139_CONFIG_1, 0x0 );
	outb( ioaddr + RTL8139_CMD, 0x10 );

	while ( inb( ioaddr + RTL8139_CMD ) & 0x10 );
	kprintf( "[%s] Done.\n", provides );

	return ret;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
