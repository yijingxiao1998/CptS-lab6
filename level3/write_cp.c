int mywrite(int fd, char buf[ ], int nbytes) 
{
  OFT *oftp = running->fd[fd];
  MINODE *mip = oftp->mptr;
  char wbuf[BLKSIZE], *cq = buf;
  int i, blk, ibuf[256], mbuf[256];
  
  while (nbytes > 0)
  {
     // compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
     int lbk       = oftp->offset / BLKSIZE;
     int startByte = oftp->offset % BLKSIZE;

     // I only show how to write DIRECT data blocks, you figure out how to 
     // write indirect and double-indirect blocks.

     if (lbk < 12) // direct block
     {                         
        if (mip->INODE.i_block[lbk] == 0)    // if no data block yet
        {   
           mip->INODE.i_block[lbk] = balloc(mip->dev); // MUST ALLOCATE a block
        }
        blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
     }
     else if (lbk >= 12 && lbk < 256 + 12)  // INDIRECT blocks 
     { 
         // HELP INFO:
         if (mip->INODE.i_block[12] == 0)
         {
            // allocate a block for it;
            mip->INODE.i_block[12] = balloc(mip->dev);
            // zero out the block on disk !!!!
            get_block(mip->dev, mip->INODE.i_block[12], wbuf);
            for(i = 0; i < BLKSIZE; i++)
               wbuf[i] = 0;
            put_block(mip->dev, mip->INODE.i_block[12], wbuf);
         }
         
         // get i_block[12] into an int ibuf[256];
         get_block(mip->dev, mip->INODE.i_block[12], ibuf);
         blk = ibuf[lbk - 12];
         
         if (blk==0)
         {
            // allocate a disk block; record it in i_block[12];
            mip->INODE.i_block[12] = balloc(mip->dev);
            blk = mip->INODE.i_block[12];
         }
     }
     else
     {
         // double indirect blocks
         if(mip->INODE.i_block[13] == 0)  
         {
            // allocate a block for it;
            mip->INODE.i_block[13] = balloc(mip->dev);
            // zero out the block on disk !!!!
            get_block(mip->dev, mip->INODE.i_block[13], wbuf);
            for(i = 0; i < BLKSIZE; i++)
               wbuf[i] = 0;
            put_block(mip->dev, mip->INODE.i_block[13], wbuf);
         }
         
         // double indirenct block -> i_block[13]
         get_block(mip->dev, mip->INODE.i_block[13], ibuf);
         /*if(ibuf[(lbk-(12+256))/256] == 0)
         {
            // allocate a block for it;
            ibuf[(lbk-(12+256))/256] = balloc(mip->dev);
            // zero out the block on disk !!!!
            get_block(mip->dev, ibuf[(lbk-(12+256))/256], wbuf);
            for(i = 0; i < BLKSIZE; i++)
               wbuf[i] = 0;
            put_block(mip->dev, ibuf[(lbk-(12+256))/256], wbuf);
         }
         
         get_block(mip->dev, ibuf[(lbk-(12+256))/256], ibuf);
         if(ibuf[(lbk-(12+256))%256] == 0)
         {
            // allocate a block for it;
            ibuf[(lbk-(12+256))%256] = balloc(mip->dev);
               
         }
         blk = ibuf[(lbk-(12+256)) % 256];	
         put_block(mip->dev, ibuf[(lbk-(12+256))/256], ibuf);
         get_block(mip->dev, ibuf+((lbk-(12+256))/256), ibuf);
         blk = ibuf + (lbk-(12+256))%256;
     }

     /* all cases come to here : write to the data block */
     get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
     char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
     int remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

     if(nbytes > remain)
     	nbytes = remain;
     while (remain > 0){               // write as much as remain allows  
           *cp++ = *cq++;              // cq points at buf[ ]
           nbytes--; remain--;         // dec counts
           oftp->offset++;             // advance offset
           if (oftp->offset > mip->INODE.i_size)  // especially for RW|APPEND mode
               mip->INODE.i_size++;    // inc file size (if offset > fileSize)
           if (nbytes <= 0) break;     // if already nbytes, break
     }
     put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
     
     // loop back to outer while to write more .... until nbytes are written
  }

  mip->dirty = 1;       // mark mip dirty for iput() 
  iput(mip);        
  return nbytes;
}

int write_file()
{
   char temp[BLKSIZE];
   int nbytes, ret;
   
   // 1. Preprations: ask for a fd and a text string to write;
   if(!running->fd[fd])  // not open
   {
    	printf("file is not open\n");
    	return -1;
   }
   // 2. verify fd is indeed opened for WR or RW or APPEND mode
   if(running->fd[fd]->mode != 1 && running->fd[fd]->mode != 2)  // not RD or RW
   {
    	printf("file is not for W or RW\n");
    	return -1;
   }

   printf("input text:\n");
   fgets(temp, 128, stdin);
   temp[strlen(temp)-1] = 0;
   
   // 3. copy the text string into a buf[] and get its length as nbytes.	
   nbytes = strlen(temp);
   ret = mywrite(fd, temp, nbytes);
   printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);  
   printf("write done\n"); 
   return ret;
}

void cp(char* src, char* dest)
{
    int fd = open_file(src, 0);
    int gd = open_file(dest,1); 
    printf("fd=%d gd=%d\n", fd, gd);
    
    char buf[BLKSIZE];
    //    NOTE:In the project, you may have to creat the dst file first, then open it 
    //         for WR, OR  if open fails due to no file yet, creat it and then open it
    //         for WR.

    while(n = myread(fd, buf, BLKSIZE))
    {
        mywrite(gd, buf, n);  // notice the n in write()
    }
    
    printf("cp done\n");
}
