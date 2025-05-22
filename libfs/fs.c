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

struct __attribute__((packed)) fat_entry {
    uint16_t value; 
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
	/* 
	Don’t forget that your function fs_mount() should perform some error checking to verify that the file system has the expected format.
	For example, the signature of the file system should correspond to the one defined by the specifications, the total amount of blocks should correspond to what block_disk_count() returns, etc.
	You need to open the virtual disk, using the block API
	, and load the meta-information that is necessary to handle the file system operations.
	*/

	uint8_t block[BLOCK_SIZE];

	// Check if disk file cannot be opened or found or is already mounted
	if (mounted || block_disk_open(diskname) < 0) {
		return -1;
	}

	// Read the superblock and copy to global var
	if (block_read(0, block) < 0) {
		block_disk_close();
		return -1;
	}
	memcpy(&sb, block, sizeof(sb));

	// Verify filesystem using superblock
	if (memcmp(sb.signature, FS_SIGNATURE, 8) != 0 || sb.total_blocks != block_disk_count()) {
    	block_disk_close();
    	return -1;
	}

	// Allocate memory for FAT
	fat = malloc(sb.data_block_count * sizeof(uint16_t));
	if (!fat) {
		block_disk_close();
    	return -1;
	}

	// Read FAT blocks into our FAT array
	for (int i = 0; i < sb.fat_block_count; i++) {
		if (block_read(1 + i, block) < 0) {
			free(fat);
			fat = NULL;
			block_disk_close();
			return -1;
		}
		memcpy((uint8_t*)fat + i * BLOCK_SIZE, block, BLOCK_SIZE);
	}

	// Read root directory and copy to our root entry array
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
	if (!mounted){
		return -1;
	}
	
	if (block_disk_close() < 0) {
		return -1;
	}

	free(fat);
	fat = NULL;
	mounted = 0;
	return 0;
}

int fs_info(void)
{
	/* Once you’re able to mount a file system, you can implement the function fs_info() 
	which prints some information about the mounted file system and make sure that the output corresponds exactly to the reference program. */
	return 0;
}
