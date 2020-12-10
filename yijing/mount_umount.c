// 1. Ask for filesys (a pathname) and mount_point (a pathname also).
int mount(char *filesys, char * mount_point)   /*  Usage: mount filesys mount_point OR mount */
{
   int i, fd, ino;
   MTABLE *mountptr;
   SUPER *ssp;
   char buf[BLKSIZE];
   MINODE *mip;
   
   // If mount with no parameters: display current mounted filesystems.
   if(strlen(filesys) == 0)
   {
   	printf("Display current mounted file systems:\n");
   	for(i = 0; i < NMTABLE; i++)
   	{
   	   if(mtable[i].dev)  // valid entry
   	   	printf("%s is mounted on %s\n", mtable[i].devName, mtable[i].mntName);
   	   	// devName is device name and mntName is mount point dir name
   	}
   	return 0;
   }

   // 2. Check whether filesys is already mounted: 
   // (you may store the name of mounted filesys in the MOUNT table entry). 
   for(i = 0; i < NMTABLE; i++)
   {
   	if(mtable[i].dev)  // If already mounted, reject;
   	{
   	   if(strcmp(mtable[i].devName, filesys) == 0)
   	   {
   	   	printf("mount failed: file system already mounted\n");
   	   	return -1;
   	   }
   	}
   }
   
   // else: allocate a free MOUNT table entry (whose dev=0 means FREE).
   for(i = 0; i < NMTABLE; i++)
   {
   	if(!mtable[i].dev)
   	{
   	   mountptr = &mtable[i];
   	   break;
   	}
   }

   // 3. open filesys for RW; use its fd number as the new DEV;
   // Check whether it's an EXT2 file system: if not, reject.
   printf("checking EXT2 FS ....");
   if((fd = open(filesys, O_RDWR)) < 0)
   {
    	printf("open %s failed\n", filesys);
    	return -1;
   }
   /********** read super block  ****************/
   get_block(fd, 1, buf);  // get superblock
   ssp = (SUPER *)buf;

  /***** verify it's an ext2 file system *******/
  // For EXT2/3/4 files systems, the magic number is 0xEF53
  if (ssp->s_magic != 0xEF53)
  {
      printf("magic = %x is not an ext2 filesystem\n", ssp->s_magic);
      return -1;
  }     
  printf("EXT2 FS OK\n");

   // 4. For mount_point: find its ino, then get its minode:
   // call  ino  = getino(pathname);  to get ino:
   ino = getino(mount_point, &dev);
   if(ino == 0)
   {
   	printf("mount failed: mount point %s does not exist\n", mount_point);
   	return -1;
   }
   // call  mip  = iget(DEV, ino);    to get its minode in memory;   
   mip = iget(dev, ino);

   // 5. Check mount_point is a DIR.  
   if(!S_ISDIR(mip->INODE.i_mode))
   {
   	printf("mount failed: %s is not a dir\n", mount_point);
   	iput(mip);
   	return -1;
   }
   // Check mount_point is NOT busy (e.g. can't be someone's CWD)
   if(mip->refCount > 1)
   {
   	printf("mount failed: %s is busy\n", mount_point);
   	iput(mip);
   	return -1;
   }

   // 6. Record new DEV in the MOUNT table entry;
   //(For convenience, store the filesys name in the Mount table, and also its
   // ninodes, nblocks, bitmap blocks, inode_start block, etc. for quick reference)
   mountptr->mntDirPtr = mip;
   mountptr->dev = fd;
   mountptr->ninodes = ssp->s_inodes_count;  // copy super block info into mtable
   mountptr->nblocks = ssp->s_blocks_count;
   strcpy(mountptr->devName, filesys);
   strcpy(mountptr->mntName, mount_point);

   // 7. Mark mount_point's minode as being mounted on and let it point at the
   mip->mounted = 1;
   // MOUNT table entry, which points back to the mount_point minode.
   mip->mptr = mountptr;
   mip->dirty = 1;
   
   printf("mount done\n");
   
   return 0; //SUCCESS;
}

int mount_file()
{
   char filesys[BLKSIZE], mount_point[BLKSIZE]; 
   printf("enter filesys to mount and a pathname as mount point\n");
   printf("filesys:");
   fgets(filesys, BLKSIZE, stdin);
   filesys[strlen(filesys)-1] = 0;
   printf("mount point:");
   fgets(mount_point, BLKSIZE, stdin);
   mount_point[strlen(mount_point)-1] = 0;
   printf("filesys = %s, mount point = %s\n", filesys, mount_point);
   
   mount(filesys, mount_point);
} 

int umount(char *filesys)
{
   int i;
   MTABLE *mountptr;
   MINODE *mip;
   
   // 1. Search the MOUNT table to check filesys is indeed mounted.
   for(i = 0; i < NMTABLE; i++)
   {
   	if(mtable[i].dev)  // entry exist
   	{
   	   if(strcmp(mtable[i].devName, filesys) == 0)
   	   {
   	   	mountptr = &mtable[i];  // point local instance at same name
   	   	printf("file system %s is currently mounted on %s\n", mountptr->devName, mountptr->mntName);
   	   	break;
   	   }
   	}
   }

  // 2. Check whether any file is still active in the mounted filesys;
  // e.g. someone's CWD or opened files are still there,
  if(running->cwd->dev == mountptr->dev)  // file system is cwd
  {
   	printf("umount failed: %s is still active\n", filesys);
   	return -1;
  }
  for(i = 0; i < NFD; i++)
  {
   	// if so, the mounted filesys is BUSY ==> cannot be umounted yet.
   	// HOW to check?      ANS: by checking all minode[].dev
   	if(running->fd[i])  // fd contains open file
   	{
   	   if(running->fd[i]->mptr->dev == mountptr->dev)
   	   {
   	   	printf("umount failed: %s has opened file\n", filesys);
   	   	return -1;
   	   }
   	}
   }

   // 3. Find the mount_point's inode (which should be in memory while it's mounted 
   // on).  Reset it to "not mounted"; 
   // then  iput()   the minode.  (because it was iget()ed during mounting)
   mip = mountptr->mntDirPtr;
   mountptr->dev = 0;
   mountptr->mntDirPtr->mounted = 0;
   iput(mip);
   
   printf("umount done\n");
   
   return 0; // SUCCESS;
}  
