#pragma once

#include <stdint.h>
#define MULTIBOOT2_MAGIC 0x36d76289

enum multiboot2_tag_types {
	MULTIBOOT2_END_TAG = 0,
	MULTIBOOT2_CMDLINE_TAG = 1,
	MULTIBOOT2_LDRNAME_TAG = 2,
	MULTIBOOT2_MODULES_TAG = 3,
	MULTIBOOT2_MEMINFO_TAG = 4,
	MULTIBOOT2_BOOTDEV_TAG = 5,
	MULTIBOOT2_MEMOMAP_TAG = 6,
	MULTIBOOT2_VBEINFO_TAG = 7,
	MULTIBOOT2_FBUINFO_TAG = 8,
	MULTIBOOT2_ELFSYMS_TAG = 9,
	MULTIBOOT2_APMINFO_TAG = 10,
	MULTIBOOT2_I386EFI_SYSTAB_TAG = 11,
	MULTIBOOT2_AMD64EFI_SYSTAB_TAG = 12,
	MULTIBOOT2_SMBIOS_TAG = 13,
	MULTIBOOT2_ACPI_RSDP_TAG = 14,
	MULTIBOOT2_ACPI_XSDP_TAG = 15,
	MULTIBOOT2_NETINFO_TAG = 16,
	MULTIBOOT2_EFIMMAP_TAG = 17,
	MULTIBOOT2_EFIBSNT_TAG = 18,
	MULTIBOOT2_EFI32_IMG_TAG = 19,
	MULTIBOOT2_EFI64_IMG_TAG = 20,
	MULTIBOOT2_IMGBASE_TAG = 21,
};

enum multiboot2_fb_type {
	MULTIBOOT2_FB_TYPE_PALETTE = 0,
	MULTIBOOT2_FB_TYPE_RGB = 1,
	MULTIBOOT2_FB_TYPE_TEXT = 2,
};

typedef uint32_t multiboot_phy_addr_t;

typedef struct multiboot2_tag {
	uint32_t type;
	uint32_t size;
} multiboot2_tag_t;

typedef struct multiboot2_cmdline {
	multiboot2_tag_t hdr;
	uint8_t str[];
} multiboot2_cmdline_t;

typedef struct multiboot2_loader_name {
	multiboot2_tag_t hdr;
	uint8_t str[];
} multiboot2_loader_name_t;

typedef struct multiboot2_modules {
	multiboot2_tag_t hdr;
	multiboot_phy_addr_t mod_start;
	multiboot_phy_addr_t mod_end;
	uint8_t str[];
} multiboot2_modules_t;

typedef struct multiboot2_meminfo {
	multiboot2_tag_t hdr;
	uint32_t lower; /*KB below 1MB*/
	uint32_t upper; /*KB above 1MB*/
} multiboot2_meminfo_t;

typedef struct multiboot2_bios_boot_dev {
	multiboot2_tag_t hdr;
	uint32_t dev; /*BIOS disk number*/
	uint32_t partition;
	uint32_t sub_part;
} multiboot2_bios_boot_dev_t;

typedef struct multiboot2_mmap_ent {
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
} multiboot2_mmap_ent_t;

typedef struct multiboot2_mmap {
	multiboot2_tag_t hdr;
	uint32_t entry_size;
	uint32_t entry_version;
	multiboot2_mmap_ent_t entries[];
} multiboot2_mmap_t;

typedef struct multiboot2_vbeinfo {
	multiboot2_tag_t hdr;
	uint16_t mode;
	uint16_t if_seg;
	uint16_t if_off;
	uint16_t if_len;
	uint8_t control_info[512];
	uint8_t mode_info[256];
} multiboot2_vbeinfo_t;

typedef struct multiboot2_color_desc {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} multiboot2_color_desc_t;

typedef struct multiboot2_fb_pallete_info {
	uint32_t num_colors;
	multiboot2_color_desc_t desc[];
} multiboot2_fb_pallete_info_t;

typedef struct multiboot2_fb_rgb_info {
	uint8_t r_pos, r_mask_sz;
	uint8_t g_pos, g_mask_sz;
	uint8_t b_pos, b_mask_sz;
} multiboot2_fb_rgb_info_t;

typedef struct multiboot2_fbinfo {
	multiboot2_tag_t hdr;
	uint64_t base;
	uint32_t pitch;
	uint32_t width;
	uint32_t height;
	uint8_t bpp;
	uint8_t type;
	uint8_t reserved;
	union {
		multiboot2_fb_pallete_info_t pallete;
		multiboot2_fb_rgb_info_t rgb;
	} color_info;	
} multiboot2_fbinfo_t;

typedef struct multiboot2_elfsym {
	multiboot2_tag_t hdr;
	uint16_t num;
	uint16_t entsize;
	uint16_t shndx;
	uint16_t reserved;
	uint8_t section_hdrs[]; /*TODO:*/
} multiboot2_elfsym_t;

typedef struct multiboot2_apm_table {
	multiboot2_tag_t hdr;
	uint16_t version;
	uint16_t cseg;
	uint32_t offset;
	uint16_t cseg_16;
	uint16_t dseg;
	uint16_t flags;
	uint16_t cseg_len;
	uint16_t cseg_16_len;
	uint16_t dseg_len;
} multiboot2_apm_table_t;

typedef struct multiboot2_i386_efi_sys_tab {
	multiboot2_tag_t hdr;
	uint32_t table_addr;
} multiboot2_i386_efi_sys_tab_t;

typedef struct multiboot2_amd64_efi_sys_tab {
	multiboot2_tag_t hdr;
	uint64_t table_addr;
} multiboot2_amd64_efi_sys_tab_t;

typedef struct multiboot2_smbios {
	multiboot2_tag_t hdr;
	uint32_t size;
	uint8_t major;
	uint8_t minor;
	uint8_t reserved[6];
	uint8_t smbios[]; /*TODO*/
} multiboot2_smbios_t;

typedef struct multiboot2_rsdp {
	multiboot2_tag_t hdr;
	uint8_t rsdp[]; /*TODO*/
} multiboot2_rsdp_t;

typedef struct multiboot2_xsdp {
	multiboot2_tag_t hdr;
	uint8_t xsdp[]; /*TODO*/
} multiboot2_xsdp_t;

typedef struct multiboot2_efi_mmap {
	multiboot2_tag_t hdr;
	uint32_t desc_size;
	uint32_t desc_version;
	uint8_t mmap[]; /*TODO*/
} multiboot2_efi_mmap;

/*Don't need a struct for boot services not exited*
 *Kernel can just set a bool
 */

typedef struct multiboot2_efi32_img {
	multiboot2_tag_t hdr;
	uint32_t pointer;
} multiboot2_efi32_img_t;

typedef struct multiboot2_efi64_img {
	multiboot2_tag_t hdr;
	uint64_t pointer;
} multiboot2_efi64_img_t;

typedef struct multiboot2_img {
	multiboot2_tag_t hdr;
	uint32_t base;
} multiboot2_img;


typedef struct multiboot2_boot_info {
	uint32_t size;
	uint32_t reserved;
	
	uint8_t tags[];
} multiboot2_boot_info_t;
