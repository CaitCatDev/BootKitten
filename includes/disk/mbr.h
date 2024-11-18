#pragma

#include <stdint.h>

typedef struct _mbr_part_s {
	uint8_t attributes;
	uint8_t start_head;
	uint8_t start_cylinder;
	uint8_t start_sector; /*Upper 6..7 bits are upper bits of cylinder*/
	uint8_t type;
	uint8_t end_head;
	uint8_t end_cylinder;
	uint8_t end_sector; /*Upper two bits are upper bits of cylinder*/
	uint32_t lba_start;
	uint32_t sectors;
} __attribute__((packed)) mbr_part_t;

typedef struct _mbr_s {
	uint8_t code[440];
	uint32_t diskid;
	uint16_t reserved;
	mbr_part_t parts[4];
	uint16_t magic;
} __attribute__((packed)) mbr_t;
