#include "filesystem.h"
#include "lib.h"
#include "systemcall.h"
/*
 filesystem_init
    DESCRIPTION: Initialize the filesystem
    INPUTS: filesys_start - pointer to start of boot block
    OUTPUTS: none
    RETURN VALUE: 0 - success
    SIDE EFFECTS: Initializes bootblock and data blocks
                  
 */
int32_t filesystem_init(module_t* filesys_start) {
    bbg = (bootblock_t*)filesys_start;                              //initializing the bootblock
    data_blocks = (data_block_t*)(bbg + 1 + bbg->nume_inodes);      //initializing the data blocks
    return 0;
}

/*
 read_dentry_by_name
    DESCRIPTION: searches for file by name in directory
    INPUTS: filename, directory entry
    OUTPUTS: none
    RETURN VALUE: 0 - success, -1 - error
    SIDE EFFECTS: Fills directory entry with the found file information
                  
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    
    /* Check validity of fname */
    if(fname == 0){
        return -1;
    }
    
    /* 
     * Iterate through directory entries to find the correct file
     * matching the given file name. Copy file name, file type,
     * and inode number into the dentry structure and return 0.
     * Otherwise return -1 for file name not found. 
    */
    int i;
    for(i = 0; i < bbg->num_dir_entries; i++){                              
        if(strncmp((const int8_t*)bbg->dir_entries[i].file_name, (const int8_t*)fname, 32) == 0){                         
            strncpy((int8_t*)dentry->file_name, (const int8_t*)bbg->dir_entries[i].file_name,32);
            dentry->file_type = bbg->dir_entries[i].file_type;              
            dentry->inode_num = bbg->dir_entries[i].inode_num;
            return 0;
        }
    }
    return -1;
}

/*
 read_dentry_by_index
    DESCRIPTION: Searches for file by index in directory
    INPUTS: index, directory entry
    OUTPUTS: none
    RETURN VALUE: 0 - success, -1 - error
    SIDE EFFECTS: fills directory entry with the found file information
                  
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    /* Check validity of index */
    if(index < 0){
        return -1;
    }

    /* 
     * Copy file name, file type, and inode number into dentry from 
     * the directory entry with given index and return 0.
     */
    strncpy((int8_t*)dentry->file_name, (const int8_t*)bbg->dir_entries[index].file_name, FILENAMESIZE);
    dentry->file_type = bbg->dir_entries[index].file_type;
    dentry->inode_num = bbg->dir_entries[index].inode_num;
    return 0;
}

/*
 read_data
    DESCRIPTION: Reads the data from a file from the file's corresponding datablocks
                 and copies them into buffer. 
    INPUTS: inode - inode number, offset - offset from start, buf - buffer that gets filled, 
            length - length of file we want to read
    OUTPUTS: none
    RETURN VALUE: -1 - error, nbytes - bytes read
    SIDE EFFECTS: Fills buffer with information from data blocks
                  
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    /* Check validity of inode number */
    if(inode < 0  || inode > bbg->nume_inodes){
        return -1;
    }

    // int nbytes = 0; //tracks number of bytes read
    // int cur_db; //tracks the current datablock being read
    // int itr_buf = 0; //tracks the location to store data block in buffer
    // int bytes_left; //tracks how many bytes are left unread in data block
    int data_block_index; //index of datablock being currently read
    int i;

    /*

    */

    inode_t* inode_ptr = ((inode_t*)bbg + 1 + inode);               //creating inode ptr
    for(i = offset; i - offset < length && i < inode_ptr->file_size; i++){
        data_block_index = (inode_ptr->data_block)[i/kBYTE4];             //selecting info current data block
        data_block_t* data_block = &(data_blocks[data_block_index]);
        buf[i-offset] = data_block->data[i%kBYTE4]; 
    }
       //creating datablock we will be accessing

    //read current data block
    // for(cur_db = 0; cur_db < bbg->num_data_blocks; cur_db++){ 
    //     inode_t* inode_ptr = ((inode_t*)bbg + 1 + inode);               //creating inode ptr
    //     data_block_index = (inode_ptr->data_block)[cur_db];             //selecting info current data block
    //     data_block_t* data_block = &(data_blocks[data_block_index]);    //creating datablock we will be accessing
    //     bytes_left = 4096 - offset%4096;                                //var for bytes left to read from data block
    //     //if we on the "first" data block
    //     if(cur_db == 0){
    //         /* while we're not at max affest, havent read all bytes, and havent exceeded buffer size */
    //         while((bytes_left != 0) && ((length - nbytes) != 0)){
    //             /* buffer will contain data block's information */
    //             /* use the index to actually find data_block */
    //             buf[itr_buf] = data_block->data[offset];
    //             // memcpy(buf[itr_buf], data_block->data[offset], strlen(buf));
    //             /* increase bytes read, go to next byte, and next buffer location */
    //             itr_buf++;
    //             offset++;
    //             nbytes++;
    //             bytes_left--;
    //         }
    //         /* if we've coppied all data, return all the bytes we've read */
    //         if(((length - nbytes) == 0)){
    //             return nbytes;
    //         }
    //     }
    //     else{
    //         //this is case when we read the next datablock
    //         offset = 0;
    //         while(((length - nbytes) != 0) && (bytes_left != 0)){
    //             /* buffer will contain data block's information */
    //             /* use the index to actually find data_block */
    //             buf[itr_buf] = data_block->data[offset];
    //             /* increase bytes read, go to next byte, and next buffer location */
    //             offset++;
    //             itr_buf++;
    //             nbytes++;
    //             bytes_left--;
    //         }
    //         /* if we've coppied all data, return all the bytes we've read */
    //         if((length - nbytes) == 0){
    //             return nbytes;
    //         }
    //     }

    // }
    return i - offset;
}

/*
 file_open
    DESCRIPTION: Does nothing in file system context
    INPUTS: filename
    OUTPUTS: NONE
    RETURN VALUE: 0
    SIDE EFFECTS: NONE
                  
 */
int32_t file_open(const uint8_t* filename){
    return 0;
}

/*
 file_close
    DESCRIPTION: Does nothing in file system context
    INPUTS: fd - file descriptor
    OUTPUTS: NONE
    RETURN VALUE: 0
    SIDE EFFECTS: NONE
                  
 */
int32_t file_close(int32_t fd){
    return 0;
}

/*
 file_read
    DESCRIPTION: Reads desired file's data using read_data function up to nbytes.
                Data read is put into buffer. 
    INPUTS: filedirectory, buffer, and number of bytes to read
    OUTPUTS: NONE
    RETURN VALUE: -1 - error, strlen(buf) - nbytes read
    SIDE EFFECTS: Fills buffer with file's data.
                  
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    //read should keep track of wya, two seperate args, {buf, num bytes}
    //read from where last left off and return number of bytes read
    //go to inode, claculate according to starting of inode (in bytes), return
    uint32_t inode_index = PCB[display_terminal]->fd_array[fd].inode; 
    // inode_t* cur_inode = ((inode_t*)bbg + 1 +inode_index);
    int32_t  read_bytes=0;
    read_bytes = read_data(inode_index, PCB[display_terminal]->fd_array[fd].file_pos, (uint8_t*)buf, nbytes);
    if(read_bytes == -1){
        return -1;
    }
    PCB[display_terminal]->fd_array[fd].file_pos+= read_bytes;
    return read_bytes;
}

/*
 file_write
    DESCRIPTION: Does nothing in file system context
    INPUTS: fd - file directory, buf - buffer, nbytes - bytes to write
    OUTPUTS: NONE
    RETURN VALUE: -1
    SIDE EFFECTS: NONE
                  
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/*
 dir_open
    DESCRIPTION: Does nothing in file system context
    INPUTS: filename
    OUTPUTS: NONE
    RETURN VALUE: 0
    SIDE EFFECTS: NONE
                  
 */
int32_t dir_open(const uint8_t* filename){
    return 0;
}

/*
 dir_close
    DESCRIPTION: Does nothing in file system context
    INPUTS: fd - file descriptor
    OUTPUTS: NONE
    RETURN VALUE: 0
    SIDE EFFECTS: NONE           
 */
int32_t dir_close(int32_t fd){
    return 0;
}

/*
 dir_read
    DESCRIPTION: Reads directory entry by index using read_dentry_by_index
                function and copies information into temporary dentry structure. 
                Use dentry to copy file name into buffer.
    INPUTS: fd - file descriptor, buf - buffer, nbytes - nbytes to read
    OUTPUTS: NONE
    RETURN VALUE: -1 - error, 32 - max file name size, strlen(buf) - nbytes read
    SIDE EFFECTS: Copies file name into buffer
                  
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){

    //dir read: returns a file name from directory-> know theres one dir, start from top
    // from '.' till you hit NULL; should include '.' file 
    // if(fd == 8372256){
    //     fd = 0;
    // }
    dentry_t temp_dentry;
    if(read_dentry_by_index(PCB[display_terminal]->fd_array[fd].file_pos, &temp_dentry) == -1){
        return -1;
    }
    strncpy((int8_t*)buf, (const int8_t*)temp_dentry.file_name, FILENAMESIZE);
    PCB[display_terminal]->fd_array[fd].file_pos++;
    if(strlen((const int8_t*)buf) > FILENAMESIZE)
        return FILENAMESIZE;
    return strlen((const int8_t*)buf);
}   

/*
 dir_write
    DESCRIPTION: Does nothing in file system context
    INPUTS: fd - file directory, buf - buffer, nbytes - bytes to wrtie
    OUTPUTS: NONE
    RETURN VALUE: -1
    SIDE EFFECTS: NONE  
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}


// dir_read how to get index for read_dentry_by_index if using file descriptor?
// file operations jump table for initializing task block with file descriptor
// how would information differ for directories(0) in the task array such as file position, inode, etc.
// difference between dir_open and file_open? because currently they are the same
