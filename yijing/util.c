/*********** util.c file ****************/
int n;
//char *name[64];   // token string pointers
char gline[256];  // holds token strings, each pointed by a name[i]
int nname = 0;    // number of token strings

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   n = read(dev, buf, BLKSIZE);
   if(n<0)
   	printf("get_block [%d %d] error\n", dev, blk);
}   
int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   n = write(dev, buf, BLKSIZE);
   if(n != BLKSIZE)
   	printf("put_block [%d %d] error\n", dev, blk);
}   

int tokenize(char *pathname)
{
  // copy pathname into gpath[]; tokenize it into name[0] to name[n-1]
  char *s;
  strcpy(gline, pathname);
  s = strtok(gline, "/");
  while(s)
  {
  	name[nname++] = s;
  	s = strtok(0, "/");
  }
}

MINODE *mialloc()   // allocate a Free minode for use
{
  int i;
  for(i = 0; i<NMINODE; i++)
  {
  	MINODE *mp = &minode[i];
  	if(mp->refCount == 0)
  	{
  		mp->refCount = 1;
  		return mp;
  	}
  }
  printf("FS panic: out of minodes\n");
  return 0;
}

// (p 323) This function returns a pointer to the in-memory minode containing the INODE of (dev, ino). The returnde minode is unique, i.e. only copy of the INODE exists in memory.
MINODE *iget(int dev, int ino)
{
  MINODE *mip;
  //MTABLE *mp;
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];
  
  // search in-memory minode first
  for(i = 0; i<NMINODE; i++)
  {
  	mip = &minode[i];
  	if(mip->refCount && (mip->dev == dev) && (mip->ino == ino))
  	{
  		mip->refCount++;
  		return mip;
  	}
  }
  // needed INODE= (dev, ino) not in memory
  mip = mialloc();     // allocate a FREE minode
  mip->dev = dev;  
  mip->ino = ino;      // assign to (dev, ino)
  block = (ino-1)/8 + inode_start;     // disk block containing this inode
  offset = (ino-1)%8;             // which inode in this block
  get_block(dev, block, buf);
  ip = (INODE *)buf + offset;
  mip->INODE = *ip;               // copy inode to minode.INODE
  // initialize minode
  mip->refCount = 1;
  mip->mounted = 0;
  mip->dirty = 0;
  mip->mptr = 0;
  return mip;
}

int midalloc(MINODE *mip)
{
  mip->refCount = 0;
}

// (p 324) THis function releases a used minode pointed by mip. Each minode has a refCount, which represents the number of users that are using the minode.
void iput(MINODE *mip)
{
  // dispose of minode pointed by mip
  INODE *ip;
  int i, block, offset;
  char buf[BLKSIZE];
  
  
  if(mip == 0)
  	return;
  mip->refCount--;          // dec reFcount by 1
  if(mip->refCount > 0)     // still has user
  	return;
  if(mip->dirty == 0)       // no need to write back
  	return;
  
  // write INODE back to disk
  block = (mip->ino-1)/8+inode_start;
  offset = (mip->ino-1)%8;
  
  // get blcok containing this inode
  get_block(mip->dev, block, buf);
  ip = (INODE *)buf + offset;        // ip points at INODE
  *ip = mip->INODE;                  // copy INODE to inode in block
  put_block(mip->dev, block, buf);   // write back to disk
  midalloc(mip);                     // mip->refCount = 0;
} 

int search(MINODE *mip, char *name)
{
  // search for name in (DIRECT) data blocks of mip->INODE
  // return its ino
  int i;
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;
  for(i=0; i<12; i++)      // search DIR direct blocks only
  {
  	if(mip->INODE.i_block[i] == 0)
  		return 0;
  	get_block(mip->dev, mip->INODE.i_block[i], sbuf);
  	dp = (DIR *)sbuf;
  	cp = sbuf;
  	printf("i_number rec_len name_len  name\n");
  	while(cp<sbuf + BLKSIZE)
  	{
  		strncpy(temp, dp->name, dp->name_len);
  		temp[dp->name_len] = 0;
  		printf("%8d%8d%8u    %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
  		if(strcmp(name, temp) == 0)
  		{
  			printf("found %s : inumber = %d\n", name, dp->inode);
  			
  			return dp->inode;
  		}
  		cp += dp->rec_len;
  		dp = (DIR *)cp;
  	}
  }
  return 0;
}

// (p 326) This function implements the file system tree traversal algorithm. It return the INODE number (ino) of a specified pathname.
int getino(char *pathname)
{
  // return ino of pathname
  MINODE *mip;
  int i, ino;
  if(strcmp(pathname, "/") == 0)
  	return 2;             // return root ino=2
  if(pathname[0] == '/')
  	mip = root;           // if absolute pathname: start from root
  else
  	mip = running->cwd;    // if relative pathname: start from CWD
  mip->refCount++;            // in order to iput(mip) later

  tokenize(pathname);              // assume: name[], nname are globals
  
  for(i=0; i<nname; i++)       // search for each component string
  {
  	if(!S_ISDIR(mip->INODE.i_mode))  // check DIR type
  	{
  		//printf("%s is not a directory\n", name[i]);
  		iput(mip);
  		return 0;
  	}
  	ino = search(mip, name[i]);
  	if(!ino)
  	{
  		printf("no such component name %s\n", name[i]);
  		iput(mip);
  		return 0;
  	}
  	iput(mip);              // release current minode
  	mip = iget(dev, ino);   // switch to new minode
  }
  iput(mip);
  return ino;
}

int findmyname(MINODE *parent, u32 myino, char *myname) 
{
  // WRITE YOUR code here:
  // search parent's data block for myino;
  // copy its name STRING to myname[ ];
  int i;
  char *cp, temp[256], sbuf[BLKSIZE];
  DIR *dp;
  for(i=0; i<12; i++)      // search DIR direct blocks only
  {
  	if(parent->INODE.i_block[i] == 0)
  		return 0;
  	get_block(parent->dev, parent->INODE.i_block[i], sbuf);
  	dp = (DIR *)sbuf;
  	cp = sbuf;
  	while(cp<sbuf + BLKSIZE)
  	{
  		if(dp->inode == myino)
  		{
  			strncpy(myname, dp->name, dp->name_len);
  			return 1;
  		}
  		cp += dp->rec_len;
  		dp = (DIR *)cp;
  	}
  }
  return 0;
}

int findino(MINODE *mip, u32 *myino) // myino = ino of . return ino of ..
{
  // mip->a DIR minode. Write YOUR code to get mino=ino of .
  //                                         return ino of ..
  char buf[BLKSIZE], *cp;
  DIR *dp;
  
  get_block(mip->dev, ip->i_block[0], buf);
  cp = buf;
  dp = (DIR *)buf;
  *myino = dp->inode;
  cp += dp->rec_len;
  dp = (DIR *)cp;
  return dp->inode;
}
