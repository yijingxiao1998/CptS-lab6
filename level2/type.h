/*************** type.h file ************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES for convenience
typedef struct ext2_super_block SUPER;    // p 303
typedef struct ext2_group_desc  GD;       // p 303
typedef struct ext2_inode       INODE;    // p 304
typedef struct ext2_dir_entry_2 DIR;      // p 305
/* struct ext2_dir_entry_2
  {
     u32 inode;                      // inode number; count from 1, not 0
     u16 rec_len;                    // this entry's length in bytes
     u8 name_len;                    // name length in bytes
     u8 file_type;                   // not used
     char name[EXT2_NAME_LEN];       // name: 1-225 chars, no ending NULL
   };*/

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   

// proc status
#define FREE        0
#define READY       1

// file system table sizes
#define BLKSIZE  1024
#define NMINODE   128
#define NMTABLE    10
#define NFD        16
#define NPROC       2
#define NOFT       40

// In-memory inodes structure
typedef struct minode{
  INODE INODE;             // disk inode
  int dev, ino;
  int refCount;            // use count
  int dirty;               // modified flag
  int mounted;             // mounted flag
  struct mntable *mptr;    // mount table pointer
}MINODE;

// open file Table    opened file instance
typedef struct oft{
  int  mode;         // mode of opened file
  int  refCount;     // number of PROCs sharing this instance
  MINODE *mptr;      // pointer to minode of file
  int  offset;       // byte offset for R|W
}OFT;

// PROC structure
typedef struct proc{
  struct proc *next;
  int          pid;
  int          ppid;
  int          status;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

// Mount Table structure
typedef struct mtable{
  int dev;                 // device number; 0 for FREE
  int ninodes;             // from superblock
  int nblocks;
  int free_blocks;         // from superblock and GD
  int free_inodes;
  int bmap;                // from group descriptor
  int imap;
  int iblock;              // inodes start block
  MINODE *mntDirPtr;       // mount point DIR pointer
  char devName[64];        // device name
  char mntName[64];        // mount point DIR name
}MTABLE;
