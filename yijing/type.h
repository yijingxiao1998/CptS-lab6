/*************** type.h file ************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES for convenience
typedef struct ext2_super_block SUPER;    // p 303
/* struct ext2_super_block
// contains information about the entire file system
{
   u32 s_inodes_count; // Inodes count
   u32 s_blocks_count; // Blocks count
   u16 s_magic;        // it is the magic number which identifies the file system type
}
*/
typedef struct ext2_group_desc  GD;       // p 303 and 232
/* struct ex22_group_desc
// EXT2 divides disk blocks into groups. Each group contains 8192 blocks.
{
   // For Linux formatted EXT2 file system, blocks 3 to 7 are reserved
   // So bmap=8, imap=9 and inode_table=10
   u32 bg_block_bitmap;    // Bmap block number
   u32 bg_inode_bitmap;    // Imap block number
   u32 bg_inode_table;     // Inodes begin block number
   u16 bg_free_blocks_count, bg_free_inodes_count, bg_used_dires_count;
   u32 bg_reserved[3];
}
*/
// (p 233) An inode is a data structure used to represent a file
typedef struct ext2_inode       INODE;    // p 304
/* struct ext2_inode
// Every file is represented by a unique inode structure of 128 bytes
{
     u16 i_mode;          // 16 bits = |tttt|ugs|rwx|rwx|rwx| 
                          // the leading 4 bits specify the fle type
     u16 i_uid;           // owner uid
     u32 i_size;          // file size in bytes
     u32 i_atime;         // time fields in seconds
     u32 i_ctime;         // since 00:00:00, 1-1-1970
     u32 i_mtime, dtime;
     u16 i_gid;           // group ID
     u16 i_links_count;   // hard-link count
     u32 i_blocks;        // number of 512-byte sectors
     u32 i_block[15];     // contains pointers to disk blocks of a file
     			   // Direct blocks: i_block[0-11] which point to direct disk blocks
     u32 i_pad[7];        // for inode size = 128 bytes
};*/
typedef struct ext2_dir_entry_2 DIR;      // p 305
// The fiest real data block is B33, which is i_block[0] of the root directory /
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
#define NPROC       2  // number of PROCs
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
  struct proc *next;        // next proc pointer
  int          pid;         // process ID
  int          ppid;        // parent process pid
  int          status;      // PROC status=FREE |READY, etc
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];     // points to opened file instance
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
