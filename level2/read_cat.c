int myread(int fd, char *buf, int nbytes)
{
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->mptr;
    int count = 0;                                 // number of bytes read
    int avil = mip->INODE.i_size - oftp->offset;   // number of bytes still available in file.
    char *cq = buf, readbuf[BLKSIZE];              // cq points at buf[ ]
    int blk, ibuf[256];

    while (nbytes && avil)
    {
       // Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

       int lbk       = oftp->offset / BLKSIZE;  // compute logical block
       int startByte = oftp->offset % BLKSIZE;  // start byte in block
     
       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
 
       if (lbk < 12)  // lbk is a direct block
       {                     
           blk = mip->INODE.i_block[lbk];  // map LOGICAL lbk to PHYSICAL blk
       }
       else if (lbk >= 12 && lbk < 256 + 12)  //  indirect blocks 
       { 
            get_block(mip->dev, mip->INODE.i_block[12], (char*)ibuf);
            blk = ibuf[lbk-12];
       }
       else  //  double indirect blocks
       { 
            get_block(mip->dev, mip->INODE.i_block[13], (char*)ibuf);
            
            get_block(mip->dev, ibuf[(lbk-(12+256))/256], (char*)ibuf);
            blk = ibuf[(lbk-(12+256)) % 256];
       } 

       // get the data block into readbuf[BLKSIZE]
       get_block(mip->dev, blk, readbuf);

       // copy from startByte to buf[ ], at most remain bytes in this block
       char *cp = readbuf + startByte;   
       int remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

       while (remain > 0){
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
             oftp->offset++;           // advance offset 
             count++;                  // inc count as number of bytes read
             avil--; nbytes--;  remain--;
             if (nbytes <= 0 || avil <= 0) 
                 break;
       }
 
       // if one data block is not enough, loop back to OUTER while for more ...

   }
   return count;   // count is the actual number of bytes read
}

int read_file(int fd, int nBytes)
{
   char temp[BLKSIZE];
   int count;
   if(!running->fd[fd])  // not open
   {
    	printf("file is not open\n");
    	return -1;
   }
   if(running->fd[fd]->mode != 0 && running->fd[fd]->mode != 2)  // not RD or RW
   {
    	printf("file is not for RD or RW\n");
    	return -1;
   }
    
   count = myread(fd, temp, nBytes);
   printf("myread: read %d char from file descriptor fd=%d\n", count, fd); 
   printf("read done\n"); 
   
   return count;
}

int cat(char *filename)
{
   char mybuf[BLKSIZE], dummy = 0;  // a null char at end of mybuf[ ]
   int n, count = 0;
   
   int fd = open_file(filename, 0);  // open filename for READ;
   if(fd < 0)
   {
   	printf("cat failed: file cannot open\n");
   	return -1;
   }
   
   printf("\n");
   while(n = myread(fd, mybuf, BLKSIZE))
   {
       mybuf[n] = 0;             // as a null terminated string
       printf("%s", mybuf);      // <=== THIS works but not good
       // spit out chars from mybuf[ ] but handle \n properly;
       bzero(mybuf, BLKSIZE);
       count += n;
   } 
   
   printf("\n");
   printf("cat done: read %d char from file\n", count);
   close_file(fd);
   return 0;
}
