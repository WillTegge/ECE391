#ifndef _FILE_S_H
#define _FILE_S_H

#include "types.h"
#include "multiboot.h"

#define kBYTE4 4096
#define FILENAMESIZE 32
#define RESERVESIZE24 24
#define RESERVESIZE52 52
#define DIR_ENTRIES 63
#define DATA_BLOCKS 1023

typedef struct __attribute__((packed)) dentry_t{

    uint8_t     file_name[FILENAMESIZE];
    uint32_t    file_type;
    uint32_t    inode_num;
    uint8_t     res[RESERVESIZE24];

} dentry_t;

//create boot block structure that is 4kb
typedef struct __attribute__((packed)) bootblock_t{

    uint32_t    num_dir_entries;
    uint32_t    nume_inodes;
    uint32_t    num_data_blocks;
    uint8_t     res[RESERVESIZE52];
    dentry_t    dir_entries[DIR_ENTRIES];

} bootblock_t;

/* inode structure */
typedef struct __attribute__((packed)) inode_t{
    
    uint32_t file_size;
    uint32_t data_block[DATA_BLOCKS];

} inode_t;

typedef struct __attribute__((packed)) data_block_t{

    uint8_t data[kBYTE4];

} data_block_t;

bootblock_t* bbg;
data_block_t* data_blocks;


int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);           

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);               

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t file_open(const uint8_t* filename);                        

int32_t file_close(int32_t fd);                                               

int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);    

int32_t dir_open(const uint8_t* filename);                         

int32_t dir_close(int32_t fd);                                                

int32_t dir_read(int32_t index, void* buf, int32_t nbytes);                                   

int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);                                                

int32_t filesystem_init(module_t* filesys_start); 

#endif /* _FILE_S_H */
