int stat_file(char *pathname)
{ 
        struct stat myst;   // p 205
        int ino = getino(pathname, &dev);  // get INODE of filename into memory:
        MINODE *mip = iget(dev, ino);
        // copy dev, ino to myst.st_dev, myst.st_ino;
        myst.st_dev = dev;
        myst.st_ino = ino;
            
        // copy mip->INODE fields to myst fields;  (p 252)
        myst.st_mode = mip->INODE.i_mode;
        myst.st_nlink = mip->INODE.i_links_count;
        myst.st_uid = mip->INODE.i_uid;
        myst.st_gid = mip->INODE.i_gid;
        myst.st_size = mip->INODE.i_size;
        myst.st_blocks = mip->INODE.i_blocks;
        myst.st_atime = mip->INODE.i_atime;
        myst.st_mtime = mip->INODE.i_mtime;
        myst.st_ctime = mip->INODE.i_ctime;
        
        // display in the terminal
        printf("\n\tFile: %s\n", pathname);
        printf("\tSize: %d  \tBlocks: %d\n", myst.st_size, myst.st_blocks);
        printf("\tDevice: %d  \tInode: %d  \tLinks: %d\n", myst.st_dev, myst.st_ino, myst.st_nlink);
        printf("\tUid: %d  \tGid: %d\n", myst.st_uid, myst.st_gid);
        printf("\n");
        
        iput(mip);
        printf("stat %s done\n", pathname);
} 

int chmod_file(char *pathname, int mode)
{
	// name mode: (mode = |rwx|rwx|rwx|, e.g. 0644 in octal)
	// get INODE of pathname into memroy:
	int ino = getino(pathname, &dev);
	MINODE *mip = iget(dev, ino);
	printf("chmod: mode is %d\n", mode);
	
	// user want to change to same mode or user is superuser
	if((mip->INODE.i_mode == mode) || (running->uid == 0))  
		return 0;
		
	if(running->uid != mip->INODE.i_uid)
	{
		printf("chmod failed: do not have access to %s", pathname);
		return -1;
	}
	
	mip->INODE.i_mode |= mode;
	mip->dirty = 1;
	iput(mip);
	printf("chmod done\n");
}
