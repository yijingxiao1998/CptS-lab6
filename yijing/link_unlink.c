// link-unlink


// link
int link(char* oldFile, char* newFile)
{
    // verify old_file exists and is not a DIR;
    int oino = getino(oldFile);
    MINODE* omip = iget(dev, oino);
    //check omip->INODE file type (must not be DIR).
    if(S_ISDIR(omip->INODE.i_mode))
    {
        printf("ERROR! try to link a dir type file");
        return 0;
    }
    // new_file must not exist yet:
    if(getino(newFile)!=0)
    {
        //must return 0;
        printf("ERROR! File already exists\n");
        return 0;
    } 
    //(3). creat new_file with the same inode number of old_file:
    char *parent = dirname(newFile); 
    char *child = basename(newFile);
    int pino = getino(parent);
    MINODE* pmip = iget(dev, pino);
    // creat entry in new parent DIR with same inode number of old_file
    enter_name(pmip, oino, child);
    //4). omip->INODE.i_links_count++; // inc INODE’s links_count by 1
    omip->INODE.i_links_count += 1;
    omip->dirty = 1; // for write back by iput(omip)
    iput(omip);
    iput(pmip);
}

// unlink
int unlink(char* fileName)
{
    //(1). get filenmae’s minode:
    int ino = getino(fileName);
    MINODE *mip = iget(dev, ino);
    //check it’s a REG or symbolic LNK file; can not be a DIR
    if(S_ISREG(mip->INODE.i_mode) || S_ISLNK(mip->INODE.i_mode))
    {
        // remove name entry from parent DIR’s data block:
        char *parent = dirname(fileName); 
        char* child = basename(fileName);
        int pino = getino(parent);
        MINODE *pmip = iget(dev, pino);
        rm_child(pmip,  child);
        pmip->dirty = 1;
        iput(pmip);
        // decrement INODE’s link_count by 1
        mip->INODE.i_links_count--;
        if (mip->INODE.i_links_count > 0)
        {
            // for write INODE back to disk
            mip->dirty = 1; 
        }
        else
        { // if links_count = 0: remove filename
            //deallocate all data blocks in INODE;
            for(int i = 0; i < 15; i++)
            {
                bdalloc(ino, i);
            }
            //deallocate INODE;
            idalloc(dev, ino);
        }
        iput(mip); // release mip
    }
    else if(S_ISDIR(mip->INODE.i_mode))
    {
        printf("ERROR! this is a DIR type file\n");
        return 0;
    }
    
    
}
