//Assume: command line = "rmdir pathname"
//1. Extract cmd, pathname from line and save them as globals.

int rmdir()
{
  //2. get inumber of pathname: determine dev, then  
  int ino = getino(pathname); 
  
  //3. get its minode[ ] pointer:
  MINODE *mip = iget(dev, ino);

  /*4. check ownership 
     super user : OK*/
  if(running->uid == 0)
  	printf("rmdir: super user: OK\n");
  // not super user: uid must match
  else
  {
     	printf("rmdir: not super user: uid must match\n");
     	if(running->uid != mip->INODE.i_uid)  // i_uid is owner uid
     	{
     		printf("uid does not match\n");
     		iput(mip);
     		return -1;
     	}
     	printf("uid match\n");
  }
 
  ------------------------------------------------------------------------
  /*5. check DIR type (HOW?), not BUSY (HOW?) AND is empty:

  HOWTO check whether a DIR is empty:
  First, check link count (links_count > 2 means not empty);*/
  //if (NOT DIR || BUSY || not empty): iput(mip); retunr -1;
  // p337
  if(mip->INODE.i_links_count > 2)
  {
     	printf("rmdir: dir is not empty\n");
     	iput(mip);
     	return -1;
  }
  // check dir type
  if(!S_ISDIR(mip->INODE.i_mode))
  {
     	printf("it is not a dir\n");
     	iput(mip);
     	return -1;
  }
  // check minode is busy or not
  if(mip->refCount != 1)  
  {
     	printf("minode is busy\n");
     	iput(mip);
     	return -1;
  }
     
  /*However, links_count == 2 may still have FILEs, so go through its data 
  block(s) to see whether it has any entries in addition to . and ..*/
  char *cp;
  char buf[BLKSIZE];
  get_block(dev, mip->INODE.i_block[0], buf);   // get data block into buf[]
  dp = (DIR *)buf;    //as dir_entry
  cp = buf;
  int x = 0;
  
  // traverse data blocks for number of entries = 2
  while(cp<buf+BLKSIZE && x<2)
  {
  	if(strcmp(dp->name, "."))
  	{
  		iput(mip);
  		return -1;
  	}
  	if(strcmp(temp, "..") == 0)
  	{
  		iput(mip);
  		return -1;
  	}
  	cp += dp->rec_len;   // advance cp by rec_len
  	dp = (DIR *)cp;      // pull dp to next entry
  	x++;
  }  

  /*6. ASSUME passed the above checks.
     Deallocate its block and inode:*/

  for (i=0; i<12; i++){
         if (mip->INODE.i_block[i]==0)
             continue;
         bdealloc(mip->dev, mip->INODE.i_block[i]);
  }
  idealloc(mip->dev, mip->ino);
    
  iput(mip); //(which clears mip->refCount = 0);
     

  //7. get parent DIR's ino and minode (pointed by pip);
  int pino = findino();   // get pino from .. entry in INODE.i_block[0]
  MINODE *pmip = iget(mip->dev, pino);

  //8. remove child's entry from parent directory by
  char *temp;
  char child[64], pntemp[64];
  strcpy(pntemp, pathname);
  temp = base(pntemp);
  strcpy(child, temp);
  rm_child(pmip, child);

  //9. decrement pip's link_count by 1; 
  pmip->INODE.i_links_count--;
  //touch pip's atime, mtime fields;
  pmip->INODE.i_atime = pmip->INODE.i_mtime = time(0);
  // mark pip dirty;
  pmip->dirty = 1;
  iput(pmip);
  return 1;
}

// rm_child(): removes the entry [INO rlen nlen name] from parent's data block.

int rm_child(MINODE *parent, char *name)
{
   int i, j, tl;
   char *cp, temp[256], sbuf[BLKSIZE];
   DIR *dp;
   //1. Search parent INODE's data block(s) for the entry of name
   for(i=0; i<12; i++)      // search DIR direct blocks only
   {
   	//2. Erase name entry from parent directory by
  	if(parent->INODE.i_block[i] == 0)
  		return 0;
  	get_block(parent->dev, parent->INODE.i_block[i], sbuf);
  	dp = (DIR *)sbuf;
  	cp = sbuf;
  	
  	/*if(strcmp(name, temp) == 0)
  	{
  		for(j = i; j<12; j++)
  		{
  			pmip->INODE.i_block[j] = pmip->INODE.i_block[j+1];
  		}
  		pmip->INODE.i_size -= BLKSIZE;  // reduce parent's file size by BLKSIZE
  		put_block(pmip->dev, pmip->INODE.i_block[i], sbuf);
  		return 1;
  	}*/
  	j = 0;
  	tl = 0;
  	while(cp<sbuf + BLKSIZE)
  	{
  		strncpy(temp, dp->name, dp->name_len);
  		temp[dp->name_len] = 0;
  		tl += dp->rec_len;
  		if(strcmp(name, temp) == 0)
  		{
  			// not first entry
  			
  			}
  		}
  		else if(dp->rec_len == 0)
  			break;
  		cp += dp->rec_len;
  		dp = (DIR *)cp;
  	}
   }
   if(dp->rec_len == 0)
   {
   	printf("Did not fine %s\n", name);
   	return 0;
   }   
   
   // if first and only entry in a data block
   
    
  (2). if in the middle of a block{
          move all entries AFTER this entry LEFT;
          add removed rec_len to the LAST entry of the block;
          no need to change parent's fileSize;

               | remove this entry   |
          -----------------------------------------------
          xxxxx|INO rlen nlen NAME   |yyy  |zzz         | 
          -----------------------------------------------

                  becomes:
          -----------------------------------------------
          xxxxx|yyy |zzz (rec_len INC by rlen)          |
          -----------------------------------------------

      }
  
  (3). if (first entry in a data block){
          deallocate the data block; modify parent's file size;

          -----------------------------------------------
          |INO Rlen Nlen NAME                           | 
          -----------------------------------------------
          
          Assume this is parent's i_block[i]:
          move parent's NONZERO blocks upward, i.e. 
               i_block[i+1] becomes i_block[i]
               etc.
          so that there is no HOLEs in parent's data block numbers
      }

    
  3. Write the parent's data block back to disk;
     mark parent minode DIRTY for write-back
}
