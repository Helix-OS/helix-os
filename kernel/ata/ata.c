#include <ata/ata.h>

char *depends[] = { "pci", "base", 0 };
char *provides = "ata";

static ata_device_t *ata_list;

void ata_enumerate_devices( ){
	pci_device_t *pci_list,
		     *temp;
	ata_device_t *move,
		     *last;

	// TODO: Free old devices on repeated calls
	last = ata_list = 0;

	pci_list = pci_get_device_list( );
	for ( temp = pci_list; temp; temp = temp->next ){

		temp = pci_get_device_by_class( temp, PCI_CLASS_MASS_STORAGE );
		if ( !temp )
			break;

		if ( pci_get_subclass_id( temp ) == PCI_SCLASS_MASS_IDE ){
			kprintf( "[%s] Have IDE controller\n", provides );

			move = kmalloc( sizeof( ata_device_t ));
			move->type = ATA_TYPE_IDE;
			move->mode = ATA_MODE_PIO;

			//move->pci_device = *temp;
			memcpy( &move->pci_device, temp, sizeof( pci_device_t ));
			move->next = 0;

			if ( last )
				last->next = move;
			else
				last = ata_list = move;
		}
	}
}

void ata_initialize_devices( ){
	ata_device_t *move;
	move = ata_list;

	for ( move = ata_list; move; move = move->next ){
		kprintf( "[%s] Initializing device...\n", provides );

		switch ( move->type ){
			case ATA_TYPE_IDE:
				ata_initialize_ide( move );
				break;

			default:
				break;
		}
	}
}

int init( ){
	pci_device_t *pci_list;

	kprintf( "[%s] Hello world!\n", provides );
	kprintf( "[%s] This is module \"%s\", and I'm in yo kernelz\n",
			provides, provides );

	ata_enumerate_devices( );
	ata_initialize_devices( );

	return 0;
}

void remove( ){
	kprintf( "[%s] Mkkay, I'm out.\n", provides ); 
}
