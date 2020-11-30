int read(int fd, char *buf, int nbytes)
{
    int count;
    /********* Algorithm of read(int fd, char *buf, int nbytes) *********/
    count = 0; // number of bytes read
    // offset = OFT.offset; // byte offset in file to READ
    // compute bytes available in file: avil = fileSize – offset;
    // (2). while (nbytes && avil){
    // compute logical block: lbk = offset / BLKSIZE;
    // start byte in block: start = offset % BLKSIZE;
    // (3). convert logical block number, lbk, to physical block number, blk,
    // through INODE.i_block[ ] array;
    // (4). get_block(dev, blk, kbuf); // read blk into char kbuf[BLKSIZE];
    // char *cp = kbuf + start;
    // remain = BLKSIZE - start;
    // (5). while (remain){ // copy bytes from kbuf[ ] to buf[ ]
    // *buf++ = *cp++;
    // offset++; count++; // inc offset, count;
    // remain--; avil--; nbytes--; // dec remain, avail, nbytes;
    // if (nbytes==0 || avil==0)
    // break;
    // } // end of while(remain)
    // } // end of while(nbytes && avil)
    // (6). return count;
}

int write(int fd, char buf[ ], int nbytes)
{
    /******* Algorithm of write(int fd, char *buf, int nbytes) *******/
    // (1). count = 0; // number of bytes written
    // (2). while (nbytes){
    // compute logical block: lbk = oftp->offset / BLOCK_SIZE;
    // compute start byte: start = oftp->offset % BLOCK_SIZE;
    // (3). convert lbk to physical block number, blk, through the i_block[ ] array;
    // (4). read_block(dev, blk, kbuf); // read blk into kbuf[BLKSIZE];
    // char *cp = kbuf + start; remain = BLKSIZE - start;
    // (5) while (remain){ // copy bytes from buf[ ] to kbuf[ ]
    // *cp++ = *buf++;
    // offset++; count++; // inc offset, count;
    // remain --; nbytes--; // dec remain, nbytes;
    // if (offset > fileSize) fileSize++; // inc file size
    // if (nbytes <= 0) break;
    // } // end while(remain)
    // (6). write_block(dev, blk, kbuf);
    // } // end while(nbytes)
    // (7). set minode dirty = 1; // mark minode dirty for iput() when fd is closed
    // return count;
}