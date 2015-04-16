#ifndef _helix_vbe_module_h
#define _helix_vbe_module_h
#include <base/stdint.h>

typedef struct vbe_info_block {
    uint8_t  vbe_sig[4];
    uint16_t version;
    uint16_t oem_str_ptr[2];
    uint8_t  capabilities[4];
    uint16_t mode_ptr[2];
    uint16_t total_mem;
} __attribute__((packed)) vbe_info_block_t;

typedef struct vbe_mode_block {
    uint16_t attributes;
    uint8_t  winA, winB;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segmentA, segmentB;
    uint16_t real_fct_ptr[2];
    uint16_t pitch;

    uint16_t Xres, Yres;
    uint8_t  w_char, y_char, planes, bpp, banks;
    uint8_t  mem_model, bank_size, image_pages;
    uint8_t  reserved0;

    uint8_t  red_mask, red_position;
    uint8_t  green_mask, green_position;
    uint8_t  blue_mask, blue_position;
    uint8_t  rsv_mask, rsv_position;
    uint8_t  direct_color_attributes;

    uint32_t physbase;
    uint32_t reserved1;
    uint16_t reserved2;
} __attribute__((packed)) vbe_mode_block_t;

typedef struct psf2_header {
    uint8_t  magic[4];
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t length;
    uint32_t charsize;
    uint32_t height;
    uint32_t width;
} __attribute__((packed)) psf2_header;

typedef struct vbe_device {
    vbe_mode_block_t *mode;
    vbe_info_block_t *info;
    uint8_t *framebuf;
    unsigned x_res, y_res, bpp, pitch;
} vbe_device_t;

#endif
