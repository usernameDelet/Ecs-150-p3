#include <assert.h>
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

typedef uint16_t fat_entry;

struct rootDirectory
{
	char 		filename[FS_FILENAME_LEN];
	uint32_t    size_of_file;
	uint16_t 	index_of_first;
	uint8_t 	unused[10];
}__attribute__((packed));;

struct file_descriptor{
	char filename[16];
	size_t offset;
	uint16_t  file_index;
};

struct superblock* super = NULL;
struct rootDirectory* rootDir = NULL;
fat_entry* fat = NULL;
struct file_descriptor* filed = NULL;


int fs_mount(const char *diskname)
{
	
	
	super = malloc(sizeof(struct superblock));
	rootDir = malloc(sizeof(struct rootDirectory) * FS_FILE_MAX_COUNT);
	

	if(block_disk_open(diskname) == ERROR){
		free(super);
		return ERROR;
	}

	if(block_read(0, super) == ERROR){
	
		free(super);
		return ERROR;
	}
	if (strncmp(super->signature, "ECS150FS", 8) != 0)
    {
        return ERROR;
    }

	if(super->total_blocks != block_disk_count())
    {
        return ERROR;
    }

	if (super->num_blocks_fat+1 != super->root_dir_index){
		free(super);
		return ERROR;	
	}
    filed = malloc(sizeof(struct file_descriptor) * FS_OPEN_MAX_COUNT);
    fat = malloc(sizeof(uint16_t) * super->num_blocks_fat * BLOCK_SIZE);
    for(int i = 0; i < super->num_blocks_fat; i++) 
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
	int free_fat = 0;
	int free_rd = 0;
	if(super == NULL){
		//perror("no virtual disk opened");
		return ERROR;
	}
	printf("%s \n", "FS Info:");
	printf("total_blk_count=%u\n",super->total_blocks);
    printf("fat_blk_count=%u\n", super->num_blocks_fat);
    printf("rdir_blk=%u\n", super->root_dir_index);
    printf("data_blk=%u\n", super->data_block_start_index);
    printf("data_blk_count=%u\n", super->amount_data_blocks);

	
	for(int i=0; i<super->amount_data_blocks; i++){
		if(fat[i] == 0){
			free_fat++;
		}
	}
	printf("fat_free_ratio=%d/%d\n", free_fat, super->amount_data_blocks);

	for(int i=0; i<FS_FILE_MAX_COUNT; i++){
		if(rootDir[i].filename[0] == '\0'){
			free_rd++;
		}
	}
	printf("rdir_free_ratio=%d/%d\n", free_rd, FS_FILE_MAX_COUNT);

	return SUCCE;
}
int fs_create(const char *filename)
{
	
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

    if (block_write(super->root_dir_index, rootDir) == ERROR)
    {
        return ERROR;
    }
    printf("he 3");
	return SUCCE;

}

int fs_delete(const char *filename)
{
	
	if (filename == NULL) 
    {
        printf("he 5");
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
        printf("he 6");
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
	if(block_disk_count() == ERROR) 
	{
        printf("he 9");
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
    printf("he 10");
    return SUCCE;
}

int fs_open(const char *filename)
{
	if (filename == NULL || strlen(filename) >= FS_FILENAME_LEN)
    {
        return ERROR;
    }

    int index = ERROR;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
    {
        if (strcmp(rootDir[i].filename, filename) == 0) 
        {
            index = i; 
            break;
        }
    }

    if (index == ERROR) 
    {
        return ERROR; 
    }

    
    if (index < 0 || index >= FS_OPEN_MAX_COUNT) 
    {

        return ERROR;
    }

    strcpy(filed[index].filename, filename); 
    filed[index].offset = 0;
    filed[index].file_index = index;

    printf("here 1\n");
    return SUCCE; 

}

int fs_close(int fd)
{
	if (fd >= FS_OPEN_MAX_COUNT || fd < 0)
    {
        return ERROR;
    }
    if (filed[fd].filename[0] == '\0')
    {
        return ERROR;
    }

	filed[fd].filename[0] = '\0'; 
    filed[fd].offset = 0;           
    filed[fd].file_index = ERROR;      

    return SUCCE;
}

int fs_stat(int fd) 
{
    if (super == NULL || fd < 0 || fd >= FS_OPEN_MAX_COUNT || filed[fd].filename[0] == '\0') {
        return ERROR; 
    }
    
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) {
        if (strcmp(rootDir[i].filename, filed[fd].filename) == 0) {
            return rootDir[i].size_of_file; 
        }
    }

    return ERROR; 
}



int fs_lseek(int fd, size_t offset)
{
    if (super == NULL || fd < 0 || fd >= FS_OPEN_MAX_COUNT || filed[fd].filename[0] == '\0')
    {
        return ERROR; 
    }
    
    int file_size = fs_stat(fd);
    if (file_size == -1 || offset > (size_t)file_size)
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
    for (uint16_t i = super->data_block_start_index; i < super->amount_data_blocks; i++) 
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
    if (super->signature[0] == '\0' || 
        fd < 0 || 
        fd >= FS_OPEN_MAX_COUNT || 
        filed[fd].filename[0] == '\0' ||  // Check if the file descriptor is valid
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
                break;  // Exit loop if unable to allocate block
            }
            fat[block_index] = current_block;  // Update FAT
            fat[current_block] = FAT_EOC;
        }

        if (block_read(current_block, block_buffer) == ERROR) 
        {
            return ERROR;  // Return error if unable to read block
        }

        memcpy(block_buffer + block_offset, buf + bytes_written, bytes_to_write);

        if (block_write(current_block, block_buffer) == ERROR) 
        {
            return ERROR;  // Return error if unable to write block
        }

        bytes_written += bytes_to_write;
        offset += bytes_to_write;
        block_index = fat[block_index];
    }

    if (offset > rootDir[file_idx].size_of_file) 
    {
        rootDir[file_idx].size_of_file = offset;  // Update file size if necessary
    }

    return bytes_written;
}

int fs_read(int fd, void *buf, size_t count)
{
    if (fd < 0 || 
		fd >= FS_OPEN_MAX_COUNT || 
		filed[fd].filename[0] == '\0' ||  
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
