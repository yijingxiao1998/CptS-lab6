//Assume: command line = "rmdir pathname"
//1. Extract cmd, pathname from line and save them as globals.

// rm_child(): removes the entry [INO rlen nlen name] from parent's data block.

int rm_child(MINODE *parent, char *name)
{
   int i, j, len = 0, pre = 0, t;
   char *cp, temp[256], sbuf[BLKSIZE], *cptemp;
   DIR *dp, *dptemp;
   //1. Search parent INODE's data block(s) for the entry of name
   for(i=0; i<12; i++)      // search DIR direct blocks only
   {
   	//2. Erase name entry from parent directory by
  	if(parent->INODE.i_block[i] == 0)
  		continue;
  	get_block(parent->dev, parent->INODE.i_block[i], sbuf);
  	dp = (DIR *)sbuf;
  	cp = sbuf;
  	
  	while(cp<sbuf + BLKSIZE)
  	{
  		strncpy(temp, dp->name, dp->name_len);
  		temp[dp->name_len] = 0;

  		if(strcmp(name, temp) != 0)
  		{
  			len += dp->rec_len;
  			pre = dp->rec_len;
  			cp += dp->rec_len;
  			dp = (DIR *)cp;
  		}
  		
  		if(dp->rec_len == BLKSIZE)  // first and only entry on block
  		{
  			memset(sbuf, 0, BLKSIZE);
  			put_block(parent->dev, parent->INODE.i_block[i], sbuf);
  			// deallocate the data block
  			idalloc(parent->dev, parent->INODE.i_block[i]);
  			// reduce parent's file size by BLKSIZE
  			parent->INODE.i_size -= BLKSIZE;
  			
  			for(j = i; j < 11; j++)
  			{
  				parent->INODE.i_block[j] = parent->INODE.i_block[j+1];
  			}
  			parent->INODE.i_block[11] = 0;
  		}
  		else if(len + dp->rec_len == BLKSIZE)  // last entry in block
  		{
  			len -= pre;
  			cp -= pre;
  			dp = (DIR *)cp;
  			dp->rec_len = BLKSIZE - len;
  			put_block(parent->dev, parent->INODE.i_block[i], sbuf);
  		}
  		// entry is first but not the only entry or in the middle of block
  		else
  		{
  			// move all trailing entries left to overlay the deleted entry
  			t = dp->rec_len;
  			cptemp = cp + dp->rec_len;
  			dptemp = (DIR *)cptemp;
  			while(len + t + dp->rec_len < BLKSIZE)
  			{
  				dp->inode = dptemp->inode;
  				dp->name_len = dptemp->name_len;
  				strncpy(dp->name, dptemp->name, dptemp->name_len);
  				dp->rec_len = dptemp->rec_len;
  				
  				// set to next position
  				cp += dptemp->rec_len;
  				cptemp += dptemp->rec_len;
  				
  				// and set next value
  				len += dptemp->rec_len;
  				cptemp += dptemp->rec_len;
  				dptemp = (DIR *)cptemp;
  				
  				if(dptemp->rec_len == 0)
  					return;
  			}
  			dp->inode = dptemp->inode;
  			dp->name_len = dptemp->name_len;
  			strncpy(dp->name, dptemp->name, dptemp->name_len);
  			dp->rec_len = BLKSIZE - len;
  			
  			put_block(parent->dev, parent->INODE.i_block[i], sbuf);  // Write the parent's data block back to disk
  		}
  		parent->dirty = 1;  // mark parent minode DIRTY for write-back
  		return;
  	}
   }
}


int rmdir(char *pathname)
{
  if(strcmp(pathname, "") == 0)
  {
     printf("needs path\n");
     return -1;
  }
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
 
  /*------------------------------------------------------------------------
  5. check DIR type (HOW?), not BUSY (HOW?) AND is empty:

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
  if(mip->refCount >= 1)  
  {
     	printf("minode is busy\n");
     	iput(mip);
     	return -1;
  }
     
  /*However, links_count == 2 may still have FILEs, so go through its data 
  block(s) to see whether it has any entries in addition to . and ..*/
  char buf[BLKSIZE], temp[256];
  get_block(dev, mip->INODE.i_block[0], buf);   // get data block into buf[]
  dp = (DIR *)buf;    //as dir_entry
  char *cp;
  cp = buf;
  int i = 0, x = 0, pino;
  
  // traverse data blocks
  for(; i<12; i++)
  {
  	while(cp<buf+BLKSIZE)
  	{
  		strncpy(temp, dp->name, dp->name_len);
  		temp[dp->name_len] = 0;
  		if(strcmp(temp, ".") != 0)
  		{
  			if(strcmp(temp, "..") != 0)
  			{
  				printf("dir is not empty\n");
  				iput(mip);
  				return -1;
  			}
  			else
  			{
  				pino = dp->inode;
  			}
  		}
  		cp += dp->rec_len;   // advance cp by rec_len
  		dp = (DIR *)cp;      // pull dp to next entry
  		x++;
  	}  
  }
  if(x>2)
  {
  	printf("dir is not empty\n");
  	iput(mip);
  	return -1;
  }

  /*6. ASSUME passed the above checks.
     Deallocate its block and inode:*/
  for (i=0; i<12; i++){
         if (mip->INODE.i_block[i]==0)
             continue;
         bdalloc(mip->dev, mip->INODE.i_block[i]);
  }
  idalloc(mip->dev, mip->ino);
    
  iput(mip); //(which clears mip->refCount = 0);
     

  //7. get parent DIR's ino and minode (pointed by pip);
  MINODE *pmip = iget(mip->dev, pino);

  //8. remove child's entry from parent directory by
  rm_child(pmip, pathname);

  //9. decrement pip's link_count by 1; 
  pmip->INODE.i_links_count--;
  //touch pip's atime, mtime fields;
  pmip->INODE.i_atime = pmip->INODE.i_mtime = time(0L);
  // mark pip dirty;
  pmip->dirty = 1;
  iput(pmip);
  return 1;
}

