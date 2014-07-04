#ifndef _helix_net_rtl8139_h
#define _helix_net_rtl8139_h
#include <base/lib/stdbool.h>
#include <pci/pci.h>

typedef enum {
	RTL8139_MAC0_5		= 0,
	RTL8139_MAC0_7		= 8,
	RTL8139_RBSTART		= 0x30,
	RTL8139_CMD		= 0x37,
	RTL8139_IMR		= 0x3c,
	RTL8139_ISR		= 0x3e,
	RTL8139_CONFIG_1	= 0x52,
} rtl8139_port_t;

bool initialize_rtl8139( pci_device_t *device );

#endif
