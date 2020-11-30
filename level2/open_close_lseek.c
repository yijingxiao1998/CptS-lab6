int open(char *filename, int flags)
{
    int ino;
    /******************* Algorithm of open *********************/
    // (1). get file’s minode:
    ino = getino(filename);
    if (ino==0)
    {   
        // if file does not exist
        creat_file(filename); // creat it first, then
        ino = getino(filename); // get its ino
    }
    MINODE* mip = iget(dev, ino);
    // (2). allocate an openTable entry OFT; initialize OFT entries:
    // mode = 0(RD) or 1(WR) or 2(RW) or 3(APPEND)
    MINODE* minodePtr = mip; // point to file’s minode
    minodePtr->refCount = 1;
    // set offset = 0 for RD|WR|RW; set to file size for APPEND mode;
    // (3). Search for the first FREE fd[index] entry with the lowest index in PROC;
    // fd[index] = &OFT; // fd entry points to the openTable entry;
    // (4). return index as file descriptor;
}

int lseek(int fd, int position)
{

}


void close(int fd)
{
    /**************** Algorithm of close ***************/
    // (1). check fd is a valid opened file descriptor;
    // (2). if (PROC’s fd[fd] != 0){ // points to an OFT
    // . OFT.refCount--; // dec OFT’s refCount by 1
    // if (refCount == 0) // if last process using this OFT
    // iput(OFT.minodePtr); // release minode
    // }
    // (4). PROC.fd[fd] = 0; // clear PROC’s fd[fd] to 0
}

