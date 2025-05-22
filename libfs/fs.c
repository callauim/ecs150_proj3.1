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

struct __attribute__((packed)) superblock {
	uint8_t signature[8];      /* Signature Length: 8*/
	uint16_t total_blocks;     /* Total amount of blocks of virtual disk Length: 2*/
	uint16_t root_dir_index;   /* Root directory block index Length: 2*/
	uint16_t data_start_index; /* Data block start index Length: 2*/
	uint16_t data_block_count; /* Amount of data blocks Length: 2*/
	uint8_t fat_block_count;   /* Number of blocks for FAT Length: 1*/
	uint8_t padding[4079];     /* Unused/Padding Length: 4079*/
};

struct __attribute__((packed)) fat_entry {
    uint16_t value;  /* 16 bit word Length: 2*/
};

struct __attribute__((packed)) root_entry {
	uint8_t filename[FS_FILENAME_LEN]; /* Filename (including NULL character) Length: 16*/
	uint32_t file_size;                /* Size of the file (in bytes) Length: 4*/
	uint16_t first_data_block;         /* Index of the first data block Length: 2*/
	uint8_t padding[10];               /* Unused/Padding Length: 10*/
};

int fs_mount(const char *diskname)
{
	/* 
	Don’t forget that your function fs_mount() should perform some error checking to verify that the file system has the expected format.
	For example, the signature of the file system should correspond to the one defined by the specifications, the total amount of blocks should correspond to what block_disk_count() returns, etc.
	You need to open the virtual disk, using the block API
	, and load the meta-information that is necessary to handle the file system operations.
	*/

	return 0;
}

int fs_umount(void)
{
	/* fs_umount() makes sure that the virtual disk is properly closed
	and that all the internal data structures of the FS layer are properly cleaned. 
	*/
	return 0;
}

int fs_info(void)
{
	/* Once you’re able to mount a file system, you can implement the function fs_info() 
	which prints some information about the mounted file system and make sure that the output corresponds exactly to the reference program. */
	return 0;
}
