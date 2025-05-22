#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

// End of chain marker
#define FAT_EOC 0xFFFF
// Filesystem signature
#define FS_SIGNATURE "ECS150FS"

// Data Structs 
struct __attribute__((packed)) superblock {
	uint8_t signature[8];
	uint16_t total_blocks;
	uint16_t root_dir_index;
	uint16_t data_start_index;
	uint16_t data_block_count;
	uint8_t fat_block_count;
	uint8_t padding[4079];
};

struct __attribute__((packed)) root_entry {
	uint8_t filename[FS_FILENAME_LEN];
	uint32_t file_size;
	uint16_t first_data_block;
	uint8_t padding[10];
};

// Global Vars
static struct superblock sb;
static uint16_t *fat = NULL;
static struct root_entry root_dir[FS_FILE_MAX_COUNT];
static int mounted = 0;

int fs_mount(const char *diskname)
{
	uint8_t block[BLOCK_SIZE];
	size_t fat_size_bytes;

	// Check if already mounted
	if (mounted) {
		return -1;
	}

	// Open the disk
	if (block_disk_open(diskname) < 0) {
		return -1;
	}

	// Read the superblock
	if (block_read(0, block) < 0) {
		block_disk_close();
		return -1;
	}
	memcpy(&sb, block, sizeof(sb));

	// Verify filesystem signature
	if (memcmp(sb.signature, FS_SIGNATURE, 8) != 0) {
    	block_disk_close();
    	return -1;
	}

	// Verify total block count matches disk
	int disk_block_count = block_disk_count();
	if (disk_block_count < 0 || sb.total_blocks != (uint16_t)disk_block_count) {
    	block_disk_close();
    	return -1;
	}

	// Validate superblock values to prevent buffer overflows
	if (sb.data_block_count == 0 || sb.fat_block_count == 0) {
		block_disk_close();
		return -1;
	}

	// Calculate expected FAT size and validate
	fat_size_bytes = sb.data_block_count * sizeof(uint16_t);
	size_t expected_fat_blocks = (fat_size_bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
	if (sb.fat_block_count != expected_fat_blocks) {
		block_disk_close();
		return -1;
	}

	// Allocate memory for FAT - allocate full blocks worth to avoid partial block issues
	size_t fat_alloc_size = sb.fat_block_count * BLOCK_SIZE;
	fat = malloc(fat_alloc_size);
	if (!fat) {
		block_disk_close();
    	return -1;
	}

	// Initialize FAT memory to zero
	memset(fat, 0, fat_alloc_size);

	// Read FAT blocks into our FAT array
	for (int i = 0; i < sb.fat_block_count; i++) {
		if (block_read(1 + i, block) < 0) {
			free(fat);
			fat = NULL;
			block_disk_close();
			return -1;
		}
		// Copy the block data to the appropriate position in the FAT array
		memcpy((uint8_t*)fat + i * BLOCK_SIZE, block, BLOCK_SIZE);
	}

	// Validate that FAT[0] is FAT_EOC as per specification
	if (fat[0] != FAT_EOC) {
		free(fat);
		fat = NULL;
		block_disk_close();
		return -1;
	}

	// Read root directory
	if (block_read(sb.root_dir_index, block) < 0) {
		free(fat);
		fat = NULL;
		block_disk_close();
		return -1;
	}
	memcpy(root_dir, block, sizeof(root_dir));

	mounted = 1;
	return 0;
}

int fs_umount(void)
{
	if (!mounted) {
		return -1;
	}
	
	if (block_disk_close() < 0) {
		return -1;
	}

	if (fat) {
		free(fat);
		fat = NULL;
	}
	
	memset(&sb, 0, sizeof(sb));
	memset(root_dir, 0, sizeof(root_dir));
	mounted = 0;
	
	return 0;
}

int fs_info(void)
{
	if (!mounted) {
		return -1;
	}

	// Count free FAT entries (entries with value 0, excluding entry 0 which is always FAT_EOC)
	int fat_free_count = 0;
	for (int i = 1; i < sb.data_block_count; i++) {
		if (fat[i] == 0) {
			fat_free_count++;
		}
	}

	// Count free root directory entries (entries where first character of filename is NULL)
	int rdir_free_count = 0;
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
		if (root_dir[i].filename[0] == '\0') {
			rdir_free_count++;
		}
	}

	// Print filesystem information
	printf("FS Info:\n");
	printf("total_blk_count=%d\n", sb.total_blocks);
	printf("fat_blk_count=%d\n", sb.fat_block_count);
	printf("rdir_blk=%d\n", sb.root_dir_index);
	printf("data_blk=%d\n", sb.data_start_index);
	printf("data_blk_count=%d\n", sb.data_block_count);
	printf("fat_free_ratio=%d/%d\n", fat_free_count, sb.data_block_count);
	printf("rdir_free_ratio=%d/%d\n", rdir_free_count, FS_FILE_MAX_COUNT);

	return 0;
}