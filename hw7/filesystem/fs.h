#ifndef FS_H
#define FS_H


typedef struct dirEntry {
   int         entryLength ;   // records length of this entry (can be used with names of variables length)
   Byte        isDir ;
   Byte        unUsed ;
   time_t      modTime ;
   int         fileLength ;
   fatEntry_t  firstBlock ;
   char        name [MAXNAME] ;
} dirEntry_t ;

typedef struct dirBlock {
   int isDir ;
   fatEntry_t parentBlockIndex;
   int nextEntry ;
   dirEntry_t entryList [ DIRENTRYCOUNT ] ; // the first two integer are marker and endpos
} dirBlock_t ;

typedef union block {
   dataBlock_t data ;
   dirBlock_t  dir  ;
   fatBlock_t  fat  ;
} diskBlock_t ;


typedef struct filedescriptor {
   int         pos;           // byte within a block
   char        mode;
   fatEntry_t  currBlockIndex;
   diskBlock_t buffer;
   fatEntry_t  lastBlockIndex;
   int         fileLength;
   fatEntry_t  parentBlockIndex;
   int         parentEntrylistIndex;
} MyFILE;


typedef struct directoryAndEntry {
   char folderName[MAXNAME];
   fatEntry_t folderFirstBlock;
   int pathToFolderFound;
   char entryName[MAXNAME];
   fatEntry_t entryFirstBlock;
   int entryFound;
} folderAndEntry;


FS * f_open(); // open the specified file with the specified access (read, write, read/write, append). If the file does not exist, handle accordingly. (rule of thumb: create file if writing/appending, return error if reading is involved). Returns a file handle if successful.

int f_read(); //read the specified number of bytes from a file handle at the current position. Returns the number of bytes read, or an error.

int f_write(); //write some bytes to a file handle at the current position. Returns the number of bytes written, or an error.

f_close(); //close a file handle

FS * f_seek(); // move to a specified position in a file

FS * f_rewind(); // move to the start of the file

f_stat(); // retrieve information about a file

f_remove(); // delete a file

f_opendir(); // recall that directories are handled as special cases of files. open a “directory file” for reading, and return a directory handle.

f_readdir(); // returns a pointer to a “directory entry” structure representing the next directory entry in the directory file specified.

f_closedir(); // close an open directory file

f_mkdir(); // make a new directory at the specified location

f_rmdir(); // delete a specified directory. Be sure to remove entire contents and the contents of all subdirectorys from the filesystem. Do NOT simply remove pointer to directory.

f_mount(); // mount a specified file system into your directory tree at a specified location. (Extra Credit)

f_umount(); // unmount a specified file system (Extra Credit)

#endif