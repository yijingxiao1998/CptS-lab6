// sym_link and read_link
int symlink(char* oldFile, char* newFile)
{
//     /********* Algorithm of symlink old_file new_file *********/
//     (1). check: old_file must exist and new_file not yet exist;
//     (2). creat new_file; change new_file to LNK type;
//     (3). // assume length of old_file name <= 60 chars
//     store old_file name in newfile’s INODE.i_block[ ] area.
//     set file size to length of old_file name
//     mark new_file’s minode dirty;
//     iput(new_file’s minode);
//     (4). mark new_file parent minode dirty;
//     iput(new_file’s parent minode);
    // verify old_file exists and is not a DIR;
    int oino = getino(oldFile);
    MINODE* omip = iget(dev, oino);
    //check omip->INODE file type (must not be DIR).
    if(getino(oldFile) == 0)
    {
        printf("ERROR! old file doesn't exist\n");
        return -1;
    }
    // new_file must not exist yet:
    if(getino(newFile)!=0)
    {
        //must return 0;
        printf("ERROR! File already exists\n");
        return -1;
    }
    //     (3). // assume length of old_file name <= 60 chars
    //     store old_file name in newfile’s INODE.i_block[ ] area.
    //     set file size to length of old_file name
    //     mark new_file’s minode dirty;
    //     iput(new_file’s minode);
    creat_file(newFile);
    int nino = getino(newFile);
    MINODE* nmip = iget(dev, nino);
    nmip->INODE.i_mode = 0XA000;
    //  store old_file name in newfile’s INODE.i_block[ ] area.
    memcpy(nmip->INODE.i_block, oldFile, strlen(oldFile));
    //strcpy(nmip->INODE.i_block, oldFile);
    nmip->INODE.i_size = strlen(oldFile);
    nmip->dirty = 1;
    iput(nmip);
    // (4). mark new_file parent minode dirty;
    //  iput(new_file’s parent minode);
    int pino = getino(dirname(newFile));
    MINODE* pmip = iget(dev, pino);
    pmip->dirty = 1;
    iput(pmip);
}

int readlink(char* file, char buf[256])
{
    /************* Algorithm of readlink (file, buffer) *************/
    // (1). get file’s INODE in memory; verify it’s a LNK file
    // (2). copy target filename from INODE.i_block[ ] into buffer;
    // (3). return file size;
    int nino = getino(file);
    MINODE* nmip = iget(dev, nino);
    if(S_ISLNK(nmip->INODE.i_mode))
    {
        memcpy(buf, nmip->INODE.i_block, strlen(file));
    }
    return nmip->INODE.i_size;
}