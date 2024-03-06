#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define ERROR -1
#define SUCCE 0

#define FAT_EOC 0xFFFF

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

uint16_t *fat;
struct superblock super;
struct rootDirectory rootDir[FS_FILE_MAX_COUNT];

int fs_mount(const char *diskname)
{
	/* TODO: Phase 1 */
	struct root_dir *rootDir = malloc(sizeof(struct rootDirectory) * FS_FILE_MAX_COUNT);
	fat = malloc(BLOCK_SIZE *(super.num_blocks_fat)* 2);
	if(block_disk_open(diskname) == ERROR)
	{
		return ERROR;
	}
	if(strcmp(super.signature, "ECS150FS") != 0)
	{
		block_disk_close();
		return ERROR;
	}
	if(block_read(0, &super) == ERROR)
	{
		block_disk_close();
		return ERROR;
	}
	if(super.total_blocks != block_disk_count())
	{
		block_disk_close();
		return ERROR;
	}
	if (rootDir == NULL) 
	{
		block_disk_close();
		return ERROR;
	}
	if (block_read(super.root_dir_index, rootDir) == ERROR) 
	{
		block_disk_close();
		return ERROR;
    }
	for(int i = 1; i < super.num_blocks_fat; i++) 
	{
		if(block_read(i+1, &fat[i*BLOCK_SIZE/2]))
		{
			return ERROR;
		}
	}
	return SUCCE;
}
int fs_umount(void)
{
	/* TODO: Phase 1 */
	if(block_disk_count() == ERROR)
	{
		return ERROR;
	}
	if(block_disk_close() == ERROR)
	{
        return ERROR;
    }
	fat = malloc(BLOCK_SIZE *(super.num_blocks_fat)* 2);
	block_write(super.root_dir_index, fat);
	for(int i = 1; i < super.num_blocks_fat; i++) 
	{
		if(block_write(i+1, &fat[i*(BLOCK_SIZE/2)]))
		{
			return ERROR;
		}
	}
	return SUCCE;
}

int fs_info(void)
{
	/* TODO: Phase 1 */
	printf("Currently Mounted File System Information:\n");
    printf("  Signature: %s\n", super.signature);
    printf("  Total blocks: %u\n", super.total_blocks);
    printf("  Root Directory Index: %u\n", super.root_dir_index);
    printf("  Data Block Start Index: %u\n", super.data_block_start_index);
    printf("  Number of Fat Blocks: %u\n", super.num_blocks_fat);
    printf("  Number of Data Blocks: %u\n", super.amount_data_blocks);
	return SUCCE;
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	if(filename == NULL || strlen(filename) >= FS_FILENAME_LEN)
	{
		return ERROR;
	}
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
	{
        if (strcmp(rootDir[i].filename, filename) == 0) 
		{
            return ERROR; 
        }
    }
	int empty_entry = ERROR;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
	{
        if (rootDir[i].filename[0] == '\0') 
		{
            empty_entry = i;
            break;
        }
    }
	if (empty_entry == ERROR) 
	{
        return ERROR;
    }
	memcpy(rootDir[empty_entry].filename, filename, FS_FILENAME_LEN);
	rootDir[empty_entry].size_of_file = 0; 
    rootDir[empty_entry].index_of_first = FAT_EOC; 
	return SUCCE;

}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
	if (filename == NULL) 
    {
        return ERROR;
    }
    int file_index = ERROR;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
    {
        if (strcmp(rootDir[i].filename, filename) == 0) 
        {
            file_index = i;
            break;
        }
    }
    if (file_index == ERROR) 
    {
        return ERROR;
    }

    uint16_t current_block = rootDir[file_index].index_of_first;
    while (current_block != FAT_EOC) 
    {
        uint16_t next_block = fat[current_block];
        fat[current_block] = 0;
        current_block = next_block;
    }

    rootDir[file_index].filename[0] = '\0';
    rootDir[file_index].size_of_file = 0;
    rootDir[file_index].index_of_first = FAT_EOC;

    return SUCCE;
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
	if(block_disk_count() == ERROR) 
	{
		return ERROR;
	}
	printf("FS Ls:\n");
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
    {
        if (rootDir[i].filename[0] != '\0') 
        {
            printf("File: %s, Size: %u, Blocks: ", rootDir[i].filename, rootDir[i].size_of_file);
            uint16_t current_block = rootDir[i].index_of_first;
            while (current_block != FAT_EOC) 
            {
                printf("%u ", current_block);
                current_block = fat[current_block];
            }
            printf("\n");
        }
    }
    return SUCCE;
}

int fs_open(const char *filename)
{
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
