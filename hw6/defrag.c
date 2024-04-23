#include "defrag.h"

char *filename;
FILE *defrag_fp;
void* buffer;
void* defrag_inodes;
int size;
SB *superblock;
int total_inodes;
int total_blocks_tracked; 


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

void read_input(){

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
    return;
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

int defrag(){
    
    read_input();

    total_inodes = (superblock->data_offset - superblock->inode_offset) * BLOCKSIZE;
    total_inodes /=  (INODESIZE + 0.0);

    int size = total_inodes * INODESIZE;
    defrag_inodes = malloc(size);
    if (defrag_inodes == NULL){
        skedaddle();
    }

    void *pos = buffer + OFFSET + (BLOCKSIZE * superblock->inode_offset);
    memcpy(defrag_inodes, pos, size);

    total_blocks_tracked = 0;

    for (int i=0; i<total_inodes; i++){
         inode *tmp = (inode*) (buffer + OFFSET + BLOCKSIZE * superblock->inode_offset + i * INODESIZE);

        if (tmp->nlink == 1){
            inode *newtemp = (inode*) (defrag_inodes + i * INODESIZE);
            //printf("\nInode %d: new node:%d\n", i, newtemp->protect);
            get_blocks(tmp, newtemp, i);
            
        }
    }

    get_free_blocks();
    skedaddle();
    return 0;
}

int main(int argc, char *argv[]){

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

  defrag();
  return 0;
}