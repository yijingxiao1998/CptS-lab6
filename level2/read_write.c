int myread(int fd, char buf[], int nbytes)
{
    OFT* oftp = running->fd[fd];
    MINODE* mip = oftp->mptr;
    int fileSize = mip->INODE.i_size;
    int count = 0, blk = 0;
    int avil = 0;
    char readbuf[BLKSIZE];
    avil = fileSize - oftp->offset; // number of bytes still available in file.
    char *cq = buf;                // cq points at buf[ ]
    char ibuf[256];

    while (nbytes && avil)
    {
       // Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

        int lbk = oftp->offset / BLKSIZE;
        int startByte = oftp->offset % BLKSIZE;
     
       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
 
       if (lbk < 12){                     // lbk is a direct block
           blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
       }
       else if (lbk >= 12 && lbk < 256 + 12) { 
            //  indirect blocks 
            get_block(mip->dev, mip->INODE.i_block[12], ibuf);
            blk = ibuf[lbk - 12];
       }
       else{ 
            //  double indirect blocks
            get_block(mip->dev, mip->INODE.i_block[13] , ibuf); //get to indirect blocks from double indirect
            get_block(mip->dev, ibuf[(lbk-268) / 256] , ibuf); //get to the indirect block
            blk = ibuf[(lbk-268) % 256]; //give the byte address
       } 

       /* get the data block into readbuf[BLKSIZE] */
       get_block(mip->dev, blk, readbuf);

       /* copy from startByte to buf[ ], at most remain bytes in this block */
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
   }
   printf("myread: read %d char from file descriptor %d\n", count, fd);  
   return count;   // count is the actual number of bytes read
}



int mywrite(int fd, char buf[ ], int nbytes)
{
    int blk = 0;
    char wbuf[BLKSIZE];
    int ibuf[256], ibuf2[256];
    char * cq = buf;
    OFT* oftp = running->fd[fd];
    MINODE* mip = oftp->mptr;
    while (nbytes > 0 )
    {
        
        // compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
        int lbk = oftp->offset / BLKSIZE;
        int startByte = oftp->offset % BLKSIZE;

        // I only show how to write DIRECT data blocks, you figure out how to 
        // write indirect and double-indirect blocks.

        if (lbk < 12)
        {   
            // direct block
            if (mip->INODE.i_block[lbk] == 0)
            {   // if no data block yet
                mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
            }
            blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
        }
        else if (lbk >= 12 && lbk < 256 + 12)
        {   
            // INDIRECT blocks 
            // HELP INFO:
            if (mip->INODE.i_block[12] == 0)
            {
                // allocate a block for it;
                // zero out the block on disk !!!!
                mip->INODE.i_block[12] = balloc(mip->dev);
                bzero(mip->INODE.i_block[12], mip->dev);
            }
            // get i_block[12] into an int ibuf[256];
            blk = ibuf[lbk - 12];
            if (blk==0)
            {
                // allocate a disk block;
                // record it in i_block[12];
                mip->INODE.i_block[12] = balloc(mip->dev);
            }
        }
        else
        {
            // double indirect blocks */
            // get_block(mip->dev, mip->INODE.i_block[13] , ibuf); //get to indirect blocks from double indirect
            // get_block(mip->dev, ibuf[(lbk-268) / 256] , ibuf); //get to the indirect block
            int block = (lbk - 268)/256;
            int offset = (lbk - 268)%256;
            if (mip->INODE.i_block[13] == 0)
            {
                mip->INODE.i_block[13] = balloc(mip->dev);
                bzero(mip->INODE.i_block[13], mip->dev);
            }
            get_block(mip->dev, mip->INODE.i_block[13], (char*)(ibuf));
            if(ibuf[block] == 0)
            {
                ibuf[block] = balloc(mip->dev);
                bzero(ibuf[block], mip->dev);
                put_block(mip->dev, mip->INODE.i_block[13], (char*)ibuf);
            }
            get_block(mip->dev, ibuf[block], (char*)ibuf2);
            if(ibuf2[offset] == 0)
            {
                ibuf2[offset] = balloc(mip->dev);
                bzero(ibuf2[offset], mip->dev);
                put_block(mip->dev, ibuf[block], (char*)ibuf2);
            }
            blk = ibuf2[offset];
        }

        /* all cases come to here : write to the data block */
        get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
        char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
        int remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

        while (remain > 0)
        {   
            // write as much as remain allows  
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
    printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
    return nbytes;
}

void cp(char* src, char* dest)
{
    int fd = open_file(src, 0);

    int gd = open_file(dest,1); 
    char buf[BLKSIZE];
    //    NOTE:In the project, you may have to creat the dst file first, then open it 
    //         for WR, OR  if open fails due to no file yet, creat it and then open it
    //         for WR.

    while( n==myread(fd, buf, BLKSIZE) )
    {
        mywrite(gd, buf, n);  // notice the n in write()
    }
}

void cat(char* filename)
{
    char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
    int n;

    int fd = open_file(filename, 0);
    printf("Open fd\n");
    while(n = myread(fd, mybuf, 1024))
    {
        // int i = 0;
        mybuf[n] = 0;             // as a null terminated string
        printf("%s", mybuf);   //<=== THIS works but not good
        //spit out chars from mybuf[ ] but handle \n properly;
        // while(mybuf[i])
        // {
        //     putchar(mybuf[i]);
        //     i ++;
        // }
   } 
   close_file(fd);
}

void mv (char* src, char* dest)
{
    // 1. verify src exists; get its INODE in ==> you already know its dev
    // 2. check whether src is on the same dev as src

    //           CASE 1: same dev:
    // 3. Hard link dst with src (i.e. same INODE number)
    // 4. unlink src (i.e. rm src name from its parent directory and reduce INODE's
    //            link count by 1).
                
    //           CASE 2: not the same dev:
    // 3. cp src to dst
    // 4. unlink src
}
