#ifndef __DEFRAG_H__
#define __DEFRAG_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define N_DBLOCKS 10
#define N_IBLOCKS 4
#define BLOCKSIZE 512
#define BLOCKDATA 128
#define INODESIZE
#define N_INODES 
#define OFFSET 1024

typedef struct superblock {
  int size; /* size of blocks in bytes */
  int inode_offset; /* offset of inode region in blocks */
  int data_offset; /* data region offset in blocks */
  int swap_offset; /* swap region offset in blocks */
  int free_inode; /* head of free inode list, index, if disk is full, -1 */
  int free_block; /* head of free block list, index, if disk is full, -1 */
} SB;

typedef struct inode {
  int next_inode; /* index of next free inode */
  int protect; /* protection field */
  int nlink; /* number of links to this file */
  int size; /* numer of bytes in file */
  int uid; /* owner’s user ID */
  int gid; /* owner’s group ID */
  int ctime; /* change time */
  int mtime; /* modification time */
  int atime; /* access time */
  int dblocks[N_DBLOCKS]; /* pointers to data blocks */
  int iblocks[N_IBLOCKS]; /* pointers to indirect blocks */
  int i2block; /* pointer to doubly indirect block */
  int i3block; /* pointer to triply indirect block */
} inode;

#undef INODESIZE
#define INODESIZE sizeof(inode)


#endif