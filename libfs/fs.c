#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

/* TODO: Phase 1 */
struct superblock 
{
    char 	 signature[8]; 
    uint16_t total_blocks;         
    uint16_t root_dir_index;       
    uint16_t data_block_start_index;
    uint16_t amount_data_blocks;  
    uint8_t  num_blocks_fat;       
    uint8_t  padding[4079];         
};

struct rootDirectory
{
	char 		filename[FS_FILENAME_LEN];
	uint32_t    size_of_file;
	uint16_t 	index_of_first;
	uint8_t 	unused[10];

};

struct superblock super;
struct rootDirectory rootDir[FS_FILE_MAX_COUNT];
int fs_mount(const char *diskname)
{
	/* TODO: Phase 1 */
	if(block_disk_open(diskname) == -1)
	{
		return -1;
	}
	if(strcmp(super.signature, "ECS150FS") != 0)
	{
		return -1;
	}
	if(block_read(0,&super) == -1)
	{
		return -1;
	}
	if(super.total_blocks != block_disk_count)
	{
		return -1;
	}
	struct root_dir *read = malloc(sizeof(struct rootDirectory) * FS_FILE_MAX_COUNT);
	if (read == NULL) 
	{
		return -1;
	}
	if (block_read(super.root_dir_index, read) == -1) 
	{
		return -1;
    }
	uint16_t *fat = malloc(BLOCK_SIZE *(super.num_blocks_fat)* 2);
	for(int i = 1; i < super.num_blocks_fat; i++) 
	{
		if(block_read(i+1, &fat[i*BLOCK_SIZE/2])){
			return -1;
	}

	
	}
	return 0;
}
int fs_umount(void)
{
	/* TODO: Phase 1 */
	if(block_disk_count() == -1)
	{
		return -1;
	}
	if(block_disk_close() == -1)
	{
        return -1;
    }
	uint16_t *fat = malloc(BLOCK_SIZE *(super.num_blocks_fat)* 2);
	block_write(super.root_dir_index, fat);
	for(int i = 1; i < super.num_blocks_fat; i++) 
	{
		if(block_write(i+1, &fat[i*(BLOCK_SIZE/2)])){
			return -1;
	}
	}
	return 0;
}

int fs_info(void)
{
	/* TODO: Phase 1 */
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
}

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
}

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
}

