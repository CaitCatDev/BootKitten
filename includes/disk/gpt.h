#pragma once

#include <stdint.h>

#define GPT_PARTITION_ATTR_REQUIRED (1)
#define GPT_PARTITION_ATTR_EFI_IGNO (1 << 1)
#define GPT_PARTITION_ATTR_BOOTABLE (1 << 2)

typedef struct _guid_s {
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];
} __attribute__((packed)) guid_t;

typedef struct _gpt_header_s {
	uint8_t magic[8];
	uint32_t revision;
	uint32_t header_size;
	uint32_t header_crc;
	uint32_t reserved;
	uint64_t header_lba;
	uint64_t backup_lba;
	uint64_t first_part_lba;
	uint64_t last_part_lba;
	guid_t guid;
	uint64_t part_table_lba;
	uint32_t part_count;
	uint32_t part_entry_sz;
	uint32_t part_crc;
	/*TODO how to load this cause can't read it directly from
	 * the disk as it's true size depends on the block size
	 * so usually 512 but could be more
	 */

} __attribute__((packed)) gpt_header_t;

typedef struct _gpt_part_s {
	guid_t type;
	guid_t unique_guid;
	uint64_t first_lba;
	uint64_t last_lba;
	uint64_t attributes;
	uint16_t utf16_name[36];
}__attribute__((packed)) gpt_part_t;
