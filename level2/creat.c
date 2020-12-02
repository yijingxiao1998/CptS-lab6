int mycreat(MINODE* pip, char* child)
{
   // Same as mymkdir() except 
   //  INODE's file type = 0x81A4 OR 0100644
   //  links_count = 1
   //  NO data block, so size = 0
   //  do NOT inc parent's link count.
   //(4).1. Allocate an INODE and a disk block:
   int ino = ialloc(dev);
   int blk = balloc(dev);
   MINODE* mip = iget(dev, ino); // load INODE into a minode
   //initialize mip->INODE as a DIR INODE;
   INODE *ip = &mip->INODE;
   ip->i_mode = 0x81A4;		// OR 040755: DIR type and permissions
   ip->i_uid  = running->uid;	// Owner uid 
   ip->i_gid  = running->gid;	// Group Id
   ip->i_size = 0;		// Size in bytes 
   ip->i_links_count = 1;	        // Links count=2 because of . and ..
   ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
   ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
   ip->i_block[0] = blk;
   for(int i = 1; i<15; i++)
   {
      ip->i_block[i] = 0;
   }
   //mark minode modified (dirty);
   mip->dirty = 1; 
   iput(mip); // write INODE back to disk
   // (4).3. make data block 0 of INODE to contain . and .. entries;
   char buf[BLKSIZE];
   bzero(buf, BLKSIZE); // optional: clear buf[ ] to 0
   DIR *dp = (DIR *)buf;
   // make . entry
   dp->inode = ino;
   dp->rec_len = 12;
   dp->name_len = 1;
   dp->name[0] = '.';
   // make .. entry: pino=parent DIR ino, blk=allocated block
   dp = (char *)dp + 12;
   dp->inode = pip->ino;
   dp->rec_len = BLKSIZE-12; // rec_len spans block
   dp->name_len = 2;
   dp->name[0] = dp->name[1] = '.';
   put_block(dev, blk, buf); // write to blk on diks
   // write to disk block blk.
   // (4).4. enter_child(pmip, ino, basename); which enters
   // (ino, basename) as a dir_entry to the parent INODE;
   enter_name(pip, ino, child);
   return ino;
}

int creat_file(char * pathname)
{
   // This is similar to mkdir() except
   // (1). the INODE.i_mode field is set to REG file type, permission bits set to
   // 0644 = rw-r--r--, and
   // (2). no data block is allocated for it, so the file size is 0.
   // (3). links_count = 1; Do not increment parent INODE’s links_count
    MINODE *start,*pip;		   
   int pino;
   char * parent, *child;
   
   // 1. pahtname = absolute: start = root;         dev = root->dev;
   //             = relative: start = running->cwd; dev = running->cwd->dev;
   if(start == root)
   {
      dev = root ->dev;
   }
   else if(start == running ->cwd)
   {
      dev = running->cwd->dev;
   }
   // 2. Let  pathname = a/b/c
   parent = dirname(pathname);   //parent= "/a/b" OR "a/b"
   child  = basename(pathname);  //child = "c"

   //    WARNING: strtok(), dirname(), basename() destroy pathname

   // 3. Get minode of parent:

   pino  = getino(parent);
   pip   = iget(dev, pino);

   //    Verify : (1). parent INODE is a DIR (HOW?)   AND
   //             (2). child does NOT exists in the parent directory (HOW?);

   if(S_ISDIR(pip->INODE.i_mode)&&search(pip, child)==0)
   {
      printf("Ready to add file\n");
      mycreat(pip, child);
   }
   else
   {
      printf("ERROR!it's already existed or parent is not a dir type\n");
      return -1;
   }
   
   //    touch its atime, i.e. atime = time(0L), mark it DIRTY
   pip->INODE.i_atime = time(0L);
   pip->dirty = 1;
   iput(pip);  
}
