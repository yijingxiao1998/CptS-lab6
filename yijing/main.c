/****************************************************************************
*                   KCW testing ext2 file system                            *
*****************************************************************************/
#include "header.h"

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;
  OFT *oft;

  printf("init()\n");
  
  // initialize all mtable entries
  for(i=0; i<NMTABLE; i++)
  	mtable[i].dev = 0;
  // setting first entry in mtbale to opening filesys
  strcpy(mtable[0].devName, disk);
  strcpy(mtable[0].mntName, "/");
  mtable[0].dev = dev;

  // initialize all minodes as FREE
  for (i=0; i<NMINODE; i++){  
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  
  // initialize PROCs
  for (i=0; i<NPROC; i++){    
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;         // all file descriptors are NULL
  }
  
  proc[NPROC - 1].next = &proc[0];    // circular list
  running = &proc[0];                 // p0 runs first
}

// load root INODE and set root pointer to it (p 328)
int mount_root()  // mount root file system
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

int main(int argc, char *argv[ ])
{
  char* readLinkBuf=malloc(BLKSIZE);
  int ino, num;
  char buf[BLKSIZE];
  char line[128], cmd[32], pathname[128], temp[128];

  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }
  dev = fd;   

  /********** read super block  ****************/
  get_block(dev, 1, buf);  // get superblock
  sp = (SUPER *)buf;

  /***** verify it's an ext2 file system *******/
  // For EXT2/3/4 files systems, the magic number is 0xEF53
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;  // get inodes_count
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf);        // get group descriptor
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;    // get imap block number
  inode_start = gp->bg_inode_table;  // root inode information
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  while(1){
    printf("input command : [ls|cd|pwd|  mkdir|rmdir|creat|  link|unlink|symlink|  open|close|lseek|pfd|  quit] ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;

    bzero(temp, 128);
    sscanf(line, "%s %s %s", cmd, pathname, temp);
    printf("cmd=%s pathname=%s %s\n", cmd, pathname, temp);
  
    if (strcmp(cmd, "ls")==0)
       ls(pathname);
    if (strcmp(cmd, "cd")==0)
       chdir(pathname);
    if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    if(strcmp(cmd, "mkdir") == 0)
      make_dir(pathname);
    if(strcmp(cmd, "creat") == 0)
      creat_file(pathname);
    if(strcmp(cmd, "rmdir")==0)
      rmdir(pathname);
    if(strcmp(cmd, "link") == 0)
    	link(pathname, temp);
    if(strcmp(cmd, "unlink") == 0)
    	unlink(pathname);
    if(strcmp(cmd, "symlink")==0)
    	symlink(pathname, temp);
    if(strcmp(cmd, "readlink")==0)
      	readlink(pathname, readLinkBuf, BLKSIZE);
    if(strcmp(cmd, "stat") == 0)
    	stat_file(pathname);	
    if(strcmp(cmd, "open") == 0)
    {
    	int mode;
    	printf("input open mode (use 0|1|2|3 for R|W|RW|APPEND):");
    	scanf("%d", &mode);
    	while((getchar()) != '\n');
    	fd = open_file(pathname,mode);
    }
    if(strcmp(cmd, "close") == 0)
    	close_file(fd);
    if(strcmp(cmd, "read") == 0)
    	read_file();
    if(strcmp(cmd, "write") == 0)
    	write_file();
    if(strcmp(cmd, "cat") == 0)
    	cat(pathname);
    if(strcmp(cmd, "cp") == 0)
    	cp(pathname, temp);
    if(strcmp(cmd, "pfd") == 0)
    	pfd();	
    if(strcmp(cmd, "mount") == 0)
    	mount_file();
    if(strcmp(cmd, "umount") == 0)
    	umount(pathname);

    if (strcmp(cmd, "quit")==0)
       quit();
  }
}
