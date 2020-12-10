//(p 344)
int open_file(char *filename, int mode)
{
  int ino;
  char chartemp;
  MINODE *mip;
  // 1. ask for a pathname and mode to open: You may use mode = 0|1|2|3 for R|W|RW|APPEND
  if(mode == 0)  // covert to char
  	chartemp = 'r';  // for R
  else
  	chartemp = 'w';  // for W|RW|APPEND
  	
  // 2. get pathname's inumber:
  if (filename[0] == '/') 
  	dev = root->dev;          // root INODE's dev
  else                  
  	dev = running->cwd->dev;  
  	
  // get file’s minode:
  ino = getino(filename, &dev); 
  if (ino == 0)  // if file does not exist
  {   
  	creat_file(filename);      // creat it first, then
  	ino = getino(filename, &dev);    // get its ino
  }

  // 3. get its Minode pointer
  mip = iget(dev, ino); 
  
  MINODE* minodePtr = mip; // point to file’s minode
  minodePtr->refCount = 1; 

  // 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
  if(!S_ISREG(mip->INODE.i_mode))
  {
  	printf("open failed: not regular file\n");
  	iput(mip);
  	return -1;
  }
  // Check whether the file is ALREADY opened with INCOMPATIBLE mode:
  if(mip->refCount > 1)
  {
  	if(mip->dirty == 1) //If it's already opened for W, RW, APPEND : reject.
  	{
  		printf("open failed: file already opened for W|RE|APPEND\n");
  		iput(mip);
  		return -1;
  	}
  	// (that is, only multiple R are OK)
  }        

  // 5. allocate a FREE OpenFileTable (OFT) and fill in values:
  OFT *oftp;
  oftp = malloc(sizeof(OFT));  // allocating space
  oftp = &oft[oft_index];      // oftp entry points to the openTable entry
  oftp->mode = mode;           // mode = 0|1|2|3 for R|W|RW|APPEND 
  oftp->refCount = 1;
  oftp->mptr = mip;            // point at the file's minode[]

  // 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
  switch(mode){
         case 0 : 
         	oftp->offset = 0;     // R: offset = 0
         	break;
         case 1 : 
         	truncate(mip);        // W: truncate file to 0 size
         	oftp->offset = 0;
         	break;
         case 2 : 
         	oftp->offset = 0;     // RW: do NOT truncate file
         	break;
         case 3 : 
         	oftp->offset =  mip->INODE.i_size;  // APPEND mode. set offset to end of file
         	break;
         default: 
         	printf("invalid mode\n");
         	return -1;
      }

   // 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
   int i;
   for(i = 0; i < NFD; i++)
   {
   	if(!running->fd[i])
   	{
   		running->fd[i] = oftp;  // Let running->fd[i] point at the OFT entry
   		break;
   	}
   }
   if(i == NFD)  // no fd[i] is null
   {
   	return -1;
   }  

   //8. update INODE's time field
   // for R: touch atime. 
   mip->INODE.i_atime = time(0L);
   // for W|RW|APPEND mode : touch atime and mtime
   if(mode != 0)
   {
      mip->INODE.i_mtime = time(0L);
      mip->dirty = 1;   // mark Minode[ ] dirty
   }
   
   oft_index++;   // after open, global index must be increased
   printf("open done\n");
   
   // 9. return i as the file descriptor  
   printf("#fd = %d\n", i);
   return i;
}

int close_file(int fd)
{
  MINODE *mip;
  OFT *oftp;
  
  // 1. verify fd is within range.
  if(NFD < fd)
  {
  	printf("close failed: fd %d is out of range\n", fd);
  	return -1;
  }

  // 2. verify running->fd[fd] is pointing at a OFT entry
  if(running->fd[fd] == 0)  // it is not a valid oft
  {
  	printf("close failed: fd %d is not pointing at a OFT entry\n");
  	return -1;
  }

  // 3. The following code segments should be fairly obvious:
  oftp = running->fd[fd];
  running->fd[fd] = 0;
  oftp->refCount--;        // dec OFT's refCount by 1
  if (oftp->refCount > 0)  // if last process using this OFT
  	return 0;

  // last user of this OFT entry ==> dispose of the Minode[]
  mip = oftp->mptr;
  iput(mip);
  
  return 0; 
}

// it only needs to check the requested position value is within the bounds of [0, fileSize-1]
int mylseek(int fd, int position)
{
  int op;
  
  // save original position
  op = running->fd[fd]->offset;
  
  // From fd, find the OFT entry. 
  // change OFT entry's offset to position but make sure NOT to over run either end of the file.
  printf("range is [0, %d]", running->fd[fd]->mptr->INODE.i_size);
  if(position < 0 || position >= running->fd[fd]->mptr->INODE.i_size)
  {	
  	printf("lseek out of bounds of file\n");
  	return -1;
  }
  
  running->fd[fd]->offset = position;  // set to new position

  return op;
}
