#include "defrag.h"

char *filename;
FILE *defrag_fp;
void* buffer;
void* defrag_inodes;
int size;
SB *superblock;
int total_inodes;
int total_blocks_tracked; 
int num_freeblocks; 
int num_datablocks; 
int curr_p = 0; 




void skedaddle(){
    free(buffer);
    free(defrag_inodes);
    fclose(defrag_fp);
    exit(1);
}

void man_page(){
    printf("\nNAME: ./defrag\n");
    printf("The defragmenter is invoked as follows: ./defrag <fragmented disk file>\n");
    printf("The defragmenter output a new disk image with ”-defrag” concatenated to the end of the input file name.\n");
    printf("Hence ./defrag myfile will produce the output file ”myfile-defrag”.\n");
    printf("\nDESCRIPTION:\n");
    printf("File system defragmenters improve performance by compacting all the blocks of a file into sequential order on disk.\n\n");
}

void write_idx_block(int start, int end){
  
  int idx[BLOCKDATA];
  for (int i=start, j=0; i <= end; i++){
    idx[j] = i;
    j++;
  }

  if (fwrite(idx, 1, BLOCKSIZE, defrag_fp) != BLOCKSIZE){
    skedaddle();
  }
 
  if (fseek(defrag_fp, 0, SEEK_END) == -1){
    skedaddle();
  } 
  //printf("Wrote index block, with address %d, indices from %d to %d\n", usedBlocks, usedBlocks + 1, usedBlocks + BLOCKSIZE/4);
}

void write_inode(inode* node, int idx){

  if (fseek(defrag_fp, OFFSET + (BLOCKSIZE * superblock->inode_offset) + (idx * INODESIZE), SEEK_SET) == -1){
    skedaddle();
  }
  if (fwrite(node, 1, INODESIZE, defrag_fp) != INODESIZE) {
    skedaddle();
  }
  if (fseek(defrag_fp, 0, SEEK_END) == -1){
    skedaddle();
  }
}

void write_block(int idx){ 

  void *block = buffer + OFFSET + (BLOCKSIZE * superblock->data_offset) + (BLOCKSIZE * idx);
  if (fwrite(block, 1, BLOCKSIZE, defrag_fp) != BLOCKSIZE){
    skedaddle();
  }
}

int get_direct(inode *node, inode *tmp, int data){

  for (int i = 0; i < N_DBLOCKS; i++){
        
    if (data <= 0) {
      break;
    }

    write_block(node->dblocks[i]);
    tmp->dblocks[i] = total_blocks_tracked;
    total_blocks_tracked++;
    data -= BLOCKSIZE;
  }
  return data;
}

int get_single_indirect(inode *node, inode *tmp, int data){
    
  for (int i=0; i<N_IBLOCKS; i++){
        
    if (data <= 0) {
      break;
    }
        
    write_idx_block(total_blocks_tracked + 1, total_blocks_tracked + BLOCKDATA);
    tmp->iblocks[i] = total_blocks_tracked;
    total_blocks_tracked++;
    
    for (int j=0; j<BLOCKSIZE; j+=4){
            
      if (data <= 0) {
        break;
      }
            
      void *tmp = buffer + OFFSET + (BLOCKSIZE * superblock->data_offset) + (BLOCKSIZE * node->iblocks[i]) + j ;
      int idx = *((int*) tmp);
      write_block(idx);
      total_blocks_tracked++;
      data -= BLOCKSIZE;
    }
  }
  return data; 
}

int get_free_blocks(){

  superblock->free_block = total_blocks_tracked;
  rewind(defrag_fp);
  
  if (fwrite(buffer, 1, OFFSET, defrag_fp) != OFFSET) {
    skedaddle();
  }

  if (fseek(defrag_fp, 0, SEEK_END) == -1){
    skedaddle();
  }

  bool swap = true; 

  void* to_sw = buffer + OFFSET + (BLOCKSIZE * superblock->swap_offset);
  void* max = to_sw - 1;
  
  if (to_sw >= buffer+size || to_sw < buffer+OFFSET){
    swap = false;
    max = buffer + size - 1;
  }

  void* temp = buffer + OFFSET + BLOCKSIZE * superblock->data_offset + BLOCKSIZE * total_blocks_tracked;
  int free = (max - (buffer+OFFSET+BLOCKSIZE*(superblock->data_offset + total_blocks_tracked - 1) + 1)) / BLOCKSIZE;
  
  for (int i = 0; i < free-1; i++){
    *(int*)temp = total_blocks_tracked;
    fwrite(temp, 1, BLOCKSIZE, defrag_fp);
    total_blocks_tracked++;
    temp = temp + BLOCKSIZE;
  }

  *(int*)(temp) = -1;
  fwrite(temp, 1, BLOCKSIZE, defrag_fp);

  //copy swap region
  if (swap){
    int k = buffer + size - to_sw - 1;
    fwrite(to_sw, 1, k, defrag_fp);
  }
  return 0;
}

void get_blocks(inode *node, inode *tmp, int idx){
  //tracking all blocks
  int data = node->size;
  
  data = get_direct(node, tmp, data);
  data = get_single_indirect(node, tmp, data);

  write_inode(tmp, idx);

}

inode defrag(inode file_inode, SB sb, FILE *disk_image, FILE *output_file) {
    struct datablock db;
    db.data = malloc(BLOCKSIZE);
    int file_size = file_inode.size/BLOCKSIZE + ((file_inode.size % BLOCKSIZE) != 0); 
    int num_pointers_per_iblock = BLOCKSIZE / sizeof(int); 
    int iter;
    
    if (file_size < N_DBLOCKS) {
        iter = file_size;
    } else {
        iter = N_DBLOCKS;
    }
    for (int i = 0; i < iter; i++) {
      
        fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (file_inode.dblocks[i] * BLOCKSIZE), SEEK_SET);
        fread(db.data, BLOCKSIZE, 1, disk_image);
        fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (curr_p * BLOCKSIZE), SEEK_SET);
        fwrite(db.data, BLOCKSIZE, 1, output_file);
        file_inode.dblocks[i] = curr_p; 
        curr_p++; 
    }

    if (file_size > N_DBLOCKS) { 
        for (int i = 0; i < N_IBLOCKS; i++) {
            if (file_inode.iblocks[i] > 0) {
                int indirect_block[num_pointers_per_iblock]; 
                fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (file_inode.iblocks[i] * BLOCKSIZE), SEEK_SET);
                fread(indirect_block, BLOCKSIZE, 1, disk_image);
                                
                file_inode.iblocks[i] = curr_p; //update inode pointers
                curr_p++; //wrote a block, move block pointer to next block

                if (file_size < ((num_pointers_per_iblock * i + N_DBLOCKS) + num_pointers_per_iblock)) {
                    //if datablocks will end in this block
                    iter = file_size - (num_pointers_per_iblock * i + N_DBLOCKS); //only iter the remaining valid blocks
                } else {
                    iter = num_pointers_per_iblock;
                }

                // Iterate over the indirect block to read data blocks
                for (int j = 0; j < iter; j++) {
                    // Read data block from disk image
                    fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (indirect_block[j] * BLOCKSIZE), SEEK_SET);
                    fread(db.data, BLOCKSIZE, 1, disk_image);

                    fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (curr_p * BLOCKSIZE), SEEK_SET);
                    fwrite(db.data, BLOCKSIZE, 1, output_file);
                    indirect_block[j] = curr_p;
                    curr_p++;
                }            

                //write contents of iblock (with updated contiguous offsets) to output file
                fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (file_inode.iblocks[i] * BLOCKSIZE), SEEK_SET);
                fwrite(indirect_block, BLOCKSIZE, 1, output_file);
            }
        }
    }
    
    //write secondary iblock
    if (file_size > (N_DBLOCKS + (N_IBLOCKS * num_pointers_per_iblock))) { //data cannot fit in IBLOCKS
        if (file_inode.i2block > 0) {
            int indirect_blocks[num_pointers_per_iblock]; //init arr to store all pointers to iblocks
            fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (file_inode.i2block * BLOCKSIZE), SEEK_SET);
            fread(indirect_blocks, BLOCKSIZE, 1, disk_image);
        
            file_inode.i2block = curr_p; //update inode pointers
            curr_p++; //wrote a block, move block pointer to next block

            for (int i = 0; i < num_pointers_per_iblock; i++) {
                if (indirect_blocks[i] > 0) {
                    int indirect_block[num_pointers_per_iblock]; //arr of 128 int "pointers" to DBLOCKS
                    fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (indirect_blocks[i] * BLOCKSIZE), SEEK_SET);
                    fread(indirect_block, BLOCKSIZE, 1, disk_image);
                    
                    indirect_blocks[i] = curr_p; //update inode pointers
                    curr_p++; //wrote a block, move block pointer to next block

                    if (file_size < ((num_pointers_per_iblock * N_IBLOCKS + N_DBLOCKS) + num_pointers_per_iblock * (i+1))) {
                        //if datablocks will end in this block
                        iter = file_size - ((num_pointers_per_iblock * N_IBLOCKS) + N_DBLOCKS + (num_pointers_per_iblock * i)); //only iter the remaining valid blocks
                    } else {
                        iter = num_pointers_per_iblock;
                    }

                    // Iterate over the indirect block to read data blocks
                    for (int j = 0; j < iter; j++) {
                        // Read data block from disk image
                        fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (indirect_block[j] * BLOCKSIZE), SEEK_SET);
                        fread(db.data, BLOCKSIZE, 1, disk_image);
                        fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (curr_p * BLOCKSIZE), SEEK_SET);
                        fwrite(db.data, BLOCKSIZE, 1, output_file);
                        indirect_block[j] = curr_p;
                        curr_p++;
                    }
                    fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (indirect_blocks[i] * BLOCKSIZE), SEEK_SET);
                    fwrite(indirect_block, BLOCKSIZE, 1, output_file);
                }
            }
            fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (file_inode.i2block * BLOCKSIZE), SEEK_SET);
            fwrite(indirect_blocks, BLOCKSIZE, 1, output_file);
        }

    }
     
    //and tertiary indirect blocks too
    if (file_size > (N_DBLOCKS + (N_IBLOCKS * num_pointers_per_iblock) + (num_pointers_per_iblock * num_pointers_per_iblock))) {
        if (file_inode.i3block > 0) {
            int i2blocks[num_pointers_per_iblock]; //init to store all pointers to i2blocks
            fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (file_inode.i3block * BLOCKSIZE), SEEK_SET);
            fread(i2blocks, BLOCKSIZE, 1, disk_image);
            
            file_inode.i3block = curr_p; //update inode pointers
            curr_p++; //wrote a block, move block pointer to next block

            for (int k = 0; k < num_pointers_per_iblock; k++) {
                if (i2blocks[k] > 0) {
                    int indirect_blocks[num_pointers_per_iblock]; //init arr to store all pointers to iblocks
                    fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (i2blocks[k] * BLOCKSIZE), SEEK_SET);
                    fread(indirect_blocks, BLOCKSIZE, 1, disk_image);
                    
                    i2blocks[k] = curr_p; //update inode pointers
                    curr_p++; //wrote a block, move block pointer to next block

                    for (int i = 0; i < num_pointers_per_iblock; i++) {
                        if (indirect_blocks[i] > 0) {
                            int indirect_block[num_pointers_per_iblock]; //arr of 128 int "pointers" to DBLOCKS
                            fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (indirect_blocks[i] * BLOCKSIZE), SEEK_SET);
                            fread(indirect_block, BLOCKSIZE, 1, disk_image);
                            
                            indirect_blocks[i] = curr_p; //update inode pointers
                            curr_p++; //wrote a block, move block pointer to next block

                            if (file_size < ((num_pointers_per_iblock * num_pointers_per_iblock) + (num_pointers_per_iblock * N_IBLOCKS + N_DBLOCKS) + num_pointers_per_iblock * (i+1))) {
                                //if datablocks will end in this block
                                iter = file_size - ((num_pointers_per_iblock * num_pointers_per_iblock) + (num_pointers_per_iblock * N_IBLOCKS) + N_DBLOCKS + (num_pointers_per_iblock * i)); //only iter the remaining valid blocks
                            } else {
                                iter = num_pointers_per_iblock;
                            }

                            // Iterate over the indirect block to read data blocks
                            for (int j = 0; j < iter; j++) {
                                // Read data block from disk image
                                fseek(disk_image, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (indirect_block[j] * BLOCKSIZE), SEEK_SET);
                                fread(db.data, BLOCKSIZE, 1, disk_image);
                                fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (curr_p * BLOCKSIZE), SEEK_SET);
                                fwrite(db.data, BLOCKSIZE, 1, output_file);
                                indirect_block[j] = curr_p;
                                curr_p++;
                            }
                            fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (indirect_blocks[i] * BLOCKSIZE), SEEK_SET);
                            fwrite(indirect_block, BLOCKSIZE, 1, output_file);
                        }
                    }
                    fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (i2blocks[k] * BLOCKSIZE), SEEK_SET);
                    fwrite(indirect_blocks, BLOCKSIZE, 1, output_file);
                }
            }
            fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (file_inode.i3block * BLOCKSIZE), SEEK_SET);
            fwrite(i2blocks, BLOCKSIZE, 1, output_file);
        }
    }
    free(db.data);
    return file_inode;
    
}

int main(int argc, char *argv[]) {


    if (argc <= 1 || argc > 2){
        printf("Invalid Input\n");
        exit(1);
    }
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0){
            man_page();
            exit(1);
    } else {
      filename = argv[1];
    }
  }
       
     FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("File not found.\n");
        exit(1);
    }

    if ( fseek(fp, 0, SEEK_END) == -1 ){
        skedaddle();
    } 
    
    size = ftell(fp);
    if (size == -1) {
        skedaddle();
    } 

    //if (size > ONE_GB){ skedaddle();}

    if ( fseek(fp, 0, SEEK_SET) == -1 ){
        skedaddle();
    }
     
    buffer = malloc(size);

    if ( buffer == NULL ){
        //printf("Not enough memory to defrag disk.\n");
        skedaddle();
    }
    
    if (fread(buffer, 1, size, fp) == -1){
        skedaddle();
    }

    superblock = (SB*) (buffer + BLOCKSIZE);
    
    //#undef BLOCKSIZE#define BLOCKSIZE sb->size
 
    if (superblock->data_offset < superblock->inode_offset){
        skedaddle();
    }
    
    int len = strlen(filename) + 8; 
    char* defrag_filename = (char*)malloc(len);
    strcat(defrag_filename, filename);
    strcat(defrag_filename, "-defrag");
    defrag_fp = fopen(defrag_filename, "w+");
    free(defrag_filename);
    
    if (defrag_fp == NULL) {
        skedaddle();
    }

    int offset = OFFSET + BLOCKSIZE * (superblock->data_offset);
    if ( fseek(defrag_fp, offset, SEEK_SET) == -1 ){
        skedaddle();
    }

    fclose(fp);
    free(buffer);
  
   //second flag is diskname
        // Open disk image file and output file
    FILE *disk_image = fopen(filename, "rb");
    char *output_filename = strcat(filename, "-defrag");
    FILE *output_file = fopen(output_filename, "wb");

    SB sb;
    fseek(fp, BLOCKSIZE, SEEK_SET);
    fread(&sb, sizeof(SB), 1, fp);

    char boot_buffer[BLOCKSIZE];
    fseek(disk_image, 0, SEEK_SET);
    fread(boot_buffer, BLOCKSIZE, 1, disk_image);
    fseek(output_file, 0, SEEK_SET);
    fwrite(boot_buffer, BLOCKSIZE, 1, output_file);

        
    SB cpysb;
    fseek(disk_image, BLOCKSIZE, SEEK_SET);
    fread(&cpysb, sizeof(SB), 1, disk_image);
    fseek(output_file, BLOCKSIZE, SEEK_SET);
    fwrite(&cpysb, sizeof(SB), 1, output_file);

    int inode_region_size = (sb.data_offset - sb.inode_offset) * BLOCKSIZE;

    // Skip to the inode region    
    fseek(disk_image, (OFFSET + (sb.inode_offset * BLOCKSIZE)), SEEK_SET);
    fseek(output_file, (OFFSET + (sb.inode_offset * BLOCKSIZE)), SEEK_SET);

    // Read inodes until the end of the inode region
    inode node;
    int inode_index = 0;
    while ((ftell(disk_image) + sizeof(inode)) < ((OFFSET + (sb.inode_offset * BLOCKSIZE)) + inode_region_size)) {
        if (fread(&node, sizeof(inode), 1, disk_image) != TRUE) {
            
            skedaddle();
        }

        if (node.nlink > 0) { //  i-nodes in use (others are unlinked)
            // Defragment the file's data blocks
            inode update_node = defrag(node, sb, disk_image, output_file);
            fseek(output_file, (OFFSET + (sb.inode_offset * BLOCKSIZE)) + (inode_index * sizeof(inode)), SEEK_SET);
            fwrite(&update_node, sizeof(inode), 1, output_file);

        } else {
            fwrite(&node, sizeof(inode), 1, output_file);
        }
        inode_index++; 
        fseek(disk_image, (OFFSET + (sb.inode_offset * BLOCKSIZE)) + (inode_index * sizeof(inode)), SEEK_SET);
        
    }
  
     struct freeblock fb;
    fb.junk = NULL;
    sb.free_block = curr_p;

    while (curr_p < sb.swap_offset) {
        fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + (curr_p * BLOCKSIZE), SEEK_SET);
        fb.next = curr_p + 1;
        fwrite(&fb.next, sizeof(int), 1, output_file);
        curr_p++;
    }

    //make last freeblock next -1
    fseek(output_file, (OFFSET + (sb.data_offset * BLOCKSIZE)) + ((curr_p - 1) * BLOCKSIZE), SEEK_SET);
    fb.next = END;
    fwrite(&fb.next, sizeof(int), 1, output_file);

        
    fclose(output_file);   
    fclose(disk_image);

    output_file = fopen(output_filename, "rb");

    fclose(output_file);

    
    return EXIT_SUCCESS;
}
