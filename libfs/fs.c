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
}__attribute__((packed));;

struct rootDirectory
{
	char 		filename[FS_FILENAME_LEN];
	uint32_t    size_of_file;
	uint16_t 	index_of_first;
	uint8_t 	unused[10];
};

struct file_descriptor
{
	char  filename[FS_FILENAME_LEN];
	int   file_index;
	int   offset;
};

uint16_t *fat;
struct superblock super;
struct rootDirectory rootDir[FS_FILE_MAX_COUNT];
struct file_descriptor filed[FS_OPEN_MAX_COUNT];

int fs_mount(const char *diskname)
{
    /* TODO: Phase 1 */
    if(block_disk_open(diskname) == ERROR)
    {
        return ERROR;
    }
    if(block_read(0, &super) == ERROR)
    {
        return ERROR;
    }
    if (strncmp(super.signature, "ECS150FS", 8) != 0)
    {
        return ERROR;
    }
    if(super.total_blocks != block_disk_count())
    {
        return ERROR;
    }
    if (block_read(super.root_dir_index, (void *)rootDir) == ERROR) 
    {
        return ERROR;
    }
    fat = malloc(sizeof(uint16_t) * super.num_blocks_fat * BLOCK_SIZE);
    for(int i = 0; i < super.num_blocks_fat; i++) 
    {
        if(block_read(i+1, &fat[i]) == ERROR)
        {
            return ERROR;
        }
    }
    return SUCCE;
}

int fs_umount(void)
{
    printf("h12");
	/* TODO: Phase 1 */
	if(block_disk_count() == ERROR)
	{
		return ERROR;
	}
	if(block_disk_close() == ERROR)
	{
        return ERROR;
    }

    free(fat);
    
	return SUCCE;
}

int fs_info(void)
{
    /* TODO: Phase 1 */
    if(block_disk_count() == ERROR)
    {
        return ERROR;
    }

	printf("FS Info:\n");
    printf("total_blk_count=%u\n", super.total_blocks);
    printf("fat_blk_count=%u\n", super.num_blocks_fat);
    printf("rdir_blk=%u\n", super.root_dir_index);
    printf("data_blk=%u\n", super.data_block_start_index);
    printf("data_blk_count=%u\n", super.amount_data_blocks);

    int fat_free_blocks = 0;
    for (int i = 0; i < super.amount_data_blocks; i++) 
    {
        if (fat[i] == 0) 
        {
            fat_free_blocks++;
        }
    }
    printf("fat_free_ratio=%d/%u\n", fat_free_blocks, super.amount_data_blocks);

    int rdirCount = 0;
    for(int i = 0; i < FS_FILE_MAX_COUNT; i++)
    {
        if(strcmp(rootDir[i].filename, "\0") == 0)
        {
            rdirCount = rdirCount + 1;
        }
    }
    printf("rdir_free_ratio=%d/%d\n", rdirCount, FS_FILE_MAX_COUNT);
    
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
        if (strcmp(rootDir[i].filename, "") == 0) 
		{
            empty_entry = i;
            break;
        }
    }
	if (empty_entry == ERROR) 
	{
        return ERROR;
    }

	memcpy(rootDir[empty_entry].filename, filename, strlen(filename));
	rootDir[empty_entry].size_of_file = 0; 
    rootDir[empty_entry].index_of_first = FAT_EOC;
    block_write(super.root_dir_index, rootDir);
	return SUCCE;

}

int fs_delete(const char *filename)
{
    printf("h 8");
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
    printf("h 7");
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

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	if(filename == NULL || strlen(filename) >= FS_FILENAME_LEN)
	{
        printf("he 1");
		return ERROR;
	}
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
	{
        if (strcmp(rootDir[i].filename, filename) == 0) 
		{
            printf("he 2");
            return ERROR; 
        }
    }
	int empty_entry = ERROR;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
	{
        if (strcmp(rootDir[i].filename, "") == 0) 
		{
            empty_entry = i;
            break;
        }
    }
	if (empty_entry == ERROR) 
	{
        printf("he 4");
        return ERROR;
    }

	memset(rootDir[empty_entry].filename, 0, FS_FILENAME_LEN); // Clear filename buffer
    strncpy(rootDir[empty_entry].filename, filename, FS_FILENAME_LEN - 1);
    rootDir[empty_entry].size_of_file = 0; 
    rootDir[empty_entry].index_of_first = FAT_EOC;

    if (block_write(super.root_dir_index, rootDir) == ERROR)
    {
        return ERROR;
    }
    printf("he 3");
    return SUCCE;

}

int fs_close(int fd)
{
    printf("h 5");
	/* TODO: Phase 3 */
    if (fd >= FS_OPEN_MAX_COUNT || fd < 0)
    {
        return ERROR;
    }
    if (filed[fd].file_index == ERROR) 
    {
        return ERROR;
    }
    filed[fd].filename[0] = '\0';
    filed[fd].file_index = ERROR;
    filed[fd].offset = 0;
    return SUCCE;
}


int fs_stat(int fd)
{
    printf("h 4");
	/* TODO: Phase 3 */
	if (super.signature[0] == '\0')
    {
        return ERROR;
    }
    
    if (fd < 0 || fd >= FS_OPEN_MAX_COUNT || filed[fd].file_index == ERROR)
    {
        return ERROR;
    }

    int block_count = block_disk_count();
    if (block_count == ERROR)
    {
        return ERROR;
    }

    int file_block_count = (rootDir[filed[fd].file_index].size_of_file + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (file_block_count > block_count)
    {
        return ERROR;
    }

    return rootDir[filed[fd].file_index].size_of_file;
}

int fs_lseek(int fd, size_t offset)
{
    printf("h 3");
    if (fd >= FS_OPEN_MAX_COUNT || fd < 0)
    {
        return ERROR;
    }
    if (filed[fd].file_index == ERROR) 
    {
        return ERROR;
    }
    if (offset > rootDir[filed[fd].file_index].size_of_file)
    {
        return ERROR;
    }
    filed[fd].offset = offset;
    return SUCCE;
}

uint16_t data_block_index(int file_index, size_t offset) 
{
    uint16_t block_index = rootDir[file_index].index_of_first;
    while (offset >= BLOCK_SIZE && block_index != FAT_EOC) 
    {
        block_index = fat[block_index];
        offset -= BLOCK_SIZE;
    }
    return block_index;
}

uint16_t allocate_block() {
    for (uint16_t i = super.data_block_start_index; i < super.amount_data_blocks; i++) 
    {
        if (fat[i] == 0) 
        {
            return i;
        }
    }
    return FAT_EOC;  
}

size_t min(size_t a, size_t b) 
{
    return a < b ? a : b;
}



int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
    printf("h 1");
    if (super.signature[0] == '\0' || 
                            fd < 0 || 
                            fd >= FS_OPEN_MAX_COUNT || 
                            filed[fd].file_index == ERROR || 
                            buf == NULL) 
    {
        return ERROR;
    }
    int file_idx = filed[fd].file_index;
    size_t offset = filed[fd].offset;
    uint16_t block_index = data_block_index(file_idx, offset);

    size_t bytes_written = 0;

    while (bytes_written < count) 
    {
        uint16_t current_block = block_index;
        int block_offset = offset % BLOCK_SIZE;
        int bytes_to_write = min(count - bytes_written, BLOCK_SIZE - block_offset);
        uint8_t block_buffer[BLOCK_SIZE];

        if (current_block == FAT_EOC) 
        {
          
            current_block = allocate_block();
            if (current_block == FAT_EOC) 
            {
                
                break;
            }
            fat[block_index] = current_block;
            fat[current_block] = FAT_EOC;
        }

      
        if (block_read(current_block, block_buffer) == ERROR) 
        {
            return ERROR;
        }

    
        memcpy(block_buffer + block_offset, buf + bytes_written, bytes_to_write);

      
        if (block_write(current_block, block_buffer) == ERROR) 
        {
            return ERROR;
        }

        bytes_written += bytes_to_write;
        offset += bytes_to_write;
        block_index = fat[block_index];
    }

   
    if (offset > rootDir[file_idx].size_of_file) 
    {
        rootDir[file_idx].size_of_file = offset;
    }

    return bytes_written;
}

int fs_read(int fd, void *buf, size_t count)
{
    printf("h 2");
    if (fd < 0 || 
		fd >= FS_OPEN_MAX_COUNT || 
		filed[fd].file_index == ERROR ||  
		block_disk_count() == ERROR ||
		buf == NULL )
    {
        return ERROR;
    }

    // Calculate the block index and offset within the block
    uint16_t block_index = filed[fd].offset / BLOCK_SIZE;
    uint16_t block_offset = filed[fd].offset % BLOCK_SIZE;

    // Read data from the file
    while (count > 0 && (uint32_t)filed[fd].offset < rootDir[filed[fd].file_index].size_of_file)
    {
        // Read the block from disk
        char block[BLOCK_SIZE];
        if (block_read(fat[rootDir[filed[fd].file_index].index_of_first + block_index], block) == ERROR)
        {
            return ERROR;
        }

        // Calculate the number of bytes to read in the current block
        size_t bytes_to_read;
        if (count < (size_t)(BLOCK_SIZE - block_offset))
        {
            bytes_to_read = count;
        }
        else
        {
            bytes_to_read = BLOCK_SIZE - block_offset;
        }
        // Copy data from the block
        memcpy(buf, block + block_offset, bytes_to_read);

        // Update file offset and counters
        filed[fd].offset += bytes_to_read;
        buf += bytes_to_read;
        count -= bytes_to_read;

        // Move to the next block
        ++block_index;
        block_offset = 0;
    }

    return SUCCE;
}
