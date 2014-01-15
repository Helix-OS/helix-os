#ifndef _helix_ata_h
#define _helix_ata_h
#include <base/stdint.h>
#include <base/stdio.h>
#include <base/arch/i386/isr.h>
#include <base/arch/i386/pitimer.h>
#include <base/hal.h>
#include <base/logger.h>
#include <pci/pci.h>

enum {
	ATA_MODE_NULL,
	ATA_MODE_PIO,
};

enum {
	ATA_TYPE_NULL,
	ATA_TYPE_IDE,
	ATAPI_TYPE_IDE
};

// status codes
enum {
	ATA_STATUS_ERROR 	= 0x01,
	ATA_STATUS_IDX 		= 0x02,
	ATA_STATUS_CORR 	= 0x04,
	ATA_STATUS_DRQ 		= 0x08,
	ATA_STATUS_DSC 		= 0x10,
	ATA_STATUS_DF 		= 0x20,
	ATA_STATUS_DRV_READY 	= 0x40,
	ATA_STATUS_BUSY 	= 0x80
};

// Error codes
enum {
	ATA_ERROR_AMNF 		= 0x01,
	ATA_ERROR_TKONF 	= 0x02,
	ATA_ERROR_ABORT 	= 0x04,
	ATA_ERROR_MCR 		= 0x08,
	ATA_ERROR_IDNF 		= 0x10,
	ATA_ERROR_MC 		= 0x20,
	ATA_ERROR_UNC 		= 0x40,
	ATA_ERROR_BBK 		= 0x80
};

// Command codes
enum {
	ATA_CMD_READ_PIO 	= 0x20,
	ATA_CMD_READ_PIO_EXT 	= 0x24,
	ATA_CMD_READ_DMA 	= 0xc8,
	ATA_CMD_READ_DMA_EXT 	= 0x25,

	ATA_CMD_WRITE_PIO 	= 0x30,
	ATA_CMD_WRITE_PIO_EXT 	= 0x34,
	ATA_CMD_WRITE_DMA 	= 0xca,
	ATA_CMD_WRITE_DMA_EXT 	= 0x35,

	ATA_CMD_CACHE_FLUSH 	= 0xe7,
	ATA_CMD_CACHE_FLUSH_EXT = 0xea,
	ATA_CMD_PACKET 		= 0xa0,
	ATA_CMD_IDENT_PACKET 	= 0xa1,
	ATA_CMD_IDENT 		= 0xec
};

// ATAPI command codes
enum {
	ATAPI_CMD_READ 	= 0xa8,
	ATAPI_CMD_EJECT = 0x1b
};

// Identification
enum {
	ATA_IDENT_DEVICE_TYPE 	= 0x00,
	ATA_IDENT_CYLINDERS 	= 0x02,
	ATA_IDENT_HEADS 	= 0x06,
	ATA_IDENT_SECTORS 	= 0x0c,
	ATA_IDENT_SERIAL 	= 0x14,
	ATA_IDENT_MODEL 	= 0x36,
	ATA_IDENT_CAPABILITIES 	= 0x62,
	ATA_IDENT_FIELD_VALID 	= 0x6a,
	ATA_IDENT_MAX_LBA 	= 0x80,
	ATA_IDENT_COMMAND_SETS 	= 0xa4,
	ATA_IDENT_MAX_LBA_EXT 	= 0xc8
};

// Master and slave selection
enum {
	ATA_MASTER,
	ATA_SLAVE
};

// ATA registers
enum {
	ATA_REG_DATA 		= 0x00,
	ATA_REG_ERROR 		= 0x01,
	ATA_REG_FEATURES 	= 0x01,
	ATA_REG_SEC_COUNT0 	= 0x02,
	ATA_REG_LBA0 		= 0x03,
	ATA_REG_LBA1 		= 0x04,
	ATA_REG_LBA2 		= 0x05,
	ATA_REG_HD_DEV_SELECT 	= 0x06,
	ATA_REG_COMMAND 	= 0x07,
	ATA_REG_STATUS 		= 0x07,
	ATA_REG_SEC_COUNT1 	= 0x08,
	ATA_REG_LBA3 		= 0x09,
	ATA_REG_LBA4 		= 0x0a,
	ATA_REG_LBA5 		= 0x0b,
	ATA_REG_CONTROL 	= 0x0c,
	ATA_REG_ALT_STATUS 	= 0x0c,
	ATA_REG_DEV_ADDR 	= 0x0d
};

enum {
	ATA_PRIMARY,
	ATA_SECONDARY
};

enum { 
	ATA_READ,
	ATA_WRITE
};

typedef struct ata_device {
	uint8_t type;
	uint8_t mode;
	uint8_t initialized;

	void *device_data;
	pci_device_t pci_device;
	struct ata_device *next;
} ata_device_t;

typedef struct ide_channel {
	uint16_t base;
	uint16_t ctrl;
	uint16_t busmaster;
	uint8_t  no_int;
} ide_channel_t;

struct ide_control;
typedef struct ide_device {
	uint8_t  reserved;
	uint8_t  channel;
	uint8_t  drive;
	uint16_t type;
	uint16_t sign;
	uint16_t capabilities;
	uint32_t commands;
	uint32_t size;
	uint8_t  model[41];

	hal_device_t *hal_buf;
	struct ide_control *ctrl;
} ide_device_t;

typedef struct ide_control {
	ide_channel_t channels[2];
	ide_device_t devices[4];
} ide_control_t;

void ata_initialize_ide( ata_device_t *device );

char ata_ide_pio_access( ide_control_t *ctrl, char direction, uint8_t drive, unsigned lba,
			uint8_t numsects, uint16_t select, unsigned edi );
char ata_ide_read_sectors( ide_control_t *ctrl, uint8_t drive, uint8_t numsects,
		uint32_t lba, uint16_t es, uint32_t edi );
char ata_ide_write_sectors( ide_control_t *ctrl, uint8_t drive, uint8_t numsects,
		uint32_t lba, uint16_t es, uint32_t edi );

void ide_reg_write( ide_control_t *ctrl, uint8_t channel, uint8_t reg, uint8_t data );
uint8_t ide_reg_read( ide_control_t *ctrl, uint8_t channel, uint8_t reg );
void ide_read_buffer( ide_control_t *ctrl, uint8_t channel, uint8_t reg, char *buffer, unsigned int quads );

uint8_t ide_polling( ide_control_t *ctrl, uint8_t channel, uint8_t advanced_check );
void ide_wait_irq( );
void ide_irq_handler( );

int ata_ide_hal_read( hal_device_t *dev, void *buf, unsigned count, unsigned offset );
int ata_ide_hal_write( hal_device_t *dev, void *buf, unsigned count, unsigned offset );
#endif
