#include <ata/ata.h>
#include <base/string.h>
#include <base/tasking/task.h>

static int ata_ide_irq_invoked = 0;

void ata_initialize_ide( ata_device_t *device ){
	pci_device_t *pci_device = &device->pci_device;
	ide_control_t *new_ctrl = kmalloc( sizeof( ide_control_t ));
	hal_device_t *hal_buf;

	int i, j, //needs_irq = 0,
	    count;
	uint32_t bar[5];
	char *buf = kmalloc( 2048 );
	//char *what = kmalloc( 25 );

	//kprintf( "device id: 0x%x\n", pci_dev_conf_read_word( pci_device, PCI_CONFIG_DEVICE ));
	kprintf( "device id: 0x%x\n", pci_device->dev_id );
	bar[0] = pci_dev_conf_read_dword( pci_device, PCI_CONFIG_BAR0 );
	bar[1] = pci_dev_conf_read_dword( pci_device, PCI_CONFIG_BAR1 );
	bar[2] = pci_dev_conf_read_dword( pci_device, PCI_CONFIG_BAR2 );
	bar[3] = pci_dev_conf_read_dword( pci_device, PCI_CONFIG_BAR3 );
	bar[4] = pci_dev_conf_read_dword( pci_device, PCI_CONFIG_BAR4 );

	for ( i = 0; i < 5; i++ )
		kprintf( "bar[%d] = 0x%x\n", i, bar[i] );

	memset( new_ctrl, 0, sizeof( ide_control_t ));
	new_ctrl->channels[ATA_PRIMARY 	].base = 0x1f0;
	new_ctrl->channels[ATA_PRIMARY 	].ctrl = 0x3f4;
	new_ctrl->channels[ATA_SECONDARY].base = 0x170;
	new_ctrl->channels[ATA_SECONDARY].ctrl = 0x374;
	/*
	new_ctrl->channels[ATA_PRIMARY 	].base = ( bar[0] & ~3 ) + 0x1f0 * !bar[0];
	new_ctrl->channels[ATA_PRIMARY 	].ctrl = ( bar[1] & ~3 ) + 0x3f4 * !bar[1];
	new_ctrl->channels[ATA_SECONDARY].base = ( bar[2] & ~3 ) + 0x170 * !bar[2];
	new_ctrl->channels[ATA_SECONDARY].ctrl = ( bar[3] & ~3 ) + 0x374 * !bar[3];
	*/
	new_ctrl->channels[ATA_PRIMARY 	].busmaster = ( bar[4] & ~3 );
	new_ctrl->channels[ATA_SECONDARY].busmaster = ( bar[4] & ~3 ) + 8;

	kprintf( "Primary status: 0x%x, secondary: 0x%x\n",
			ide_reg_read( new_ctrl, ATA_PRIMARY, ATA_REG_STATUS ),
			ide_reg_read( new_ctrl, ATA_SECONDARY, ATA_REG_STATUS ));

	ide_reg_write( new_ctrl, ATA_PRIMARY, ATA_REG_CONTROL, 2 );
	ide_reg_write( new_ctrl, ATA_SECONDARY, ATA_REG_CONTROL, 2 );

	for ( count = i = 0; i < 2; i++ ){
		for ( j = 0; j < 2; j++ ){	
			char error = 0,
			     status = 0;
			int k;

			//count = i * 2 + j;

			new_ctrl->devices[count].reserved = 0;

			ide_reg_write( new_ctrl, i, ATA_REG_HD_DEV_SELECT, 0xa0 | ( j << 4 ));
			usleep( 400 );

			ide_reg_write( new_ctrl, i, ATA_REG_COMMAND, ATA_CMD_IDENT );
			usleep( 400 );
			
			if ( ide_reg_read( new_ctrl, i, ATA_REG_STATUS ) == 0 ){
				//count++;
				continue;
			}

			while ( 1 ){
				status = ide_reg_read( new_ctrl, i, ATA_REG_STATUS );
				if ( status & ATA_STATUS_ERROR ){
					error = 1;
					break;
				}

				if (!( status & ATA_STATUS_BUSY ) && ( status & ATA_STATUS_DRQ )){
					break;
				}
			}

			if ( error ){
				// ATAPI detection to be added later
				count++;
				continue;
			}

			ide_read_buffer( new_ctrl, i, ATA_REG_DATA, buf, 128 );

			new_ctrl->devices[count].reserved 	= 1;
			new_ctrl->devices[count].type 		= ATA_TYPE_IDE;
			new_ctrl->devices[count].channel 	= i;
			new_ctrl->devices[count].drive 	 	= j;
			new_ctrl->devices[count].sign 		= *(unsigned short *)(buf + ATA_IDENT_DEVICE_TYPE );
			new_ctrl->devices[count].sign 		= *(unsigned short *)
								  ( buf + ATA_IDENT_DEVICE_TYPE );
			new_ctrl->devices[count].capabilities	= *(unsigned short *)
								  ( buf + ATA_IDENT_CAPABILITIES );
			new_ctrl->devices[count].commands	= *(unsigned *)
								  ( buf + ATA_IDENT_COMMAND_SETS );

			if ( new_ctrl->devices[count].commands & ( 1<<26 ))
				new_ctrl->devices[count].size = *(unsigned *)( buf + ATA_IDENT_MAX_LBA_EXT );
			else 
				new_ctrl->devices[count].size = *(unsigned *)( buf + ATA_IDENT_MAX_LBA );

			for ( k = 0; k < 40; k+=2 ){
				new_ctrl->devices[count].model[k] = buf[ k + 1 + ATA_IDENT_MODEL ];
				new_ctrl->devices[count].model[k + 1] = buf[ k + ATA_IDENT_MODEL ];
			}
			new_ctrl->devices[count].model[40] = 0;

			hal_buf = kmalloc( sizeof( hal_device_t ));
			hal_buf->read = (hal_device_read_block)ata_ide_hal_read;
			hal_buf->write = ata_ide_hal_write;
			//hal_buf->dev = new_ctrl->devices + count;
			hal_buf->dev = &new_ctrl->devices[count];
			hal_buf->block_size = 512;
			hal_buf->type = HAL_TYPE_STORAGE;
			hal_buf->flags = HAL_FLAG_NULL;

			hal_buf->name = strdup((char []){ 'a', 't', 'a', count + '0', 0 });

			kprintf( "[%s] Have \"%s\" hal_buf->dev at 0x%x, new_ctrl at 0x%x\n", __func__,
					hal_buf->name, hal_buf->dev, new_ctrl );
			hal_register_device( hal_buf );

			new_ctrl->devices[count].hal_buf = hal_buf;
			new_ctrl->devices[count].ctrl = new_ctrl;

			kprintf( "[ata_initialize_ide] Found device %d, slot %d:%d: 0x%d sectors, model: \"%s\"\n",
					count, i, j, new_ctrl->devices[count].size, new_ctrl->devices[count].model );

			count++;
		}
	}

	register_interrupt_handler( IRQ14, ide_irq_handler );
	register_interrupt_handler( IRQ15, ide_irq_handler );
	register_interrupt_handler( IRQ11, ide_irq_handler );
	register_interrupt_handler( IRQ9,  ide_irq_handler );

	//ata_ide_read_sectors( new_ctrl, 0, 1, 0, 0, buf );
	//ata_ide_hal_read( hal_buf, buf, 1, 0 );
	/*
	hal_buf->read( hal_buf, buf, 1, 0 );
	for ( i = 0; i < 512; i++ )
		kprintf( "%c", buf[i] );
		*/

	// Beware, there's heap corruption going on somewhere, can't pinpoint exactly where
	// Adding this memset seemed to "fix" it, TODO: actually fix this
	memset( buf, 0, 2048 );
	kfree( buf );
}

void ide_wait_irq( ){
	while( !ata_ide_irq_invoked );
	// TODO: Let the scheduler know we can sleep
	ata_ide_irq_invoked = 0;
}

void ide_irq_handler( ){
	ata_ide_irq_invoked = 1;
}

char ata_ide_pio_access( ide_control_t *ctrl, char direction, uint8_t drive, unsigned lba,
			uint8_t numsects, uint16_t select, unsigned edi )
{
	uint32_t channel = ctrl->devices[drive].channel,
		 slavebit = ctrl->devices[drive].drive,
		 //bus = ctrl->channels[drive].base,
		 bus = ctrl->channels[channel].base,
		 words = 256;
	uint16_t i;
	uint8_t cmd, mode = 1,
		lba_io[6] = { 0 },
		head = 0;

	// Disable interrupts
	ata_ide_irq_invoked = 0;
	ctrl->channels[channel].no_int = 2;
	ide_reg_write( ctrl, channel, ATA_REG_CONTROL, 2 );

	lba_io[0] = ( lba & 0xff );
	lba_io[1] = ( lba & 0xff00 ) >> 8;
	lba_io[2] = ( lba & 0xff0000 ) >> 16;
	
	if ( lba > 0x10000000 ){
		mode = 2;
		lba_io[3] = ( lba & 0xff000000 ) >> 24;
	}

	while ( ide_reg_read( ctrl, channel, ATA_REG_STATUS ) & ATA_STATUS_BUSY );

	ide_reg_write( ctrl, channel, ATA_REG_HD_DEV_SELECT, 0xe0 | (slavebit << 4) | head );

	if ( mode == 2 ){
		ide_reg_write( ctrl, channel, ATA_REG_SEC_COUNT1, 0 );
		ide_reg_write( ctrl, channel, ATA_REG_LBA3, lba_io[3] );
		ide_reg_write( ctrl, channel, ATA_REG_LBA4, lba_io[4] );
		ide_reg_write( ctrl, channel, ATA_REG_LBA5, lba_io[5] );
	}

	ide_reg_write( ctrl, channel, ATA_REG_SEC_COUNT0, numsects );
	ide_reg_write( ctrl, channel, ATA_REG_LBA0, lba_io[0] );
	ide_reg_write( ctrl, channel, ATA_REG_LBA1, lba_io[1] );
	ide_reg_write( ctrl, channel, ATA_REG_LBA2, lba_io[2] );

	if 	( mode == 1 && !direction ) cmd = ATA_CMD_READ_PIO;
	else if	( mode == 2 && !direction ) cmd = ATA_CMD_READ_PIO_EXT;
	else if	( mode == 1 &&  direction ) cmd = ATA_CMD_WRITE_PIO;
	else if	( mode == 2 &&  direction ) cmd = ATA_CMD_WRITE_PIO_EXT;
    else cmd = 0;

	ide_reg_write( ctrl, channel, ATA_REG_COMMAND, cmd );

	if ( direction == ATA_READ ){
		for ( i = 0; i < numsects; i++ ){
			if ( ide_polling( ctrl, channel, 1 ))
				return 0;

			asm volatile( "pushw %es" );
			asm volatile( "mov %%ax, %%es" :: "a"(select));
			asm volatile( "rep insw"::"c"(words), "d"(bus), "D"(edi));
			asm volatile( "popw %es" );
			edi += words * 2;
		}
	} else if ( direction == ATA_WRITE ){
		for ( i = 0; i < numsects; i++ ){
			if ( ide_polling( ctrl, channel, 1 ))
				return 0;

			asm volatile( "pushw %ds" );
			asm volatile( "mov %%ax, %%ds" :: "a"(select));
			asm volatile( "rep outsw"::"c"(words), "d"(bus), "D"(edi));
			asm volatile( "popw %ds" );
			edi += words * 2;
		}

		cmd = (mode == 1)? ATA_CMD_CACHE_FLUSH : ATA_CMD_CACHE_FLUSH_EXT;
		ide_reg_write( ctrl, channel, ATA_REG_COMMAND, cmd );
		ide_polling( ctrl, channel, 0 );
	}

	return 0;
}

char ata_ide_read_sectors( ide_control_t *ctrl, uint8_t drive, uint8_t numsects,
		uint32_t lba, uint16_t es, uint32_t edi )
{
	kprintf( "[%s] Have ctrl at 0x%x\n", __func__, ctrl );
	if ( drive > 3 || ctrl->devices[drive].reserved == 0 ){
		kprintf( "[%s] Don't have drive %d\n", __func__, drive );
		return 1;
	}

	if (( lba + numsects > ctrl->devices[drive].size ))
		return 2;

	char error = 0;

	if ( ctrl->devices[drive].type == ATA_TYPE_IDE )
		error = ata_ide_pio_access( ctrl, ATA_READ, drive, lba, numsects, es, edi );

	return error;
}

char ata_ide_write_sectors( ide_control_t *ctrl, uint8_t drive, uint8_t numsects,
		uint32_t lba, uint16_t es, uint32_t edi )
{
	if ( drive > 3 || ctrl->devices[drive].reserved == 0 )
		return 1;

	if (( lba + numsects > ctrl->devices[drive].size ))
		return 2;

	char error;

	if ( ctrl->devices[drive].type == ATA_TYPE_IDE )
		error = ata_ide_pio_access( ctrl, ATA_WRITE, drive, lba, numsects, es, edi );

	return error;
}

void ide_reg_write( ide_control_t *ctrl, uint8_t channel, uint8_t reg, uint8_t data ){

	if ( reg > 0x7 && reg < 0xc )
		ide_reg_write( ctrl, channel, ATA_REG_CONTROL, 0x80 | ctrl->channels[channel].no_int );

	if 	( reg < 0x08 ) 	outb( ctrl->channels[channel].base + reg, data );
	else if ( reg < 0x0c ) 	outb( ctrl->channels[channel].base + reg - 0x06, data );
	else if ( reg < 0x0e ) 	outb( ctrl->channels[channel].ctrl + reg - 0x0a, data );
	else if ( reg < 0x16 ) 	outb( ctrl->channels[channel].busmaster + reg - 0x0e, data );
	else 			kprintf( "[ide_reg_write] Have invalid register \"0x%x\"\n", reg );

	if ( reg > 0x7 && reg < 0x0c )
		ide_reg_write( ctrl, channel, ATA_REG_CONTROL, ctrl->channels[channel].no_int );
}

uint8_t ide_reg_read( ide_control_t *ctrl, uint8_t channel, uint8_t reg ){
	uint8_t ret;

	if ( reg > 0x7 && reg < 0xc )
		ide_reg_write( ctrl, channel, ATA_REG_CONTROL, 0x80 | ctrl->channels[channel].no_int );

	if 	( reg < 0x08 ) 	ret = inb( ctrl->channels[channel].base + reg );
	else if ( reg < 0x0c ) 	ret = inb( ctrl->channels[channel].base + reg - 0x06 );
	else if ( reg < 0x0e ) 	ret = inb( ctrl->channels[channel].ctrl + reg - 0x0a );
	else if ( reg < 0x16 ) 	ret = inb( ctrl->channels[channel].busmaster + reg - 0x0e );
	else {
		kprintf( "[ide_reg_read] Have invalid register \"0x%x\"\n", reg );
		ret = 0;
	}

	if ( reg > 0x7 && reg < 0x0c )
		ide_reg_write( ctrl, channel, ATA_REG_CONTROL, ctrl->channels[channel].no_int );

	return ret;
}

void ide_read_buffer( ide_control_t *ctrl, uint8_t channel, uint8_t reg, char *buffer, unsigned int quads ){

	if ( reg > 0x7 && reg < 0xc )
		ide_reg_write( ctrl, channel, ATA_REG_CONTROL, 0x80 | ctrl->channels[channel].no_int );

	asm volatile( "pushw %es; movw %ds, %ax; movw %ax, %es" );

	if 	( reg < 0x08 ) 	insl( ctrl->channels[channel].base + reg, buffer, quads );
	else if ( reg < 0x0c ) 	insl( ctrl->channels[channel].base + reg - 0x06, buffer, quads );
	else if ( reg < 0x0e ) 	insl( ctrl->channels[channel].ctrl + reg - 0x0a, buffer, quads );
	else if ( reg < 0x16 ) 	insl( ctrl->channels[channel].busmaster + reg - 0x0e, buffer, quads );
	else 			kprintf( "[ide_reg_read] Have invalid register \"0x%x\"\n", reg );

	asm volatile( "popw %es" );

	if ( reg > 0x7 && reg < 0x0c )
		ide_reg_write( ctrl, channel, ATA_REG_CONTROL, ctrl->channels[channel].no_int );
}

uint8_t ide_polling( ide_control_t *ctrl, uint8_t channel, uint8_t advanced_check ){
	int i;

	for ( i = 4; i; i-- )
		ide_reg_read( ctrl, channel, ATA_REG_ALT_STATUS );

	while ( ide_reg_read( ctrl, channel, ATA_REG_STATUS ) & ATA_STATUS_BUSY );

	return 0;
}

int ata_ide_hal_read( hal_device_t *dev, void *buf, unsigned count, unsigned offset ){
	ide_device_t *ide_dev;
	ide_control_t *ctrl;
	int ret = 0;
	int drive;

	if ( !dev )
		return 0;

	ide_dev = dev->dev;
	ctrl = ide_dev->ctrl;

	drive = ide_dev->drive + ide_dev->channel * 2;

	kprintf( "[%s] Got here, like a boss. reading drive %d, ctrl at 0x%x\n", __func__, drive, ctrl );
	ret = ata_ide_read_sectors( ctrl, drive, count, offset, 0x10, (unsigned)buf );

	//ret = count;
	return ret;
}

int ata_ide_hal_write( hal_device_t *dev, void *buf, unsigned count, unsigned offset ){
	ide_device_t *ide_dev;
	ide_control_t *ctrl;
	int ret = 0;

	if ( !dev )
		return 0;

	ide_dev = dev->dev;
	ctrl = ide_dev->ctrl;

	ata_ide_write_sectors( ctrl, ide_dev->drive, offset, count, 0x10, (unsigned)buf );

	ret = count;
	return ret;
}
