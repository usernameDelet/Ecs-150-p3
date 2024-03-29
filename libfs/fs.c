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
    // error checking
    if(block_disk_open(diskname) == ERROR)
    {
        // no disk opened
        return ERROR;
    }
    if(block_read(0, &super) == ERROR)
    {
        // fail to read the block
        return ERROR;
    }
    if (strncmp(super.signature, "ECS150FS", 8) != 0)
    {
        // the signatura does not match 
        return ERROR;
    }
    if(super.total_blocks != block_disk_count())
    {
        // the opened disk number does not match
        return ERROR;
    }
    if (block_read(super.root_dir_index, (void *)rootDir) == ERROR) 
    {
        // fail to read the block
        return ERROR;
    }
    // allocate new memory for fat for block reading 
    fat = malloc(sizeof(uint16_t) * super.num_blocks_fat * BLOCK_SIZE);
    for(int i = 0; i < super.num_blocks_fat; i++) 
    {
        if(block_read(i+1, &fat[i]) == ERROR)
        {
            // fail to read the block to fat
            return ERROR;
        }
    }
    // success in mount a new file system
    return SUCCE;
}
int fs_umount(void)
{
	/* TODO: Phase 1 */
    //error checking
	if(block_disk_count() == ERROR)
	{
        // check if there is disk opened
		return ERROR;
	}
	if(block_disk_close() == ERROR)
	{
        return ERROR;
    }
    //unmount the file system
    free(fat);
	return SUCCE;
}

int fs_info(void)
{
    /* TODO: Phase 1 */
    //error checking
    if(block_disk_count() == ERROR)
    {
        // check if there is disk opened
        return ERROR;
    }
    // print the current information of the file system
	printf("FS Info:\n");
    printf("total_blk_count=%u\n", super.total_blocks);
    printf("fat_blk_count=%u\n", super.num_blocks_fat);
    printf("rdir_blk=%u\n", super.root_dir_index);
    printf("data_blk=%u\n", super.data_block_start_index);
    printf("data_blk_count=%u\n", super.amount_data_blocks);
    // count the blocks that are fat free
    int fat_free_blocks = 0;
    for (int i = 0; i < super.amount_data_blocks; i++) 
    {
        if (fat[i] == 0) 
        {
            fat_free_blocks++;
        }
    }
    // fat_free_ratio is the ratio of blocks that are fat free to the total number of blocks
    printf("fat_free_ratio=%d/%u\n", fat_free_blocks, super.amount_data_blocks);
    // count the number of root directory that are free
    int rdirCount = 0;
    for(int i = 0; i < FS_FILE_MAX_COUNT; i++)
    {
        if(strcmp(rootDir[i].filename, "\0") == 0)
        {
            rdirCount = rdirCount + 1;
        }
    }
    printf("rdir_free_ratio=%d/%d\n", rdirCount, FS_FILE_MAX_COUNT);
    //done
	return SUCCE;
}
int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
    // error checking for the parameter
	if(filename == NULL || strlen(filename) >= FS_FILENAME_LEN)
	{
        // invlid parameter
		return ERROR;
	}
    // check is the given filename exist
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
	{
        if (strcmp(rootDir[i].filename, filename) == 0) 
		{
            // given filename exist, can't create
            return ERROR; 
        }
    }
    // get a empty directory to store the new file
	int empty_entry = ERROR;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
	{
        if (strcmp(rootDir[i].filename, "") == 0) 
		{
            // empty space found
            empty_entry = i;
            break;
        }
    }
	if (empty_entry == ERROR) 
	{
        // there are no more empty space to create a new file
        return ERROR;
    }
    // initialize the directory 
	memset(rootDir[empty_entry].filename, 0, FS_FILENAME_LEN);
    strncpy(rootDir[empty_entry].filename, filename, FS_FILENAME_LEN - 1);
    rootDir[empty_entry].size_of_file = 0; 
    rootDir[empty_entry].index_of_first = FAT_EOC;

    if (block_write(super.root_dir_index, rootDir) == ERROR)
    {
        // fail in writing the block to disk
        return ERROR;
    }
    // done
	return SUCCE;
}
int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
	if (filename == NULL) 
    {
        // parameter has error 
        return ERROR;
    }
    // find the index of the file in the root directory
    int file_index = ERROR;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
    {
        if (strcmp(rootDir[i].filename, filename) == 0) 
        {
            // found match file for delete
            file_index = i;
            break;
        }
    }
    if (file_index == ERROR) 
    {
        // did not found the file that match the parameter
        return ERROR;
    }
    // free up the data blocks allocated to the file
    uint16_t current_block = rootDir[file_index].index_of_first;
    while (current_block != FAT_EOC) 
    {
        uint16_t next_block = fat[current_block];
        fat[current_block] = 0;
        current_block = next_block;
    }
    // set the value of the file as deleted in the root directory
    rootDir[file_index].filename[0] = '\0';
    rootDir[file_index].size_of_file = 0;
    rootDir[file_index].index_of_first = FAT_EOC;
    // done
    return SUCCE;
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
	if(block_disk_count() == ERROR) 
	{
        // no block is opened in this disk
		return ERROR;
	}
    // print the information 
	printf("FS Ls:\n");
    // iterate through the entries in the root directory
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
    {
        // check if the directory entry is not empty
        if (rootDir[i].filename[0] != '\0') 
        {
            // print information about the current file
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
    // done
    return SUCCE;
}

int fs_open(const char *filename)
{
    // error checking
    if (filename == NULL || strlen(filename) >= FS_FILENAME_LEN)
    {
        // filename is invalid 
        return ERROR;
    }
    // check if there is a entry matches the provided filename
    int index = ERROR;
    for (int i = 0; i < FS_FILE_MAX_COUNT; i++) 
    {
        if (strcmp(rootDir[i].filename, filename) == 0) 
        {
            // fount the entry that need to open
            index = i; 
            break;
        }
    }
    if (index == ERROR) 
    {
        // did not found the file
        return ERROR; 
    }
    // make sure the index is within the valid range 
    if (index < 0 || index >= FS_OPEN_MAX_COUNT) 
    {
        return ERROR;
    }
    // set the file descriptor entry with the provided information
    strcpy(filed[index].filename, filename); 
    filed[index].offset = 0;
    filed[index].file_index = index;
    //done
    return SUCCE; 
}
int fs_close(int fd)
{
	/* TODO: Phase 3 */
    //error checking
    if (fd >= FS_OPEN_MAX_COUNT || fd < 0)
    {
        //fd is not within the range
        return ERROR;
    }
    if (filed[fd].file_index == ERROR) 
    {
        // the target file is not valid 
        return ERROR;
    }
    // clear the file descriptor entry
    filed[fd].filename[0] = '\0';
    filed[fd].file_index = ERROR;
    filed[fd].offset = 0;
    // done
    return SUCCE;
}
int fs_stat(int fd)
{
	/* TODO: Phase 3 */
    //error checking
	if (super.signature[0] == '\0')
    {
        // filesystem is not mounted
        return ERROR;
    }
    if (fd < 0 || fd >= FS_OPEN_MAX_COUNT || filed[fd].file_index == ERROR)
    {
        // fd is not within the range or target file is invlid
        return ERROR;
    }
    int block_count = block_disk_count();
    if (block_count == ERROR)
    {
        // invlid counts of the block
        return ERROR;
    }
    int file_block_count = (rootDir[filed[fd].file_index].size_of_file + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (file_block_count > block_count)
    {
        // calculated block count is greater than the total block count
        return ERROR;
    }
    // done, return the size of the file
    return rootDir[filed[fd].file_index].size_of_file;
}
int fs_lseek(int fd, size_t offset)
{
    //error checking
    if (fd >= FS_OPEN_MAX_COUNT || fd < 0)
    {
        //parameter out of bounds
        return ERROR;
    }
    if (filed[fd].file_index == ERROR) 
    {
        //target file is invlid
        return ERROR;
    }
    if (offset > rootDir[filed[fd].file_index].size_of_file)
    {
        //provided offset is greater than the size of the file, invlid offset
        return ERROR;
    }
    // update the offset of the target file
    filed[fd].offset = offset;
    // doen
    return SUCCE;
}
// calculate the block index for a given file and offset
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
//find and allocate an available data block in the fat
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
// return the minimum of two size values
size_t min(size_t a, size_t b) 
{
    return a < b ? a : b;
}
int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
    //error checking
    if (super.signature[0] == '\0' || 
                            fd < 0 || 
                            fd >= FS_OPEN_MAX_COUNT || 
                            filed[fd].file_index == ERROR || 
                            buf == NULL) 
    {
        // invalid parameters or filesystem not mounted
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
        // allocate a new block if current block is 0xFFF
        if (current_block == FAT_EOC) 
        {
            current_block = allocate_block();
            if (current_block == FAT_EOC) 
            {
                //allocation was successful
                break;
            }
            // update fat
            fat[block_index] = current_block;
            fat[current_block] = FAT_EOC;
        }
        // read the block from disk into buffer
        if (block_read(current_block, block_buffer) == ERROR) 
        {
            // fail to read the block
            return ERROR;
        }
        // move the data from the provided buffer to the block buffer
        memcpy(block_buffer + block_offset, buf + bytes_written, bytes_to_write);
        // update the changed buffer
        if (block_write(current_block, block_buffer) == ERROR) 
        {
            // fail to updaate
            return ERROR;
        }
        // update the counters and move to the next block
        bytes_written += bytes_to_write;
        offset += bytes_to_write;
        block_index = fat[block_index];
    }
    // update file size in the root directory entry
    if (offset > rootDir[file_idx].size_of_file) 
    {
        rootDir[file_idx].size_of_file = offset;
    }
    //done
    return bytes_written;
}
int fs_read(int fd, void *buf, size_t count)
{
    // error checking
    if (fd < 0 || 
        fd >= FS_OPEN_MAX_COUNT || 
        filed[fd].filename[0] == '\0' || 
        block_disk_count() == ERROR || 
        buf == NULL)
    {
        //invlid parameter
        return ERROR;
    }
    int file_size = rootDir[filed[fd].file_index].size_of_file;
    if (filed[fd].offset >= file_size)
    {
        //there is nothing to read
        return SUCCE;
    }

    //calculate the block index and offset within the block
    uint16_t block_index = filed[fd].offset / BLOCK_SIZE;
    uint16_t block_offset = filed[fd].offset % BLOCK_SIZE;

    // read data from the file
    size_t bytes_read = 0;
    while (count > 0 && filed[fd].offset < file_size)
    {
        // read the block from disk
        char block[BLOCK_SIZE];
        uint16_t block_to_read = fat[rootDir[filed[fd].file_index].index_of_first + block_index];
        //check for FAT_EOC and read the block
        if (block_to_read == FAT_EOC || block_read(block_to_read, block) == ERROR)
        {
            // fail to read the block
            return ERROR; 
        }
        // calculate the number of bytes to read in the current block
        size_t bytes_to_read = count;
        if (bytes_to_read > (size_t)(BLOCK_SIZE - block_offset))
        {
            bytes_to_read = BLOCK_SIZE - block_offset;
        }
        // copy the data from the block
        memcpy(buf, block + block_offset, bytes_to_read);

        //update file offset and counters
        filed[fd].offset += bytes_to_read;
        buf += bytes_to_read;
        count -= bytes_to_read;
        bytes_read += bytes_to_read;

        // move to the next block
        ++block_index;
        block_offset = 0;
    }

    // done
    return bytes_read;
}
