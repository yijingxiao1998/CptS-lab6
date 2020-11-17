/************************mkdir_creat file**********************************/
int mymkdir(int pip, char* child)
{
   //(4).1. Allocate an INODE and a disk block:
   int ino = ialloc(dev);
   int blk = balloc(dev);
   MINODE* mip = iget(dev, ino); // load INODE into a minode
   //initialize mip->INODE as a DIR INODE;
   mip->INODE.i_block[0] = blk;
   for(int i = 1; i<15; i++)
   {
      mip->INODE.i_block[i] = 0;
   }
   //mark minode modified (dirty);
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
   dp->inode = pino;
   dp->rec_len = BLKSIZE-12; // rec_len spans block
   dp->name_len = 2;
   dp->name[0] = d->name[1] = '.';
   put_block(dev, blk, buf); // write to blk on diks
   // write to disk block blk.
   // (4).4. enter_child(pmip, ino, basename); which enters
   // (ino, basename) as a dir_entry to the parent INODE;
}

int enter_name(MINODE *pip, int ino, char *name)
{
   /****************** Algorithm of enter_name *******************/
   for(int i =0; i<12; i++) //each data block of parent DIR do // assume: only 12 direct blocks
   {
      if (i_block[i]==0) BREAK; 
      // to step (5) below
      (1). Get parent’s data block into a buf[ ];
      (2). In a data block of the parent directory, each dir_entry has an ideal
      length
      ideal_length = 4*[ (8 + name_len + 3)/4 ] // a multiple of 4
      All dir_entries rec_len = ideal_length, except the last entry. The
      rec_len of the LAST entry is to the end of the block, which may be
      larger than its ideal_length.
      (3). In order to enter a new entry of name with n_len, the needed length is
      need_length = 4*[ (8 + n_len + 3)/4 ] // a multiple of 4
      (4). Step to the last entry in the data block:
      get_block(parent->dev, parent->INODE.i_block[i], buf);
      dp = (DIR *)buf;
      cp = buf;
      while (cp + dp->rec_len < buf + BLKSIZE)
      {
         cp += dp->rec_len;
         dp = (DIR *)cp;
      }
      // dp NOW points at last entry in block
      remain = LAST entry’s rec_len - its ideal_length;
      if (remain >= need_length)
      {
         enter the new entry as the LAST entry and
         trim the previous entry rec_len to its ideal_length;
      }
      goto step (6);
   }
}

int make_dir(char* pathname)
{
   MINODE *start;		   
   int pip, pino;
   char * parent, child;
   
// 1. pahtname = absolute: start = root;         dev = root->dev;
//             = relative: start = running->cwd; dev = running->cwd->dev;

// 2. Let  pathname = a/b/c
   parent = dirname(pathname);   //parent= "/a/b" OR "a/b"
   child  = basename(pathname);  //child = "c"

//    WARNING: strtok(), dirname(), basename() destroy pathname

// 3. Get minode of parent:

   pino  = getino(parent);
   pip   = iget(dev, pino); 

//    Verify : (1). parent INODE is a DIR (HOW?)   AND
//             (2). child does NOT exists in the parent directory (HOW?);
               
   mymkdir(pip, child);

// 5. inc parent inodes's links count by 1; 
//    touch its atime, i.e. atime = time(0L), mark it DIRTY

   iput(pip);  
}

creat(char * pathname)
{
// This is similar to mkdir() except
// (1). the INODE.i_mode field is set to REG file type, permission bits set to
// 0644 = rw-r--r--, and
// (2). no data block is allocated for it, so the file size is 0.
// (3). links_count = 1; Do not increment parent INODE’s links_count
}
